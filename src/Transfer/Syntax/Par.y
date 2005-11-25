-- This Happy file was machine-generated by the BNF converter
{
module Transfer.Syntax.Par where
import Transfer.Syntax.Abs
import Transfer.Syntax.Lex
import Transfer.ErrM
}

%name pModule Module
%name pExp Exp

-- no lexer declaration
%monad { Err } { thenM } { returnM }
%tokentype { Token }

%token 
 ';' { PT _ (TS ";") }
 ':' { PT _ (TS ":") }
 '{' { PT _ (TS "{") }
 '}' { PT _ (TS "}") }
 '=' { PT _ (TS "=") }
 '(' { PT _ (TS "(") }
 ')' { PT _ (TS ")") }
 '_' { PT _ (TS "_") }
 '->' { PT _ (TS "->") }
 '\\' { PT _ (TS "\\") }
 '||' { PT _ (TS "||") }
 '&&' { PT _ (TS "&&") }
 '==' { PT _ (TS "==") }
 '/=' { PT _ (TS "/=") }
 '<' { PT _ (TS "<") }
 '<=' { PT _ (TS "<=") }
 '>' { PT _ (TS ">") }
 '>=' { PT _ (TS ">=") }
 '+' { PT _ (TS "+") }
 '-' { PT _ (TS "-") }
 '*' { PT _ (TS "*") }
 '/' { PT _ (TS "/") }
 '%' { PT _ (TS "%") }
 '.' { PT _ (TS ".") }
 'Type' { PT _ (TS "Type") }
 'case' { PT _ (TS "case") }
 'data' { PT _ (TS "data") }
 'derive' { PT _ (TS "derive") }
 'else' { PT _ (TS "else") }
 'if' { PT _ (TS "if") }
 'import' { PT _ (TS "import") }
 'in' { PT _ (TS "in") }
 'let' { PT _ (TS "let") }
 'of' { PT _ (TS "of") }
 'then' { PT _ (TS "then") }
 'where' { PT _ (TS "where") }

L_ident  { PT _ (TV $$) }
L_quoted { PT _ (TL $$) }
L_integ  { PT _ (TI $$) }
L_err    { _ }


%%

Ident   :: { Ident }   : L_ident  { Ident $1 }
String  :: { String }  : L_quoted { $1 }
Integer :: { Integer } : L_integ  { (read $1) :: Integer }

Module :: { Module }
Module : ListImport ListDecl { Module $1 $2 } 


Import :: { Import }
Import : 'import' Ident { Import $2 } 


ListImport :: { [Import] }
ListImport : {- empty -} { [] } 
  | Import { (:[]) $1 }
  | Import ';' ListImport { (:) $1 $3 }


Decl :: { Decl }
Decl : 'data' Ident ':' Exp 'where' '{' ListConsDecl '}' { DataDecl $2 $4 $7 } 
  | Ident ':' Exp { TypeDecl $1 $3 }
  | Ident ListPattern '=' Exp { ValueDecl $1 (reverse $2) $4 }
  | 'derive' Ident Ident { DeriveDecl $2 $3 }


ListDecl :: { [Decl] }
ListDecl : {- empty -} { [] } 
  | Decl { (:[]) $1 }
  | Decl ';' ListDecl { (:) $1 $3 }


ConsDecl :: { ConsDecl }
ConsDecl : Ident ':' Exp { ConsDecl $1 $3 } 


ListConsDecl :: { [ConsDecl] }
ListConsDecl : {- empty -} { [] } 
  | ConsDecl { (:[]) $1 }
  | ConsDecl ';' ListConsDecl { (:) $1 $3 }


Pattern :: { Pattern }
Pattern : Ident Pattern1 ListPattern { PConsTop $1 $2 (reverse $3) } 
  | Pattern1 { $1 }


Pattern1 :: { Pattern }
Pattern1 : '(' Ident ListPattern ')' { PCons $2 (reverse $3) } 
  | '{' ListFieldPattern '}' { PRec $2 }
  | 'Type' { PType }
  | String { PStr $1 }
  | Integer { PInt $1 }
  | Ident { PVar $1 }
  | '_' { PWild }


ListPattern :: { [Pattern] }
ListPattern : {- empty -} { [] } 
  | ListPattern Pattern1 { flip (:) $1 $2 }


FieldPattern :: { FieldPattern }
FieldPattern : Ident '=' Pattern { FieldPattern $1 $3 } 


ListFieldPattern :: { [FieldPattern] }
ListFieldPattern : {- empty -} { [] } 
  | FieldPattern { (:[]) $1 }
  | FieldPattern ';' ListFieldPattern { (:) $1 $3 }


Exp :: { Exp }
Exp : 'let' '{' ListLetDef '}' 'in' Exp { ELet $3 $6 } 
  | 'case' Exp 'of' '{' ListCase '}' { ECase $2 $5 }
  | 'if' Exp 'then' Exp 'else' Exp { EIf $2 $4 $6 }
  | Exp1 { $1 }


LetDef :: { LetDef }
LetDef : Ident ':' Exp '=' Exp { LetDef $1 $3 $5 } 


ListLetDef :: { [LetDef] }
ListLetDef : {- empty -} { [] } 
  | LetDef { (:[]) $1 }
  | LetDef ';' ListLetDef { (:) $1 $3 }


Case :: { Case }
Case : Pattern '->' Exp { Case $1 $3 } 


ListCase :: { [Case] }
ListCase : {- empty -} { [] } 
  | Case { (:[]) $1 }
  | Case ';' ListCase { (:) $1 $3 }


Exp2 :: { Exp }
Exp2 : '\\' VarOrWild '->' Exp { EAbs $2 $4 } 
  | '(' VarOrWild ':' Exp ')' '->' Exp { EPi $2 $4 $7 }
  | Exp3 '->' Exp { EPiNoVar $1 $3 }
  | Exp3 { $1 }


VarOrWild :: { VarOrWild }
VarOrWild : Ident { VVar $1 } 
  | '_' { VWild }


Exp3 :: { Exp }
Exp3 : Exp4 '||' Exp3 { EOr $1 $3 } 
  | Exp4 { $1 }


Exp4 :: { Exp }
Exp4 : Exp5 '&&' Exp4 { EAnd $1 $3 } 
  | Exp5 { $1 }


Exp5 :: { Exp }
Exp5 : Exp6 '==' Exp6 { EEq $1 $3 } 
  | Exp6 '/=' Exp6 { ENe $1 $3 }
  | Exp6 '<' Exp6 { ELt $1 $3 }
  | Exp6 '<=' Exp6 { ELe $1 $3 }
  | Exp6 '>' Exp6 { EGt $1 $3 }
  | Exp6 '>=' Exp6 { EGe $1 $3 }
  | Exp6 { $1 }


Exp6 :: { Exp }
Exp6 : Exp6 '+' Exp7 { EAdd $1 $3 } 
  | Exp6 '-' Exp7 { ESub $1 $3 }
  | Exp7 { $1 }


Exp7 :: { Exp }
Exp7 : Exp7 '*' Exp8 { EMul $1 $3 } 
  | Exp7 '/' Exp8 { EDiv $1 $3 }
  | Exp7 '%' Exp8 { EMod $1 $3 }
  | Exp8 { $1 }


Exp8 :: { Exp }
Exp8 : Exp8 '.' Ident { EProj $1 $3 } 
  | Exp9 { $1 }


Exp9 :: { Exp }
Exp9 : '-' Exp9 { ENeg $2 } 
  | Exp10 { $1 }


Exp10 :: { Exp }
Exp10 : Exp10 Exp11 { EApp $1 $2 } 
  | Exp11 { $1 }


Exp11 :: { Exp }
Exp11 : '{' '}' { EEmptyRec } 
  | '{' ListFieldType '}' { ERecType $2 }
  | '{' ListFieldValue '}' { ERec $2 }
  | Ident { EVar $1 }
  | 'Type' { EType }
  | String { EStr $1 }
  | Integer { EInt $1 }
  | '(' Exp ')' { $2 }


FieldType :: { FieldType }
FieldType : Ident ':' Exp { FieldType $1 $3 } 


ListFieldType :: { [FieldType] }
ListFieldType : FieldType { (:[]) $1 } 
  | FieldType ';' ListFieldType { (:) $1 $3 }


FieldValue :: { FieldValue }
FieldValue : Ident '=' Exp { FieldValue $1 $3 } 


ListFieldValue :: { [FieldValue] }
ListFieldValue : FieldValue { (:[]) $1 } 
  | FieldValue ';' ListFieldValue { (:) $1 $3 }


Exp1 :: { Exp }
Exp1 : Exp2 { $1 } 



{

returnM :: a -> Err a
returnM = return

thenM :: Err a -> (a -> Err b) -> Err b
thenM = (>>=)

happyError :: [Token] -> Err a
happyError ts =
  Bad $ "syntax error at " ++ tokenPos ts ++ if null ts then [] else (" before " ++ unwords (map prToken (take 4 ts)))

myLexer = tokens
}

