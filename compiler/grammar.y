%locations
%define parse.error verbose
%define parse.trace
%{
    #ifndef GRAMMAR_TAB_H
    #define GRAMMAR_TAB_H 1
    #include "SymTable.h"
    #include "Node.h"
    #include <unordered_map>
    #include <vector>
    extern int yylineno;
    extern int yylex();
    bool ERROR = false;
    void yyerror(const char *s) {
        fprintf(stderr, "[%d] ERROR: %s\n", yylineno-1, s);
    }
    extern Node *root;
    extern SymTable symbols;
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

%union {
    char *text;
    int val;
    int token;
    Program *prog;
    Declarations *decl;
    Commands *cmds;
    Command *cmd;
    Id *id;
    Value *value;
    Condition *cond;
    Expression *expr;

}
%type <prog> program
%type <decl> vdeclarations
%type <cmds> commands
%type <cmd> command
%type <expr> expression
%type <cond> condition
%type <value> value
%type <id> identifier
%start program
%%

program:
        VAR vdeclarations KEY_BEGIN commands KEY_END {
            $$ = new Program($2, $4);
            root = $$;
        }
    ;

vdeclarations:
        vdeclarations PIDENTIFIER {
            $1->declare(new Var($2, yylineno));
        }
    |   vdeclarations PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE {
            $1->declare(new ConstArray($2, $4, yylineno));
        }
    |   { $$ = new Declarations(); }
    ;

commands:
        commands command { $1->add_command($2); }
    |   command {
            $$ = new Commands();
            $$->add_command($1);
            }
    ;

command:
        identifier ASSIGN expression ENDSTMT { $$ = new Assign($1, $3); }
    |   IF condition THEN commands ELSE commands ENDIF { $$ = new If($2, $4, $6); }
    |   WHILE condition DO commands ENDWHILE { $$ = new While($2, $4); }
    |   FOR PIDENTIFIER FROM value TO value DO commands ENDFOR {
            $$ = new For(new Var($2, yylineno), $4, $6, $8, false);
        }
    |   FOR PIDENTIFIER FROM value DOWNTO value DO commands ENDFOR {
            $$ = new For(new Var($2, yylineno), $4, $6, $8, true);
        }
    |   READ identifier ENDSTMT { $$ = new Read($2); }
    |   WRITE value ENDSTMT { $$ = new Write($2); }
    |   SKIP ENDSTMT { $$ = new Skip(); }
    ;

expression:
        value { $$ = new Const($1); }
    |   value PLUS value    { $$ = new Plus($1, $3); }
    |   value MINUS value   { $$ = new Minus($1, $3); }
    |   value MULT value    { $$ = new Mult($1, $3); }
    |   value DIV value     { $$ = new Div($1, $3); }
    |   value MOD value     { $$ = new Mod($1, $3); }
    ;

condition:
        value REL_EQ value  { $$ = new Eq($1, $3); }
    |   value REL_NEQ value { $$ = new Neq($1,$3); }
    |   value REL_GT value  { $$ = new Gt($1,$3); }
    |   value REL_LT value  { $$ = new Lt($1,$3); }
    |   value REL_LEQ value { $$ = new Leq($1,$3); }
    |   value REL_GEQ value { $$ = new Geq($1,$3); }
    ;

value:
        NUM { $$ = new Value($1); }
    |   identifier { $$ = new Value($1); }
    ;

identifier:
        PIDENTIFIER {
            symbols.get_var($1);
            $$ = new Var($1, yylineno);
         }
    |   PIDENTIFIER BRACKET_OPEN PIDENTIFIER BRACKET_CLOSE { $$ = new VarArray($1, new Var($3, yylineno), yylineno); }
    |   PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE { $$ = new ConstArray($1, $3, yylineno); }
    ;
%%
#endif
