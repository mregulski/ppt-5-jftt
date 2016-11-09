    int words = 0;
    int lines = 0;
%%
^[ \t]+
[ \t]+$
\r?\n                   lines++;
[ \t]+                  printf(" ");  words++;
(\r?\n)*[ \t]*(\r?\n)+  printf("\n"); words++; lines++;
%%
main() {
    yylex();
    printf("\n-----------------------------\nwords: %d\n", words);
    printf("lines: %d\n\n", lines);
}
