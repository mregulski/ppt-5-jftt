%{
    #include <iostream>
    #include <string>
    extern int yylineno;
    extern "C" int yylex();

    void yyerror(const char *s) {
        std::cerr << "[" << yylineno << "] ERROR: " << s << std::endl;
    }
%}
%token <text> PIDENTIFIER
%token <val> NUM
%token <token> BRACKET_OPEN BRACKET_CLOSE
%token <token> REL_EQ REL_NEQ REL_GT REL_LT REL_LEQ REL_GEQ
%token <token> VAR KEY_BEGIN KEY_END READ WRITE
%token <token> SKIP ENDSTMT
%token <token> IF THEN ELSE ENDIF
%token <token> FOR FROM TO DOWNTO DO ENDFOR
%token <token> WHILE ENDWHILE
%token <token> ASSIGN PLUS MINUS MULT DIV MOD
%left PLUS MINUS
%left MULT DIV MOD

%define parse.trace

%union {
    int token;
    std::string *text;
    int val;
}

%start program
%%

program:
        VAR vdeclarations KEY_BEGIN commands KEY_END
    ;

vdeclarations:
        vdeclarations PIDENTIFIER
    |   vdeclarations PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE
    |
    ;

commands:
        commands command
    |   command
    ;

command:
        identifier ASSIGN expression ENDSTMT
    |   IF condition THEN commands ELSE commands ENDIF
    |   WHILE condition DO commands ENDWHILE
    |   FOR PIDENTIFIER FROM value TO value DO commands ENDFOR
    |   FOR PIDENTIFIER FROM value DOWNTO value DO commands ENDFOR
    |   READ identifier ENDSTMT
    |   WRITE value ENDSTMT
    |   SKIP ENDSTMT
    ;

expression:
        value
    |   value PLUS value
    |   value MINUS value
    |   value MULT value
    |   value DIV value
    |   value MOD value
    ;

condition:
        value REL_EQ value
    |   value REL_NEQ value
    |   value REL_GT value
    |   value REL_LT value
    |   value REL_LEQ value
    |   value REL_GEQ value
    ;

value:
        NUM
    |   identifier
    ;

identifier:
        PIDENTIFIER
    |   PIDENTIFIER BRACKET_OPEN PIDENTIFIER BRACKET_CLOSE
    |   PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE
    ;
%%
