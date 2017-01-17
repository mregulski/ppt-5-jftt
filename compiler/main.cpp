#include <vector>
#include <iostream>
#include "inter.h"
#include "symtable.h"
#include "codegen.h"

#ifndef YYDEBUG
    #define YYDEBUG 0
#endif
extern int yyparse();
extern int yydebug;
extern int errors;

using namespace Imp;
using namespace std;
Imp::Node *root;
Imp::SymTable symbols;
CodeGen generator;

int main() {
    yydebug = YYDEBUG;
    int syntaxInvalid = yyparse();
    if (syntaxInvalid || errors > 0) {
        cerr << "Error compiling file: " << errors << " errors found" << endl;
        return 0;
    } else {
        cerr << "No syntax errors" << endl;
        if (root != NULL) {
            cerr << "====================\n\tAST\n====================" << endl;
            ((Program *)root)->dump(cerr, 0);
            cerr << "====================\n\tOUTPUT\n====================" << endl;
            generator.generate_to(cout, root);
        }
    }
    return 1;
}
