%option noyywrap
%option yylineno
%{
    // #include <string.h>
    #include "node.h"
    #include "grammar.tab.h"

    #define TOKEN(t) return (yylval.token = t)
    #define SAVE_INT yylval.val = strtoll(yytext, NULL, 10)
    extern int CUR_LINE;

    // extern int yylex(YYSTYPE *lvalp, YYLTYPE *llocp);
%}

%x COMMENT
%%
"{"             BEGIN(COMMENT);
<COMMENT>"}"    BEGIN(INITIAL);
<COMMENT>.      ;
<COMMENT>\n     ; CUR_LINE++;
[_a-z]+         {
                    yylval.text = (char *) malloc(yyleng * sizeof(char) + 1);
                    strcpy(yylval.text, yytext);
                    return PIDENTIFIER;
                }
[0-9]+          { SAVE_INT; return NUM; }
"-"             { TOKEN(MINUS); }
"+"             { TOKEN(PLUS); }
"*"             { TOKEN(MULT); }
"/"             { TOKEN(DIV); }
"%"             { TOKEN(MOD); }
":="            { TOKEN(ASSIGN); }
"="             { TOKEN(REL_EQ); }
"<>"            { TOKEN(REL_NEQ); }
">"             { TOKEN(REL_GT); }
"<"             { TOKEN(REL_LT); }
"<="            { TOKEN(REL_LEQ); }
">="            { TOKEN(REL_GEQ); }
"VAR"           { TOKEN(VAR); }
"BEGIN"         { TOKEN(KEY_BEGIN); }
"END"           { TOKEN(KEY_END); }
"READ"          { TOKEN(READ); }
"WRITE"         { TOKEN(WRITE); }
"SKIP"          { TOKEN(SKIP); }
"IF"            { TOKEN(IF); }
"THEN"          { TOKEN(THEN); }
"ELSE"          { TOKEN(ELSE); }
"ENDIF"         { TOKEN(ENDIF); }
"FOR"           { TOKEN(FOR); }
"FROM"          { TOKEN(FROM); }
"TO"            { TOKEN(TO); }
"DOWNTO"        { TOKEN(DOWNTO); }
"DO"            { TOKEN(DO); }
"ENDFOR"        { TOKEN(ENDFOR); }
"WHILE"         { TOKEN(WHILE); }
"ENDWHILE"      { TOKEN(ENDWHILE); }
";"             { TOKEN(ENDSTMT); }
"["             { TOKEN(BRACKET_OPEN); }
"]"             { TOKEN(BRACKET_CLOSE); }
"\n"            { CUR_LINE++; }
[ \t]           ;
.               ;
%%
