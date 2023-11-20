/* word_char_counter_cpp.l */
/* %option c++ */
%option noyywrap

%{
#include <cstdio>    
#include <cstring>
#include <string>
#include "parser.tab.h"
int chars = 0;
int words = 0;
using namespace std;

%}

%%
while     { printf("%s\n", yytext);yylval.str = new string(yytext); return WHILE; }
else      { printf("%s\n", yytext);yylval.str = new string(yytext); return ELSE; }
[a-zA-Z]+ { chars += strlen(yytext); words++; printf("%s\n", yytext); yylval.str = new string(yytext) ; return CHAR;}
. { }
%%

