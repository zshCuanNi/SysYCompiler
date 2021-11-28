%{
#include "ast.hpp"
#include "symtab.hpp"
#include <string>
#include <stdio.h>

extern FILE *logOut;
int yyparse(void);
int yylex(void);
int yywrap() { return 1; }
// should revise to my version
void yyerror(const char *msg, int lineno = yylineno) {
    fprintf(stderr, "Error detected at line %d: %s\n", lineno, msg);
}
%}

%union {
    Node *node;
    NodeVarDecl *vardecl;
    NodeFuncDecl *funcdecl;
    NodeStatement *stmt;
    NodeExpression *exp;
    NodeConditionExp *cond;
    int number;
    const char *string;
    ReturnType rettype;
};

%token T_EQ T_LEQ T_GEQ T_NEQ T_AND T_OR T_VOID T_INT T_IF T_ELSE T_WHILE T_BREAK T_CONTINUE T_RETURN T_CONST

%token<number> T_DECIMAL_CONST

%token<string> T_IDENTIFIER

%type<node> Root CompUnit Block BlockItems BlockItem

%type<vardecl> Decl ConstDecl ConstDefList ConstDef VarDecl VarDefList VarDef FuncFParams FuncFParam 

%type<funcdecl> FuncDef 

%type<stmt> Stmt FakeIf FakeWhile

%type<exp> ConstWidthList ConstInitVal ConstInitVals InitVal InitVals Exp LVal WidthList PrimaryExp UnaryExp FuncRParams MulExp AddExp RelExp EqExp ConstExp

%type<cond> Cond LAndExp LOrExp

%type<rettype> FuncType


%%

Root
    : CompUnit                          { ASTRoot = $$ = $1; }
CompUnit
    : Decl                              { $$ = new Node(); $$->son = $1; $$->code = $1->code; }
    | FuncDef                           { $$ = $1; }
    | CompUnit Decl                     { $1->setBrother($2); $$ = $1; }
    | CompUnit FuncDef                  { $1->setBrother($2); $$ = $1; }

Decl
    : ConstDecl                         { $$ = $1; }
    | VarDecl                           { $$ = $1; }

ConstDecl
    : T_CONST FuncType ConstDefList ';' { $$ = $3; }
ConstDefList
    : ConstDef                          { $$ = $1; }
    | ConstDef ',' ConstDefList         { $1->setBrother($3); $$ = $1; }
ConstDef
    : T_IDENTIFIER ConstWidthList '=' ConstInitVal
                                        { $$ = new NodeVarDecl($1, $2, $4, true); }
ConstWidthList
    : '[' ConstExp ']' ConstWidthList   { $2->setBrother($4); $$ = $2; }
    |                                   { $$ = nullptr; }
ConstInitVal
    : ConstExp                          { $$ = $1; }
    | '{' ConstInitVals '}'             { $$ = new NodeExpression(tINIT, $2); }
    | '{' '}'                           { $$ = new NodeExpression(tINIT); }
ConstInitVals
    : ConstInitVal                      { $$ = $1; }
    | ConstInitVal ',' ConstInitVals    { $1->setBrother($3); $$ = $1; }

VarDecl
    : FuncType VarDefList ';'           { $$ = $2; }
VarDefList
    : VarDef                            { $$ = $1; }
    | VarDef ',' VarDefList             { $1->setBrother($3); $$ = $1; }
VarDef
    : T_IDENTIFIER ConstWidthList '=' InitVal
                                        { $$ = new NodeVarDecl($1, $2, $4); }
    | T_IDENTIFIER ConstWidthList       { $$ = new NodeVarDecl($1, $2); }
InitVal
    : Exp                               { $$ = $1; }
    | '{' InitVals '}'                  { $$ = new NodeExpression(tINIT, $2); }
    | '{' '}'                           { $$ = new NodeExpression(tINIT); }
InitVals
    : InitVal                           { $$ = $1; }
    | InitVal ',' InitVals              { $1->setBrother($3); $$ = $1; }
FuncDef
    : FuncType T_IDENTIFIER '('         { symTab.registerFunc($2, $1); 
                                          symTab.blockInc();
                                          symTab.create = 0; 
                                          /* understand this part later */}
      FuncFParams ')' Block             { $$ = new NodeFuncDecl($1, $2, $7); }
FuncFParams
    : FuncFParam                        { /* skip */ }
    | FuncFParam ',' FuncFParams        { /* skip */ }
    |                                   { /* ignore */ }
FuncFParam
    : FuncType T_IDENTIFIER             { $$ = new NodeVarDecl($2, nullptr, nullptr, false, false, true); }
    | FuncType T_IDENTIFIER '[' ']' ConstWidthList
                                        { $$ = new NodeVarDecl($2, $5, nullptr, false, true, true); }
FuncType
    : T_VOID                            { $$ = rtVOID; }
    | T_INT                             { $$ = rtINT; }

Block
    : '{'                               { symTab.blockInc(); }
      BlockItems                        { symTab.blockDec(); }
      '}'                               { $$ = $3; }
BlockItems
    : BlockItem BlockItems              { $1->setBrother($2); $$ = $1; }
    |                                   { $$ = nullptr; }
BlockItem
    : Decl                              { $$ = $1; }
    | Stmt                              { $$ = $1; }

Stmt
    : LVal '=' Exp ';'                  { $$ = new NodeAssignStmt($1, $3); }
    | ';'                               { $$ = new NodeStatement(); }
    | Exp ';'                           { $$ = new NodeExpressionStmt($1); }
    | Block                             { $$ = new NodeStatement($1); }
    | T_IF '(' FakeIf Cond ')' Stmt
                                        { $3->setMember($4, $6); $$ = $3; }
    | T_IF '(' FakeIf Cond ')' Stmt T_ELSE Stmt
                                        { $3->setMember($4, $6, $8); $$ = $3; }
    | T_WHILE '(' FakeWhile Cond ')' Stmt
                                        { $3->setMember($4, $6); $$ = $3; }
    | T_BREAK ';'                       { $$ = new NodeBreakStmt(); }
    | T_CONTINUE ';'                    { $$ = new NodeContinueStmt(); }
    | T_RETURN ';'                      { $$ = new NodeReturnStmt(); }
    | T_RETURN Exp ';'                  { $$ = new NodeReturnStmt($2); }
FakeIf
    :                                   { $$ = new NodeIfElseStmt(); }
FakeWhile
    :                                   { $$ = new NodeWhileStmt(); }

                
Exp             
    : AddExp                            { $$ = $1; }
Cond            
    :                                   { conditionFlag = true; }
      LOrExp                            { conditionFlag = false; $$ = $2; }
LVal            
    : T_IDENTIFIER WidthList            { $$ = new NodeArrayExp($1, $2); }
WidthList
    : '[' Exp ']' WidthList             { $2->setBrother($4); $$ = $2; }
    |                                   { $$ = nullptr; }
PrimaryExp      
    : '(' Exp ')'                       { $$ = $2; }
    | LVal                              { $$ = $1; }
    | T_DECIMAL_CONST                   { $$ = new NodeExpression(tNUM, nullptr, $1); }
UnaryExp        
    : PrimaryExp                        { $$ = $1; }
    | T_IDENTIFIER '(' FuncRParams ')'  { $$ = new NodeFunctionCallExp($1, $3); }
    | '+' UnaryExp                      { $$ = $2; }
    | '-' UnaryExp                      { $$ = new NodeArithmeticExp(oNEG, $2); }
    | '!' UnaryExp                      { $$ = new NodeArithmeticExp(oNOT, $2); }
FuncRParams     
    : Exp                               { $$ = $1; }
    | Exp ',' FuncRParams               { $1->setBrother($3); $$ = $1; }
    |                                   { $$ = nullptr; }
MulExp          
    : UnaryExp                          { $$ = $1; }
    | MulExp '*' UnaryExp               { $$ = new NodeArithmeticExp(oMUL, $1, $3); }
    | MulExp '/' UnaryExp               { $$ = new NodeArithmeticExp(oDIV, $1, $3); }
    | MulExp '%' UnaryExp               { $$ = new NodeArithmeticExp(oMOD, $1, $3); }
AddExp          
    : MulExp                            { $$ = $1; }
    | AddExp '+' MulExp                 { $$ = new NodeArithmeticExp(oADD, $1, $3); }
    | AddExp '-' MulExp                 { $$ = new NodeArithmeticExp(oSUB, $1, $3); }
RelExp          
    : AddExp                            { $$ = $1; }
    | RelExp '<' AddExp                 { $$ = new NodeArithmeticExp(oLESS, $1, $3); }
    | RelExp '>' AddExp                 { $$ = new NodeArithmeticExp(oGREATER, $1, $3); }
    | RelExp T_LEQ AddExp               { $$ = new NodeArithmeticExp(oLEQ, $1, $3); }
    | RelExp T_GEQ AddExp               { $$ = new NodeArithmeticExp(oGEQ, $1, $3); }
EqExp           
    : RelExp                            { $$ = $1; }
    | EqExp T_EQ RelExp                 { $$ = new NodeArithmeticExp(oEQ, $1,  $3); }
    | EqExp T_NEQ RelExp                { $$ = new NodeArithmeticExp(oNEQ, $1,  $3); }
LAndExp         
    : EqExp                             { $$ = new NodeConditionExp(oNONE, $1); }
    | LAndExp T_AND EqExp               { $$ = new NodeConditionExp(oLAND, $1, new NodeConditionExp(oNONE, $3)); }
LOrExp          
    : LAndExp                           { $$ = $1; }
    | LOrExp T_OR LAndExp               { $$ = new NodeConditionExp(oLOR, $1, $3); }
ConstExp        
    : AddExp                            { $1->constantFold(); $$ = $1; }

%%
