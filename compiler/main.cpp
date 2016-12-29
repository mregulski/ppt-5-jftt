#include <vector>
#include "main.h"
#include "symtable.h"
#ifndef YYDEBUG
    #define YYDEBUG 0
#endif
extern int yyparse();
extern int yydebug;

Node *root = NULL;
SymTable symbols;
void walk_tree(Node *node, int depth);
int main() {
    yydebug = YYDEBUG;
    int result = yyparse();
    if (result) {
        printf("Invalid input\n");
    } else {
        printf("No syntax errors\n");
        if (root != NULL) {
            print_tree(root);
            walk_tree(root, 0);
        }
    }
    return result;
}


void walk_tree(Node *node, int depth) {
    if (node == NULL) return;
    switch (node->type) {
        case N_READ: case N_WRITE:
        case N_WHILE: case N_ASSIGN: case N_PLUS: case N_MINUS: case N_MULT: case N_DIV:
        case N_MOD: case N_CEQ: case N_CNEQ: case N_CGT: case N_CLT: case N_CLEQ: case N_CGEQ:
        case N_IF:
        case N_FOR: case N_FOR_D:
        case N_BLOCK: case N_DECLS: case N_PROGRAM: {
            for(int i = 0; i < node->children.count; i++) {
                walk_tree(node->children.list[i], depth + 1);
            }
            printf("%s#%d\n", get_node_type_name(node->type), node->id);
            break;
        }
        case N_NUM: {
            printf("%s(%lld)#%d\n", get_node_type_name(node->type), node->num, node->id);
            break;
        }
        case N_DECL: {
            break;
        }
        case N_ID: {
            printf("%s(%s)#%d\n", get_node_type_name(node->type), node->name, node->id);
            break;
        }
        case N_SKIP: {
            printf("%s()#%d\n", get_node_type_name(node->type), node->id);
            break;
        }
        default: {
            printf("UNKNOWN(%d)#%d\n", node->type, node->id);
        }
    }
}