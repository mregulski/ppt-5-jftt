#include <vector>
#include <iostream>
#include "SymTable.h"
#ifndef YYDEBUG
    #define YYDEBUG 0
#endif
extern int yyparse();
extern int yydebug;

Node *root;
SymTable symbols;

int main() {
    yydebug = YYDEBUG;
    int result = yyparse();
    if (result) {
        printf("Invalid input\n");
    } else {
        printf("No syntax errors\n");
        if (root != NULL) {
            root->gen_ir();
            std::cout << ((Program *)root)->dump(std::cout, 0);
            // walk_tree(root, 0);
        }
    }
    return result;
}