%locations
%define parse.error verbose
%define parse.trace
%{
    #ifndef GRAMMAR_TAB_H
    #define GRAMMAR_TAB_H 1

    #include <unordered_map>
    #include <vector>
    #include <iostream>
    #include <string>
    #include <sstream>

    #include "symtable.h"
    #include "node.h"

    extern int CUR_LINE;
    extern int yylex();

    int errors = 0;

    void yyerror(std::string msg) {
        // fprintf(stderr, "[%d] ERROR: %s\n", CUR_LINE-1, s);
        errors++;
        std::cerr << "[" << CUR_LINE-1 << "] " << "ERROR: " << msg << std::endl;
    }
    extern Imp::Node *root;
    extern Imp::SymTable symbols;
    Imp::Node *decls = NULL;
    int CUR_LINE = 1;
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
    Imp::number val;
    int token;
    Imp::Program *prog;
    Imp::Declarations *decl;
    Imp::Commands *cmds;
    Imp::Command *cmd;
    Imp::Id *id;
    Imp::Value *value;
    Imp::Condition *cond;
    Imp::Expression *expr;

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
            $$ = new Imp::Program($2, $4);
            root = $$;
        }
    ;

vdeclarations:
        vdeclarations PIDENTIFIER {
            auto var = new Imp::Var($2, CUR_LINE);
            $1->declare(var);
        }
    |   vdeclarations PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE {
            $1->declare(new Imp::ConstArray($2, $4, CUR_LINE));
        }
    |   { $$ = new Imp::Declarations(); decls = $$; }
    ;

commands:
        commands command { $1->add_command($2); }
    |   command {
            $$ = new Imp::Commands();
            $$->add_command($1);
            }
    ;

command:
        identifier ASSIGN expression ENDSTMT { $$ = new Imp::Assign($1, $3, CUR_LINE); }
    |   IF condition THEN commands ELSE commands ENDIF { $$ = new Imp::If($2, $4, $6, CUR_LINE); }
    |   WHILE condition DO commands ENDWHILE { $$ = new Imp::While($2, $4, CUR_LINE); }
    |   FOR PIDENTIFIER FROM value TO value DO commands ENDFOR {
            Imp::Var *iterator = new Imp::Var($2, CUR_LINE);
            $$ = new Imp::For(iterator, $4, $6, $8, false, CUR_LINE);
        }
    |   FOR PIDENTIFIER FROM value DOWNTO value DO commands ENDFOR {
            $$ = new Imp::For(new Imp::Var($2, CUR_LINE), $4, $6, $8, true, CUR_LINE);
        }
    |   READ identifier ENDSTMT { $$ = new Imp::Read($2, CUR_LINE); }
    |   WRITE value ENDSTMT { $$ = new Imp::Write($2, CUR_LINE); }
    |   SKIP ENDSTMT { $$ = new Imp::Skip(); }
    ;

expression:
        value { $$ = new Imp::Const($1, CUR_LINE); }
    |   value PLUS value    { $$ = new Imp::Plus($1, $3, CUR_LINE); }
    |   value MINUS value   { $$ = new Imp::Minus($1, $3, CUR_LINE); }
    |   value MULT value    { $$ = new Imp::Mult($1, $3, CUR_LINE); }
    |   value DIV value     { $$ = new Imp::Div($1, $3, CUR_LINE); }
    |   value MOD value     { $$ = new Imp::Mod($1, $3, CUR_LINE); }
    ;

condition:
        value REL_EQ value  { $$ = new Imp::Eq($1, $3, CUR_LINE); }
    |   value REL_NEQ value { $$ = new Imp::Neq($1,$3, CUR_LINE); }
    |   value REL_GT value  { $$ = new Imp::Gt($1,$3, CUR_LINE); }
    |   value REL_LT value  { $$ = new Imp::Lt($1,$3, CUR_LINE); }
    |   value REL_LEQ value { $$ = new Imp::Leq($1,$3, CUR_LINE); }
    |   value REL_GEQ value { $$ = new Imp::Geq($1,$3, CUR_LINE); }
    ;

value:
        NUM { $$ = new Imp::Value($1, CUR_LINE); }
    |   identifier { $$ = new Imp::Value($1, CUR_LINE); }
    ;

identifier:
        PIDENTIFIER {
            $$ = new Imp::Var($1, CUR_LINE);
         }
    |   PIDENTIFIER BRACKET_OPEN PIDENTIFIER BRACKET_CLOSE { $$ = new Imp::VarArray($1, new Imp::Var($3, CUR_LINE), CUR_LINE); }
    |   PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE { $$ = new Imp::ConstArray($1, $3, CUR_LINE); }
    ;
%%
#endif
