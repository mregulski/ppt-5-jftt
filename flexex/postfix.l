%{
	#include <math.h>
	#define STACK_SIZE 1000
	static int stackPtr = -1;
	static int stack[STACK_SIZE];
	
	int calc_error = 0;
	
	static void push(int num) {
		stackPtr++;
		if (stackPtr < STACK_SIZE) {
			stack[stackPtr] = num;
		} else {
			fprintf(stderr, "Error: stack overflow\n");
			calc_error = 1;
		}
	}
	
	static int pop() {
		//printf("pop at [%d] (%d)\n", stackPtr, stack[stackPtr]);
		if (stackPtr >= 0) {
			return stack[stackPtr--];
		}
		fprintf(stderr, "Error: not enough operands\n");
		calc_error = 1;
		return 0;
	}
	
	static void dump_stack(int n) {
		if (n > STACK_SIZE) {
			n = STACK_SIZE;
		}
		printf("\n--------------------\n");
		printf("STACK DUMP\n");
		int i;
		for (i = 0; i < n; i++) {
			if (i == stackPtr) {
				printf("_%d_ ", stack[i]);
			} else {
				printf("%d ", stack[i]);
			}
		}
		printf("\n--------------------\n");
		
	}
%}

%x error
%%

-?{number}		{ push(atoi(yytext));  }
"+"				{ push(pop() + pop()); }
"-"				{ 
					int sub = pop(); 
					push(pop() - sub); 
				}
"*"				{ push(pop() * pop()); }
"/"				{ 
					int div = pop(); 
					if (div == 0) {
						fprintf(stderr, "Error: division by 0\n");
						BEGIN(error);  
					} else {
						push(pop() / div); 
					}
				}
"^"				{ 
					int exp = pop();
					int base = pop();
					push((int) pow((double) base, (double) exp)); 
				}
"%"				{ 
					int mod = pop(); 
					if (mod == 0) {
						fprintf(stderr, "Error: division by 0\n");
						BEGIN(error);  
					} else {
						push(pop() % mod); 
					}
				}
[ \t]			;
\n				{ 
					int result = pop();
					if (!calc_error) {
						printf("= %d\n", result);
					}
					calc_error = 0;
					stackPtr = -1;
					BEGIN(INITIAL);
					
				}
[d]				dump_stack(10); BEGIN(error);
.				BEGIN(error); printf("Error: unknown character: %s\n", yytext);
<error>.		;
<error>\n		BEGIN(INITIAL); 

