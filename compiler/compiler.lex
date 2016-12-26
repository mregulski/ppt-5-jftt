%{
    #include <stdio.h>
    #include "compiler.tab.h"
%}
%x COMMENT
%%
"{"             BEGIN(COMMENT);
<COMMENT>.      ;
<COMMENT>"}"    BEGIN(INITIAL);
[_a-z]+         { yylval = yytext; return VAL_IDENTIFIER; }
[0-9]+          { yylval = strtoll(yytext, NULL, 10); return VAL_NUM; }
"-"             return OP_MINUS;
"+"             return OP_PLUS;
"*"             return OP_MULT;
"/"             return OP_DIV;
"%"             return OP_MOD;
":="            return OP_ASSIGN;
"="             return REL_EQ;
"<>"            return REL_NEQ;
">"             return REL_GT;
"<"             return REL_LT;
"<="            return REL_LEQ;
">="            return REL_GEQ;
"VAR"           return KEY_VAR;
"BEGIN"         return KEY_BEGIN;
"END"           return KEY_END;
"READ"          return KEY_READ;
"WRITE"         return KEY_WRITE;
"SKIP"          return KEY_SKIP;
"IF"            return KEY_IF;
"THEN"          return KEY_THEN;
"ELSE"          return KEY_ELSE;
"ENDIF"         return KEY_ENDIF;
"FOR"           return KEY_FOR;
"TO"            return KEY_TO;
"DOWNTO"        return KEY_DOWNTO;
"DO"            return KEY_DO;
"ENDFOR"        return KEY_ENDFOR;
"WHILE"         return KEY_WHILE;
"ENDWHILE"      return KEY_ENDWHILE;
";"             return OP_END;
%%
