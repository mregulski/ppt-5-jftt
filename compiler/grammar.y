%locations
%define parse.error verbose
%define parse.trace
%{
    #include "main.h"
    #include "symtable.h"
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
    Node *node;
}
%type <node> program vdeclarations commands command expression condition value identifier
%start program
%%

program:
        VAR vdeclarations KEY_BEGIN commands KEY_END {
             root = create_block_node(N_PROGRAM);
             add_child(root, $2);
             add_child(root, $4);
             register_node(root);
        }
    ;

vdeclarations:
        vdeclarations PIDENTIFIER {
            Node *id = create_decl_node($2);
            symbols.declare_var(id->name);
            id = register_node(id);
            add_child($1, id);

        }
    |   vdeclarations PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE {
            Node *id = create_decl_node($2);
            symbols.declare_arr(id->name, $4);
            id = register_node(id);
            add_child($1, id);
        }
    |   { Node *decls = create_block_node(N_DECLS); $$ = register_node(decls); }
    ;

commands:
        commands command { add_child($1, $2); }
    |   command {
            Node *cmds = create_block_node(N_BLOCK);
            add_child(cmds, $1);
            $$ = register_node(cmds);
            }
    ;

command:
        identifier ASSIGN expression ENDSTMT { $$ = register_node(create_op2_node(N_ASSIGN, $1, $3)); }
    |   IF condition THEN commands ELSE commands ENDIF { $$ = register_node(create_op3_node(N_IF, $2, $4, $6)); }
    |   WHILE condition DO commands ENDWHILE { $$ = register_node(create_op2_node(N_WHILE, $2, $4)); }
    |   FOR PIDENTIFIER FROM value TO value DO commands ENDFOR {
            Node *idnode = register_node(create_id_node($2));
            $$ = register_node(create_op4_node(N_FOR, idnode, $4, $6, $8));
        }
    |   FOR PIDENTIFIER FROM value DOWNTO value DO commands ENDFOR {
            Node *idnode = register_node(create_id_node($2));
            $$ = register_node(create_op4_node(N_FOR, idnode, $4, $6, $8));
        }
    |   READ identifier ENDSTMT { $$ = register_node(create_op1_node(N_READ, $2)); }
    |   WRITE value ENDSTMT { $$ = register_node(create_op1_node(N_WRITE, $2)); }
    |   SKIP ENDSTMT { $$ = register_node(create_node(N_SKIP)); }
    ;

expression:
        value { $$ = $1; }
    |   value PLUS value    { $$ = register_node(create_op2_node(N_PLUS, $1, $3));  }
    |   value MINUS value   { $$ = register_node(create_op2_node(N_MINUS, $1, $3)); }
    |   value MULT value    { $$ = register_node(create_op2_node(N_MULT, $1, $3));  }
    |   value DIV value     { $$ = register_node(create_op2_node(N_DIV, $1, $3));   }
    |   value MOD value     { $$ = register_node(create_op2_node(N_MOD, $1, $3));   }
    ;

condition:
        value REL_EQ value  { $$ = register_node(create_op2_node(N_CEQ, $1, $3)); }
    |   value REL_NEQ value { $$ = register_node(create_op2_node(N_CNEQ, $1, $3)); }
    |   value REL_GT value  { $$ = register_node(create_op2_node(N_CGT, $1, $3)); }
    |   value REL_LT value  { $$ = register_node(create_op2_node(N_CLT, $1, $3)); }
    |   value REL_LEQ value { $$ = register_node(create_op2_node(N_CLEQ, $1, $3)); }
    |   value REL_GEQ value { $$ = register_node(create_op2_node(N_CGEQ, $1, $3)); }
    ;

value:
        NUM { $$ = register_node(create_value_node($1)); }
    |   identifier { $$ = $1; }
    ;

identifier:
        PIDENTIFIER {
             $$ = register_node(create_id_node($1));
         }
    |   PIDENTIFIER BRACKET_OPEN PIDENTIFIER BRACKET_CLOSE { $$ = create_id_node((char *) "arr[id]"); }
    |   PIDENTIFIER BRACKET_OPEN NUM BRACKET_CLOSE { $$ = create_id_node((char *) "arr[num]"); }
    ;
%%
