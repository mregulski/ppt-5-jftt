#include <vector>
#include <iostream>
#include "Inter.h"
#include "SymTable.h"
#include "CodeGen.h"

#ifndef YYDEBUG
    #define YYDEBUG 0
#endif
extern int yyparse();
extern int yydebug;

Node *root;
SymTable symbols;

using namespace std;

int main() {
    yydebug = YYDEBUG;
    int result = yyparse();
    if (result) {
        cerr << "Invalid input" << endl;
    } else {
        cerr << "No syntax errors" << endl;
        if (root != NULL) {
            cerr << "====================\n\tAST\n====================" << endl;
            ((Program *)root)->dump(cerr, 0);
            CodeGen generator;
            cerr << "====================\n\tOUTPUT\n====================" << endl;
            generator.generate_to(cout, root);
        }
    }
    return result;
}
