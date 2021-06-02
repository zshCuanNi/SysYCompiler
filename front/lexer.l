%option yylineno
%option noinput
%option nounput
%option noyywrap
%{
#include "ast.hpp"
#include "parser.tab.hpp"
#include "stdio.h"
#include <string.h>

void p(const char *token, const char *value = nullptr);
// void lex_error(std::string msg, int line);
%}

white_space             ([ \t\r\a]+)
include_statement       ("#"[^\n]*)
single_line_comment     ("//"[^\n]*)
multi_line_comment      ("/*"([^*]*[*]+[^*/])*[^*]*[*]+"/")
operator                ([+*-/%=,;!<>\(\){}\[\]])
identifier              ([_a-zA-Z][_a-zA-Z0-9]*)
decimal_const           ([1-9][0-9]*)
octal_const             (0[0-7]*)
hexadecimal_const       (0[xX][0-9a-fA-F]+)

%%

\n                      { p("enter"); }
{white_space}           { p("space"); /* ignore white spaces */ }
{include_statement}     { p("include", yytext); /* ignore include statements */ }
{multi_line_comment}    { p("multi_comment", yytext); /* skip multi-line comments */ }
{single_line_comment}   { p("single_comment", yytext); /* skip single line comments */ }

{operator}              { p("op", yytext); return yytext[0]; }

"=="                    { p("teq"); return T_EQ; }
"<="                    { p("tleq"); return T_LEQ; }
">="                    { p("tgeq"); return T_GEQ; }
"!="                    { p("tneq"); return T_NEQ; }
"&&"                    { p("land"); return T_AND; }
"||"                    { p("lor"); return T_OR; }
"void"                  { p("tvoid"); return T_VOID; }
"int"                   { p("tint"); return T_INT; }
"if"                    { p("tif"); return T_IF; }
"else"                  { p("telse"); return T_ELSE; }
"while"                 { p("twhile"); return T_WHILE; }
"break"                 { p("tbreak"); return T_BREAK; }
"continue"              { p("tcontinue"); return T_CONTINUE; }
"return"                { p("tret"); return T_RETURN; }
"const"                 { p("tconst"); return T_CONST; }

{identifier}            { p("id", yytext); yylval.string = strdup(yytext); return T_IDENTIFIER; }
{hexadecimal_const}     { p("num", yytext); yylval.number = strtol(yytext, NULL, 0); return T_DECIMAL_CONST; }
{decimal_const}         { p("num", yytext); yylval.number = strtol(yytext, NULL, 0); return T_DECIMAL_CONST; }
{octal_const}           { p("num", yytext); yylval.number = strtol(yytext, NULL, 0); return T_DECIMAL_CONST; }

.                       { /* do nothing */ }

%%

void p(const char* token, const char *value) {
    return;
    printf("%s", token);
    if (value)
        printf(", %s", value);
    printf("\n");
}