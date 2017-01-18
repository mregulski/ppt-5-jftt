OUT_DIR=./bin
WORK_DIR=./build
COMPILE=clang++ -std=c++11 -Wno-deprecated-register

debug: DEBUG= -g -O0 -DDEBUG1
debug: clean parser interpreter

parser: out_dir work_dir lex grammar
	cp *.h $(WORK_DIR)/
	$(COMPILE) $(WORK_DIR)/lex.yy.c $(WORK_DIR)/grammar.tab.c *.cpp -o $(OUT_DIR)/parser -lfl $(DEBUG)

interpreter: out_dir
	$(COMPILE) example/interpreter.cc -o $(OUT_DIR)/interpreter $(DEBUG)

lex: work_dir
lex: lex.l
	flex -o $(WORK_DIR)/lex.yy.c lex.l

grammar: work_dir
grammar: grammar.y
	bison --defines=$(WORK_DIR)/grammar.tab.h -o $(WORK_DIR)/grammar.tab.c grammar.y

clean:
	rm -rf $(OUT_DIR)
	rm -rf $(WORK_DIR)

out_dir:
	mkdir -p $(OUT_DIR)

work_dir:
	mkdir -p $(WORK_DIR)
