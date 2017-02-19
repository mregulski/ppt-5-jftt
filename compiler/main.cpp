#include <vector>
#include <iostream>
#include "inter.h"
#include "symtable.h"
#include "codegen.h"
#include "colors.h"
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
        cerr << "Error compiling file: " << errors << " syntax errors found" << endl;
        return 1;
    } else {
        // cerr << Color::green << "No syntax errors" << Color::def << endl;
        if (root != NULL) {
            #ifdef DEBUG
            cerr << Color::cyan << string(40, '=') << endl
                << "\tAST" << endl
                << string(40, '=') << Color::def << endl;
            ((Program *)root)->dump(cerr, 0);
            #endif
            bool result = generator.generate_to(cout, root);
            if (result) {
                cerr << Color::green << "Compilation successful"
                    << Color::def << endl;
            }
            return !result;
        }
    }
    return 1;
}
