-- | Translate grammars to Canonical form
-- (a common intermediate representation to simplify export to other formats)
module GF.Compile.GrammarToCanonical(
       grammar2canonical,abstract2canonical,concretes2canonical,
       projection,selection
       ) where
import Data.List(nub,partition)
import qualified Data.Map as M
import Data.Maybe(fromMaybe)
import qualified Data.Set as S
import GF.Data.ErrM
import GF.Text.Pretty
import GF.Grammar.Grammar as G
import GF.Grammar.Lookup(lookupOrigInfo,allOrigInfos,allParamValues)
import GF.Grammar.Macros(typeForm,collectOp,collectPattOp,composSafeOp,mkAbs,mkApp,term2patt,sortRec)
import GF.Grammar.Lockfield(isLockLabel)
import GF.Grammar.Predef(cPredef,cInts)
-- import GF.Compile.Compute.Value(Predefined(..))
import GF.Infra.Ident(ModuleName(..),Ident,ident2raw,rawIdentS,showIdent,isWildIdent)
import GF.Infra.Option(Options,optionsPGF)
import GF.Infra.CheckM
import PGF2(Literal(..))
import GF.Compile.Compute.Concrete(normalForm)
import GF.Grammar.Canonical as C
import System.FilePath ((</>), (<.>))
import qualified Debug.Trace as T


-- | Generate Canonical code for the named abstract syntax and all associated
-- concrete syntaxes
grammar2canonical :: Options -> ModuleName -> G.Grammar -> Check C.Grammar
grammar2canonical opts absname gr = do
  abs <- abstract2canonical absname gr
  cncs <- concretes2canonical opts absname gr
  return (Grammar abs (map snd cncs))

-- | Generate Canonical code for the named abstract syntax
abstract2canonical :: ModuleName -> G.Grammar -> Check Abstract
abstract2canonical absname gr =
  return (Abstract (modId absname) (convFlags gr absname) cats funs)
  where
    cats = [CatDef (gId c) (convCtx ctx) | ((_,c),AbsCat ctx) <- adefs]

    funs = [FunDef (gId f) (convType ty) |
            ((_,f),AbsFun (Just (L _ ty)) ma mdef _) <- adefs]

    adefs = allOrigInfos gr absname

    convCtx = maybe [] (map convHypo . unLoc)
    convHypo (bt,name,t) =
      case typeForm t of
        ([],(_,cat),[]) -> gId cat -- !!
        tf -> error ("abstract2canonical convHypo: " ++ show tf)

    convType t =
      case typeForm t of
        (hyps,(_,cat),args) -> Type bs (TypeApp (gId cat) as)
          where
            bs = map convHypo' hyps
            as = map convType args

    convHypo' (bt,name,t) = TypeBinding (gId name) (convType t)

-- | Generate Canonical code for the all concrete syntaxes associated with
-- the named abstract syntax in given the grammar.
concretes2canonical :: Options -> ModuleName -> G.Grammar -> Check [(FilePath, Concrete)]
concretes2canonical opts absname gr =
  sequence
    [fmap ((,) cncname) (concrete2canonical gr absname cnc cncmod)
        | cnc<-allConcretes gr absname,
          let cncname = "canonical" </> render cnc <.> "gf"
              Ok cncmod = lookupModule gr cnc
        ]

-- | Generate Canonical GF for the given concrete module.
concrete2canonical :: G.Grammar -> ModuleName -> ModuleName -> ModuleInfo -> Check Concrete
concrete2canonical gr absname cnc modinfo = do
  defs <- fmap concat $ mapM (toCanonical gr absname) (M.toList (jments modinfo))
  return (Concrete (modId cnc) (modId absname) (convFlags gr cnc)
                   (neededParamTypes S.empty (params defs))
                   [lincat | (_,Left lincat) <- defs]
                   [lin | (_,Right lin) <- defs])
  where
    params = S.toList . S.unions . map fst

    neededParamTypes have [] = []
    neededParamTypes have (q:qs) =
        if q `S.member` have
        then neededParamTypes have qs
        else let ((got,need),def) = paramType gr q
             in def++neededParamTypes (S.union got have) (S.toList need++qs)

-- toCanonical :: G.Grammar -> ModuleName -> (Ident, Info) -> [(S.Set QIdent, Either LincatDef LinDef)]
toCanonical gr absname (name,jment) =
  case jment of
    CncCat (Just (L loc typ)) _ _ pprn _ -> do
      ntyp <- normalForm gr (L loc name) typ
      let pts = paramTypes gr ntyp
      return [(pts,Left (LincatDef (gId name) (convType ntyp)))]
    CncFun (Just r@(cat,ctx,lincat)) (Just (L loc def)) pprn _ -> do
      let params = [(b,x)|(b,x,_)<-ctx]
          args   = map snd params
      e0 <- normalForm gr (L loc name) (mkAbs params (mkApp def (map Vr args)))
      let e   = cleanupRecordFields lincat (unAbs (length params) e0)
          tts = tableTypes gr [e]
      return [(tts,Right (LinDef (gId name) (map gId args) (convert gr e)))]
    AnyInd _ m  -> case lookupOrigInfo gr (m,name) of
                     Ok (m,jment) -> toCanonical gr absname (name,jment)
                     _ -> return []
    _ -> return []
  where
    unAbs 0 t = t
    unAbs n (Abs _ _ t) = unAbs (n-1) t
    unAbs _ t = t

tableTypes :: G.Grammar -> [Term] -> S.Set QIdent
tableTypes gr ts = S.unions (map tabtys ts)
  where
    tabtys t =
      case t of
        V t cc -> S.union (paramTypes gr t) (tableTypes gr cc)
        T (TTyped t) cs -> S.union (paramTypes gr t) (tableTypes gr (map snd cs))
        _ -> collectOp tabtys t

paramTypes :: G.Grammar -> G.Type -> S.Set QIdent
paramTypes gr t =
  case t of
    RecType fs -> S.unions (map (paramTypes gr.snd) fs)
    Table t1 t2 -> S.union (paramTypes gr t1) (paramTypes gr t2)
    App tf ta -> S.union (paramTypes gr tf) (paramTypes gr ta)
    Sort _ -> S.empty
    EInt _ -> S.empty
    Q q -> lookup q
    QC q -> lookup q
    FV ts -> S.unions (map (paramTypes gr) ts)
    _ -> ignore
  where
    lookup q = case lookupOrigInfo gr q of
                 Ok (_,ResOper  _ (Just (L _ t))) ->
                                       S.insert q (paramTypes gr t)
                 Ok (_,ResParam {}) -> S.singleton q
                 _ -> ignore

    ignore = T.trace ("Ignore: " ++ show t) S.empty

-- | Filter out record fields from definitions which don't appear in lincat.
cleanupRecordFields :: G.Type -> Term -> Term
cleanupRecordFields (RecType ls) (R as) =
  let defnFields = M.fromList ls
  in R
      [ (lbl, (mty, t'))
      | (lbl, (mty, t)) <- as
      , M.member lbl defnFields
      , let Just ty = M.lookup lbl defnFields
      , let t' = cleanupRecordFields ty t
      ]
cleanupRecordFields ty t@(FV _) = composSafeOp (cleanupRecordFields ty) t
cleanupRecordFields _ t = t

convert :: G.Grammar -> Term -> LinValue
convert gr = convert' gr []

convert' :: G.Grammar -> [Ident] -> Term -> LinValue
convert' gr vs = ppT
  where
    ppT0 = convert' gr vs
    ppTv vs' = convert' gr vs'

    ppT t =
      case t of
--      Abs b x t -> ...
--      V ty ts -> VTableValue (convType ty) (map ppT ts)
        V ty ts -> TableValue (convType ty) [TableRow (ppP p) (ppT t)|(p,t)<-zip ps ts]
          where
            Ok pts = allParamValues gr ty
            Ok ps = mapM term2patt pts
        T (TTyped ty) cs -> TableValue (convType ty) (map ppCase cs)
        S t p -> selection (ppT t) (ppT p)
        C t1 t2 -> concatValue (ppT t1) (ppT t2)
        App f a -> ap (ppT f) (ppT a)
        R r -> RecordValue (fields (sortRec r))
        P t l -> projection (ppT t) (lblId l)
        Vr x -> VarValue (gId x)
        Cn x -> VarValue (gId x) -- hmm
        Con c -> ParamConstant (Param (gId c) [])
        Sort k -> VarValue (gId k)
        EInt n -> LiteralValue (LInt n)
        Q (m,n) -> if m==cPredef then ppPredef n else VarValue (gQId m n)
        QC (m,n) -> ParamConstant (Param (gQId m n) [])
        K s -> LiteralValue (LStr s)
        Empty -> LiteralValue (LStr "")
        FV ts -> VariantValue (map ppT ts)
        Alts t' vs -> alts vs (ppT t')
        _ -> error $ "convert' ppT: " ++ show t

    ppCase (p,t) = TableRow (ppP p) (ppTv (patVars p++vs) t)

    ppPredef n = error "TODO: ppPredef" {-
      case predef n of
        Ok BIND       -> p "BIND"
        Ok SOFT_BIND  -> p "SOFT_BIND"
        Ok SOFT_SPACE -> p "SOFT_SPACE"
        Ok CAPIT      -> p "CAPIT"
        Ok ALL_CAPIT  -> p "ALL_CAPIT"
        _ -> VarValue (gQId cPredef n) -- hmm
      where
       p = PredefValue . PredefId . rawIdentS
-}
    ppP p =
      case p of
        PC c ps -> ParamPattern (Param (gId c) (map ppP ps))
        PP (m,c) ps -> ParamPattern (Param (gQId m c) (map ppP ps))
        PR r -> RecordPattern (fields r) {-
        PW -> WildPattern
        PV x -> VarP x
        PString s -> Lit (show s) -- !!
        PInt i -> Lit (show i)
        PFloat x -> Lit (show x)
        PT _ p -> ppP p
        PAs x p -> AsP x (ppP p) -}
        _ -> error $ "convert' ppP: " ++ show p
      where
        fields = map field . filter (not.isLockLabel.fst)
        field (l,p) = RecordRow (lblId l) (ppP p)

--  patToParam p = case ppP p of ParamPattern pv -> pv

--  token s = single (c "TK" `Ap` lit s)

    alts vs = PreValue (map alt vs)
      where
        alt (t,p) = (pre p,ppT0 t)

        pre (K s) = [s]
        pre Empty = [""]  -- Empty == K ""
        pre (Strs ts) = concatMap pre ts
        pre (EPatt _ _ p) = pat p
        pre t = error $ "convert' alts pre: " ++ show t

        pat (PString s) = [s]
        pat (PAlt p1 p2) = pat p1++pat p2
        pat (PSeq _ _ p1 _ _ p2) = [s1++s2 | s1<-pat p1, s2<-pat p2]
        pat p = error $ "convert' alts pat: "++show p

    fields = map field . filter (not.isLockLabel.fst)
    field (l,(_,t)) = RecordRow (lblId l) (ppT t)
  --c = Const
  --c = VarValue . VarValueId
  --lit s = c (show s) -- hmm

    ap f a = case f of
               ParamConstant (Param p ps) ->
                 ParamConstant (Param p (ps++[a]))
               _ -> error $ "convert' ap: "++render (ppA f <+> ppA a)

concatValue :: LinValue -> LinValue -> LinValue
concatValue v1 v2 =
  case (v1,v2) of
    (LiteralValue (LStr ""),_) -> v2
    (_,LiteralValue (LStr "")) -> v1
    _ -> ConcatValue v1 v2

-- | Smart constructor for projections
projection :: LinValue -> LabelId -> LinValue
projection r l = fromMaybe (Projection r l) (proj r l)

proj :: LinValue -> LabelId -> Maybe LinValue
proj r l =
  case r of
    RecordValue r -> case [v | RecordRow l' v <- r, l'==l] of
                          [v] -> Just v
                          _ -> Nothing
    _ -> Nothing

-- | Smart constructor for selections
selection :: LinValue -> LinValue -> LinValue
selection t v =
  -- Note: impossible cases can become possible after grammar transformation
  case t of
    TableValue tt r ->
        case nub [rv | TableRow _ rv <- keep] of
          [rv] -> rv
          _ -> Selection (TableValue tt r') v
      where
        -- Don't introduce wildcard patterns, true to the canonical format,
        -- annotate (or eliminate) rhs in impossible rows
        r' = map trunc r
        trunc r@(TableRow p e) = if mightMatchRow v r
                                 then r
                                 else TableRow p (impossible e)
        {-
        -- Creates smaller tables, but introduces wildcard patterns
        r' = if null discard
             then r
             else keep++[TableRow WildPattern impossible]
        -}
        (keep,discard) = partition (mightMatchRow v) r
    _ -> Selection t v

impossible :: LinValue -> LinValue
impossible = CommentedValue "impossible"

mightMatchRow :: LinValue -> TableRow rhs -> Bool
mightMatchRow v (TableRow p _) =
  case p of
    WildPattern -> True
    _ -> mightMatch v p

mightMatch :: LinValue -> LinPattern -> Bool
mightMatch v p =
  case v of
    ConcatValue _ _ -> False
    ParamConstant (Param c1 pvs) ->
      case p of
        ParamPattern (Param c2 pps) -> c1==c2 && length pvs==length pps &&
                                       and [mightMatch v p|(v,p)<-zip pvs pps]
        _ -> False
    RecordValue rv ->
      case p of
        RecordPattern rp ->
          and [maybe False (`mightMatch` p) (proj v l) | RecordRow l p<-rp]
        _ -> False
    _ -> True

patVars :: Patt -> [Ident]
patVars p =
  case p of
    PV x -> [x]
    PAs x p -> x:patVars p
    _ -> collectPattOp patVars p

convType :: Term -> LinType
convType = ppT
  where
    ppT t =
      case t of
        Table ti tv -> TableType (ppT ti) (ppT tv)
        RecType rt -> RecordType (convFields rt)
--      App tf ta -> TAp (ppT tf) (ppT ta)
--      FV [] -> tcon0 (identS "({-empty variant-})")
        Sort k -> convSort k
--      EInt n -> tcon0 (identS ("({-"++show n++"-})")) -- type level numeric literal
        FV (t:ts) -> ppT t -- !!
        QC (m,n) -> ParamType (ParamTypeId (gQId m n))
        Q (m,n) -> ParamType (ParamTypeId (gQId m n))
        _ -> error $ "convType ppT: " ++ show t

    convFields = map convField . filter (not.isLockLabel.fst)
    convField (l,r) = RecordRow (lblId l) (ppT r)

    convSort k = case showIdent k of
                   "Float" -> FloatType
                   "Int" -> IntType
                   "Str" -> StrType
                   _ -> error $ "convType convSort: " ++ show k

toParamType :: Term -> ParamType
toParamType t = case convType t of
                  ParamType pt -> pt
                  _ -> error $ "toParamType: " ++ show t

toParamId :: Term -> ParamId
toParamId t = case toParamType t of
                   ParamTypeId p -> p

paramType :: G.Grammar
             -> (ModuleName, Ident)
             -> ((S.Set (ModuleName, Ident), S.Set QIdent), [ParamDef])
paramType gr q@(_,n) =
    case lookupOrigInfo gr q of
      Ok (m,ResParam (Just (L _ ps)) _)
       {- - | m/=cPredef && m/=moduleNameS "Prelude"-} ->
         ((S.singleton (m,n),argTypes ps),
          [ParamDef name (map (param m) ps)]
         )
       where name = gQId m n
      Ok (m,ResOper  _ (Just (L _ t)))
        | m==cPredef && n==cInts ->
           ((S.empty,S.empty),[]) {-
           ((S.singleton (m,n),S.empty),
            [Type (ConAp ((gQId m n)) [identS "n"]) (TId (identS "Int"))])-}
        | otherwise ->
           ((S.singleton (m,n),paramTypes gr t),
            [ParamAliasDef (gQId m n) (convType t)])
      _ -> ((S.empty,S.empty),[])
  where
    param m (n,ctx) = Param (gQId m n) [toParamId t|(_,_,t)<-ctx]
    argTypes = S.unions . map argTypes1
    argTypes1 (n,ctx) = S.unions [paramTypes gr t|(_,_,t)<-ctx]

lblId :: Label -> C.LabelId
lblId (LIdent ri) = LabelId ri
lblId (LVar i) = LabelId (rawIdentS (show i)) -- hmm

modId :: ModuleName -> C.ModId
modId (MN m) = ModId (ident2raw m)

class FromIdent i where
  gId :: Ident -> i

instance FromIdent VarId where
  gId i = if isWildIdent i then Anonymous else VarId (ident2raw i)

instance FromIdent C.FunId where gId = C.FunId . ident2raw
instance FromIdent CatId where gId = CatId . ident2raw
instance FromIdent ParamId where gId = ParamId . unqual
instance FromIdent VarValueId where gId = VarValueId . unqual

class FromIdent i => QualIdent i where
  gQId :: ModuleName -> Ident -> i

instance QualIdent ParamId where gQId m n = ParamId (qual m n)
instance QualIdent VarValueId where gQId m n = VarValueId (qual m n)

qual :: ModuleName -> Ident -> QualId
qual m n = Qual (modId m) (ident2raw n)

unqual :: Ident -> QualId
unqual n = Unqual (ident2raw n)

convFlags :: G.Grammar -> ModuleName -> Flags
convFlags gr mn =
  Flags [(rawIdentS n,v) |
         (n,v)<-err (const []) (optionsPGF.mflags) (lookupModule gr mn)]
