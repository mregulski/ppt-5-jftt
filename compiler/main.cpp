#include <iostream>
#include <unordered_map>
#include "main.h"
#include "symtable.h"
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
extern int yyparse();
extern int yydebug;
SymTable symbols;
using namespace std;

int main() {

    yydebug = YYDEBUG;
    int result = yyparse();
    if (result) {
        std::cout << "Invalid input\n";
    } else {
        std::cout << "Valid input\n";
    }
    return result;
}



