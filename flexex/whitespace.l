    int words = 0;
    int lines = 1;
w [:blank:]
nw [[:alnum:][:punct:]]
%%

^{nw}+		words++; ECHO;
{nw}+		words++; ECHO;

^[ \t]*\r?\n			
[ \t]+				printf(" ");
\r?\n	lines++; ECHO;

%%
main() {
    yylex();
    printf("\n-----------------------------\nwords: %d\n", words);
    printf("lines: %d\n\n", lines);
}
