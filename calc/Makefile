run: build
	./calc

debug: build_debug
	./calc

build: lex parse
	gcc calc.tab.c lex.yy.c -o calc -lm -ll

build_debug: lex parse
	gcc calc.tab.c lex.yy.c -g -o calc -lm -ll -DYYDEBUG 1

parse:
	bison -d calc.y
lex:
	flex calc.lex
