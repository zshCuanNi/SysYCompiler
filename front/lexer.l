%{
#define YYSTYPE char *
#include "token.h"
int curLine = 1;
int lval;
void init_scanner();
void lex_error(char* msg, int line);
// #define _DUPTEXT {yylval = strdup(yytext);}
%}

white_space             ([ \t\r\a]+)
include_statement       ("#"[^\n]*)
single_line_comment     ("//"[^\n]*)
multi_line_comment      ("/*"([^*]*[*]+[^*/])*[^*]*[*]+"/")
operator                ([+*-/%=,;!<>(){}])
identifier              ([_a-zA-Z][_a-zA-Z0-9]*)
decimal_const           ([1-9][0-9]*)
octal_const             (0[0-7]*)
hexadecimal_const       (0[xX][0-9a-fA-F]+)

%%

\n                      { curLine++; }
{white_space}           { /* ignore white spaces */ }
{include_statement}     { /* ignore include statements */ }
{multi_line_comment}    { /* skip multi-line comments */ }
{single_line_comment}   { /* skip single line comments */ }

{operator}              { return yytext[0]; }

"=="                    { return T_EQ; }
"<="                    { return T_LEQ; }
">="                    { return T_GEQ; }
"!="                    { return T_NEQ; }
"&&"                    { return T_AND; }
"||"                    { return T_OR; }
"void"                  { return T_VOID; }
"int"                   { return T_INT; }
"if"                    { return T_IF; }
"else"                  { return T_ELSE; }
"while"                 { return T_WHILE; }
"break"                 { return T_BREAK; }
"continue"              { return T_CONTINUE; }
"return"                { return T_RETURN; }
"const"                 { return T_CONST; }

{identifier}            { lval = strdup(yytext); return T_IDENTIFIER; }
{hexadecimal_const}     { lval = strtol(yytext, NULL, 0); return T_DECIMAL_CONST; }
{decimal_const}         { lval = strtol(yytext, NULL, 0); return T_DECIMAL_CONST; }
{octal_const}           { lval = strtol(yytext, NULL, 0); return T_DECIMAL_CONST; }

.                       { lex_error("Unrecognized character", curLine); }

%%

#define YYSTYPE char *
#include "token.h"

int main(int argc, char* argv[]) {
    int token;
    init_scanner();
    while (token = yylex()) {
        print_token(token);
        if (token != T_DECIMAL_CONST)
            puts(yytext);
        else 
            printf("%d\n", lval);
    }
    return 0;
}

void init_scanner() {
    printf("%-20s%s\n", "TOKEN-TYPE", "TOKEN-VALUE");
    printf("-------------------------------------------------\n");
}

void lex_error(char* msg, int line) {
    printf("\nError at line %-3d: %s\n\n", line, msg);
}

int yywrap(void) {
    return 1;
}