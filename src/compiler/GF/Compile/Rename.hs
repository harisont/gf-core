----------------------------------------------------------------------
-- |
-- Module      : Rename
-- Maintainer  : AR
-- Stability   : (stable)
-- Portability : (portable)
--
-- > CVS $Date: 2005/05/30 18:39:44 $
-- > CVS $Author: aarne $
-- > CVS $Revision: 1.19 $
--
-- AR 14\/5\/2003
-- The top-level function 'renameGrammar' does several things:
--
--   - extends each module symbol table by indirections to extended module
--
--   - changes unqualified and as-qualified imports to absolutely qualified
--
--   - goes through the definitions and resolves names
--
-- Dependency analysis between modules has been performed before this pass.
-- Hence we can proceed by @fold@ing "from left to right".
-----------------------------------------------------------------------------

module GF.Compile.Rename (
     renameSourceTerm,
     renameModule
    ) where

import GF.Infra.Ident
import GF.Infra.CheckM
import GF.Grammar.Grammar
import GF.Grammar.Values
import GF.Grammar.Predef
import GF.Grammar.Lookup
import GF.Grammar.Macros
import GF.Grammar.Printer
import GF.Data.Operations

import Control.Monad
import Data.List (nub,(\\))
import qualified Data.List as L
import qualified Data.Map as Map
import Data.Maybe(mapMaybe)
import GF.Text.Pretty

-- | this gives top-level access to renaming term input in the cc command
renameSourceTerm :: Grammar -> ModuleName -> Term -> Check Term
renameSourceTerm g m t = do
  mi     <- lookupModule g m
  status <- buildStatus "" g (m,mi)
  renameTerm status [] t

renameModule :: FilePath -> Grammar -> Module -> Check Module
renameModule cwd gr mo@(m,mi) = do
  status <- buildStatus cwd gr mo
  js     <- checkMapRecover (renameInfo cwd status mo) (jments mi)
  return (m, mi{jments = js})

type Status = (StatusMap, [(OpenSpec, StatusMap)])

type StatusMap = Map.Map Ident StatusInfo

type StatusInfo = Ident -> Term

-- Delays errors, allowing many errors to be detected and reported
renameIdentTerm env = accumulateError (renameIdentTerm' env)

-- Fails immediately on error, makes it possible to try other possibilities
renameIdentTerm' :: Status -> Term -> Check Term
renameIdentTerm' env@(act,imps) t0 =
  case t0 of
    Vr c -> ident predefAbs c
    Cn c -> ident (\_ s -> checkError s) c
    Q (m',c) | m' == cPredef {- && isInPredefined c -} -> return t0
    Q (m',c) -> do
      m <- lookupErr m' qualifs
      f <- lookupIdent c m
      return $ f c
    QC (m',c) | m' == cPredef {- && isInPredefined c -} -> return t0
    QC (m',c) -> do
      m <- lookupErr m' qualifs
      f <- lookupIdent c m
      return $ f c
    _ -> return t0
  where
    opens   = [st  | (OSimple _,st) <- imps]
    qualifs = [(m, st) | (OQualif m _, st) <- imps] ++
              [(m, st) | (OQualif _ m, st) <- imps] ++
              [(m, st) | (OSimple m, st) <- imps] -- qualif is always possible

    -- this facility is mainly for BWC with GF1: you need not import PredefAbs
    predefAbs c s
      | isPredefCat c = return (Q (cPredefAbs,c))
      | otherwise     = checkError s

    ident alt c =
      case Map.lookup c act of
        Just f -> return (f c)
        _      -> case mapMaybe (Map.lookup c) opens of
                    [f]  -> return (f c)
                    []   -> alt c ("constant not found:" <+> c $$
                                   "given" <+> fsep (punctuate ',' (map fst qualifs)))
                    fs   -> case nub [f c | f <- fs]  of
                              [tr]     -> return tr
                              ts@(t:_) -> do checkWarn ("atomic term" <+> ppTerm Qualified 0 t0 $$
                                                        "conflict" <+> hsep (punctuate ',' (map (ppTerm Qualified 0) ts)) $$
                                                        "given" <+> fsep (punctuate ',' (map fst qualifs)))
                                             return (bestTerm ts) -- Heuristic for resource grammar. Returns t for all others.
       where
        -- Hotfix for https://github.com/GrammaticalFramework/gf-core/issues/56
        -- Real bug is probably somewhere deeper in recognising excluded functions. /IL 2020-06-06
        notFromCommonModule :: Term -> Bool
        notFromCommonModule term =
          let t = render $ ppTerm Qualified 0 term :: String
           in not $ any (\moduleName -> moduleName `L.isPrefixOf` t)
                        ["CommonX", "ConstructX", "ExtendFunctor"
                        ,"MarkHTMLX", "ParamX", "TenseX", "TextX"]

        -- If one of the terms comes from the common modules,
        -- we choose the other one, because that's defined in the grammar.
        bestTerm :: [Term] -> Term
        bestTerm [] = error "constant not found" -- not reached: bestTerm is only called for case ts@(t:_)
        bestTerm ts@(t:_) =
          let notCommon = [t | t <- ts, notFromCommonModule t]
           in case notCommon of
                []    -> t -- All terms are from common modules, return first of original list
                (u:_) -> u -- ≥1 terms are not from common modules, return first of those

info2status :: Maybe ModuleName -> Ident -> Info -> StatusInfo
info2status mq c i = case i of
  AbsFun _ _ Nothing _ -> maybe Con (curry QC) mq
  ResValue _  -> maybe Con (curry QC) mq
  ResParam _ _  -> maybe Con (curry QC) mq
  AnyInd True m -> maybe Con (const (curry QC m)) mq
  AnyInd False m -> maybe Cn (const (curry Q m)) mq
  _           -> maybe Cn (curry Q) mq

tree2status :: OpenSpec -> Map.Map Ident Info -> StatusMap
tree2status o = case o of
  OSimple i   -> Map.mapWithKey (info2status (Just i))
  OQualif i j -> Map.mapWithKey (info2status (Just j))

buildStatus :: FilePath -> Grammar -> Module -> Check Status
buildStatus cwd gr mo@(m,mi) = checkInModule cwd mi NoLoc empty $ do
  let gr1  = prependModule gr mo
      exts = [(OSimple m,mi) | (m,mi) <- allExtends gr1 m]
  ops <- mapM (\o -> lookupModule gr1 (openedModule o) >>= \mi -> return (o,mi)) (mopens mi)
  let sts = map modInfo2status (exts++ops)
  return (if isModCnc mi
            then (Map.empty,       reverse sts)  -- the module itself does not define any names
            else (self2status m mi,reverse sts)) -- so the empty ident is not needed

modInfo2status :: (OpenSpec,ModuleInfo) -> (OpenSpec, StatusMap)
modInfo2status (o,mo) = (o,tree2status o (jments mo))

self2status :: ModuleName -> ModuleInfo -> StatusMap
self2status c m = Map.mapWithKey (info2status (Just c)) (jments m)


renameInfo :: FilePath -> Status -> Module -> Ident -> Info -> Check Info
renameInfo cwd status (m,mi) i info =
  case info of
    AbsCat pco -> liftM AbsCat (renPerh (renameContext status) pco)
    AbsFun  pty pa ptr poper -> liftM4 AbsFun (renTerm pty) (return pa) (renMaybe (mapM (renLoc (renEquation status))) ptr) (return poper)
    ResOper pty ptr -> liftM2 ResOper (renTerm pty) (renTerm ptr)
    ResOverload os tysts -> liftM (ResOverload os) (mapM (renPair (renameTerm status [])) tysts)
    ResParam (Just pp) m -> do
      pp' <- renLoc (mapM (renParam status)) pp
      return (ResParam (Just pp') m)
    ResValue t -> do
      t <- renLoc (renameTerm status []) t
      return (ResValue t)
    CncCat mcat mdef mref mpr mpmcfg -> liftM5 CncCat (renTerm mcat) (renTerm mdef) (renTerm mref) (renTerm mpr) (return mpmcfg)
    CncFun mty mtr mpr mpmcfg -> liftM3 (CncFun mty)         (renTerm mtr) (renTerm mpr) (return mpmcfg)
    _ -> return info
  where
    renTerm = renPerh (renameTerm status [])

    renPerh ren = renMaybe (renLoc ren)

    renMaybe ren (Just x) = ren x >>= return . Just
    renMaybe ren Nothing  = return Nothing

    renLoc ren (L loc x) =
      checkInModule cwd mi loc ("Happened in the renaming of" <+> i) $ do
        x <- ren x
        return (L loc x)

    renPair ren (x, y) = do x <- renLoc ren x
                            y <- renLoc ren y
                            return (x, y)

    renEquation :: Status -> Equation -> Check Equation
    renEquation b (ps,t) = do
        (ps',vs) <- liftM unzip $ mapM (renamePattern b) ps
        t'       <- renameTerm b (concat vs) t
        return (ps',t')

    renParam :: Status -> Param -> Check Param
    renParam env (c,co) = do
      co' <- renameContext env co
      return (c,co')

renameTerm :: Status -> [Ident] -> Term -> Check Term
renameTerm env vars = ren vars where
  ren vs trm = case trm of
    Abs b x t    -> liftM  (Abs b x) (ren (x:vs) t)
    Prod bt x a b -> liftM2 (Prod bt x) (ren vs a) (ren (x:vs) b)
    Typed a b  -> liftM2 Typed (ren vs a) (ren vs b)
    Vr x
      | elem x vs -> return trm
      | otherwise -> renid trm
    Cn _   -> renid trm
    Con _  -> renid trm
    Q _    -> renid trm
    QC _   -> renid trm
    T i cs -> do
      i' <- case i of
        TTyped ty -> liftM TTyped $ ren vs ty -- the only annotation in source
        _ -> return i
      liftM (T i') $ mapM (renCase vs) cs

    Let (x,(m,a)) b -> do
      m' <- case m of
        Just ty -> liftM Just $ ren vs ty
        _ -> return m
      a' <- ren vs a
      b' <- ren (x:vs) b
      return $ Let (x,(m',a')) b'

    P t@(Vr r) l                                               -- Here we have $r.l$ and this is ambiguous it could be either
                                                               -- record projection from variable or constant $r$ or qualified expression with module $r$
      | elem r vs -> return trm                                -- try var proj first ..
      | otherwise -> checks [ renid' (Q (MN r,label2ident l))      -- .. and qualified expression second.
                            , renid' t >>= \t -> return (P t l) -- try as a constant at the end
                            , checkError ("unknown qualified constant" <+> trm)
                            ]

    EPatt minp maxp p -> do
      (p',_) <- renpatt p
      return $ EPatt minp maxp p'

    _ -> composOp (ren vs) trm

  renid = renameIdentTerm env
  renid' = renameIdentTerm' env
  renCase vs (p,t) = do
    (p',vs') <- renpatt p
    t' <- ren (vs' ++ vs) t
    return (p',t')
  renpatt = renamePattern env

-- | vars not needed in env, since patterns always overshadow old vars
renamePattern :: Status -> Patt -> Check (Patt,[Ident])
renamePattern env patt =
    do r@(p',vs) <- renp patt
       let dupl = vs \\ nub vs
       unless (null dupl) $ checkError (hang ("[C.4.13] Pattern is not linear. All variable names on the left-hand side must be distinct.") 4
                                             patt)
       return r
  where
    renp patt = case patt of
      PMacro c -> do
        c' <- renid $ Vr c
        case c' of
          Q d -> renp $ PM d
          _ -> checkError ("unresolved pattern" <+> patt)

      PC c ps -> do
        c' <- renid $ Cn c
        case c' of
          QC c -> do psvss <- mapM renp ps
                     let (ps,vs) = unzip psvss
                     return (PP c ps, concat vs)
          Q  _ -> checkError ("data constructor expected but" <+> ppTerm Qualified 0 c' <+> "is found instead")
          _    -> checkError ("unresolved data constructor" <+> ppTerm Qualified 0 c')

      PP c ps -> do
        (QC c') <- renid (QC c)
        psvss <- mapM renp ps
        let (ps',vs) = unzip psvss
        return (PP c' ps', concat vs)

      PM c -> do
        x <- renid (Q c)
        c' <- case x of
                (Q c') -> return c'
                _      -> checkError ("not a pattern macro" <+> ppPatt Qualified 0 patt)
        return (PM c', [])

      PV x -> checks [ renid' (Vr x) >>= \t' -> case t' of
                                                 QC c -> return (PP c [],[])
                                                 _    -> checkError (pp "not a constructor")
                     , return (patt, [x])
                     ]

      PR r -> do
        let (ls,ps) = unzip r
        psvss <- mapM renp ps
        let (ps',vs') = unzip psvss
        return (PR (zip ls ps'), concat vs')

      PAlt p q -> do
        (p',vs) <- renp p
        (q',ws) <- renp q
        return (PAlt p' q', vs ++ ws)

      PSeq minp maxp p minq maxq q -> do
        (p',vs) <- renp p
        (q',ws) <- renp q
        return (PSeq minp maxp p' minq maxq q', vs ++ ws)

      PRep minp maxp p -> do
        (p',vs) <- renp p
        return (PRep minp maxp p', vs)

      PNeg p -> do
        (p',vs) <- renp p
        return (PNeg p', vs)

      PAs x p -> do
        (p',vs) <- renp p
        return (PAs x p', x:vs)

      _ -> return (patt,[])

    renid = renameIdentTerm env
    renid' = renameIdentTerm' env

renameContext :: Status -> Context -> Check Context
renameContext b = renc [] where
  renc vs cont = case cont of
    (bt,x,t) : xts
      | isWildIdent x -> do
          t'   <- ren vs t
          xts' <- renc vs xts
          return $ (bt,x,t') : xts'
      | otherwise -> do
          t'   <- ren vs t
          let vs' = x:vs
          xts' <- renc vs' xts
          return $ (bt,x,t') : xts'
    _ -> return cont
  ren = renameTerm b
