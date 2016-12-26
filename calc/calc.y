%{
    #include <stdio.h>
    #include <math.h>
    #define YYDEBUG 0
    int yylex();
    int err = 0;
    void yyerror(const char *s);

    // 'real' modulo, i.e. for all a, b modulo(a,b) >= 0
    long long modulo(int a, int b) {
        int r = a % b;
        return a - b * (a / b);
    }
%}
%define parse.error verbose
%define parse.trace true
%token NUM NEWLINE LEFT RIGHT
%left MINUS PLUS
%left MULT DIV MOD
%precedence NEG
%right EXP
%%
input:
  %empty
| input line
;

line:
  NEWLINE
| exp NEWLINE   { 
                    if(!err) {
                        printf ("\n = %d\n", $1);
                    } else {
                        printf("\n");
                    }
                    err = 0;
                }
| error NEWLINE { err = 0; yyerrok; }
;

exp:
  NUM                   { $$ = $1; printf("%d ", $1); }
| exp PLUS exp          { $$ = $1 + $3; printf("+ "); }
| exp MINUS exp         { $$ = $1 - $3; printf("- "); }
| exp MULT exp          { $$ = $1 * $3; printf("* "); }
| exp DIV exp           { 
                            printf("/ ");
                            if ($3 == 0) {
                                yyerror("division by 0");
                            } else {
                                // $$ = div($1, $3);
                                $$ = $1 / $3;
                            } 

                        }
| exp MOD exp           {    
                            printf("%% ");
                            if ($3 == 0) {
                                yyerror("division by 0");
                            } else {
                                // $$ = modulo($1, $3);
                                $$ = $1 % $3;
                            }
                        }
| MINUS exp  %prec NEG  { $$ = -$2; printf("~ "); }
| exp EXP exp           {
                            printf("^ ");
                            if( $3 < 0) {
                                yyerror("negative exponents are not supported on integers");
                            } else {
                            $$ = pow ($1, $3);
                            } 
                        }
| LEFT exp RIGHT        { $$ = $2; }
;
%%

void yyerror(const char *s) {
    err = 1;
    fprintf(stderr, "\n[Error]%s.\n", s);
}

int main() {
    #if YYDEBUG
    yydebug = YYDEBUG;
    #endif
    yyparse();
}