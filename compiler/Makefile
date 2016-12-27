OUT_DIR=./out
COMPILE=g++ -std=c++11

debug: DEBUG=-DPRINT=1 -DYYDEBUG=1
debug: clean parser

parser: clean lex grammar
	$(COMPILE) $(OUT_DIR)/lex.yy.c $(OUT_DIR)/grammar.tab.c main.cpp -o $(OUT_DIR)/parser -lfl $(DEBUG)

lex: out_dir
lex: lex.l
	flex -o $(OUT_DIR)/lex.yy.c lex.l

grammar: out_dir
grammar: grammar.y
	bison --defines=$(OUT_DIR)/grammar.tab.h -o $(OUT_DIR)/grammar.tab.c grammar.y

clean:
	rm $(OUT_DIR) -rf

out_dir:
	mkdir -p $(OUT_DIR)