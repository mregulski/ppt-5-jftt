OUT_DIR=./out
COMPILE=clang++ -std=c++11 -Wno-deprecated-register

debug: DEBUG= -g
debug: clean parser

parser: clean lex grammar
	cp *.h $(OUT_DIR)/
	$(COMPILE) $(OUT_DIR)/lex.yy.c $(OUT_DIR)/grammar.tab.c *.cpp -o $(OUT_DIR)/parser -lfl $(DEBUG)

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