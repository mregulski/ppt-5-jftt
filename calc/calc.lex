%{
    #include <stdio.h>
    #include <string.h>
    #define YY_DECL extern int yylex()
    #include "calc.tab.h"

    
%}
%x comment
%%

\\\n    ;
[ \t]+  ;
^#              BEGIN(comment);
<comment>\\\n   ;
<comment>\n     BEGIN(0);
<comment>.     ;
\n      return NEWLINE;
[0-9]+  yylval = strtoll(yytext,0, 10); return NUM;
"+"     return PLUS;
"-"     return MINUS;
"*"     return MULT;
"/"     return DIV;
"%"     return MOD;
"^"     return EXP;
"("     return LEFT;
")"     return RIGHT;
.       ;
