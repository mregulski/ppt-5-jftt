%token PIDENTIFIER NUM
%token REL_EQ REL_NEQ REL_GT REL_LT REL_LEQ REL_GEQ
%token KEY_VAR KEY_BEGIN KEY_END KEY_READ KEY_WRITE
%token KEY_SKIP
%token KEY_IF KEY_THEN KEY_ELSE KEY_ENDIF
%token KEY_FOR KEY_TO KEY_DOWNTO KEY_ENDFOR
%token KEY_WHILE KEY_ENDWHILE
%token KEY_ASSIGN
%left OP_PLUS OP_MINUS OP_MULT OP_DIV OP_MOD
%%

program:
        VAR vdeclarations KEY_BEGIN commands KEY_END
    ;

vdeclarations:
        vdeclarations VAL_IDENTIFIER 
    |   vdeclarations VAL_IDENTIFIER[VAL_NUM]
    |
    ;
   
commands:
        commands command
    |   command
    ;
    
command:
        identifier KEY_ASSIGN expression OP_END
    |   KEY_IF condition KEY_THEN commands KEY_ELSE commands KEY_ENDIF
    |   KEY_WHILE condition KEY_DO commands KEY_ENDWHILE
    |   KEY_FOR VAL_IDENTIFIER KEY_FROM value KEY_TO value KEY_DO commands KEY_ENDFOR
    |   KEY_FOR VAL_IDENTIFIER KEY_FROM value KEY_DOWNTO value KEY_DO commands KEY_ENDFOR
    |   KEY_READ identifier OP_END
    |   KEY WRITE value OP_END
    |   KEY_SKIP OP_END
    ;
%%