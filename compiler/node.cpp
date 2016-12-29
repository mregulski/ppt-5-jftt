#include "main.h"
#include "symtable.h"
#include <unordered_set>
using namespace std;

NodeTable node_table;
/*
    returns `node` if it's a new one, or the already registered copy.
*/
Node *register_node(Node *node) {
    for(int i = 0; i < node_table.nodes.size(); i++) {
        if (node_equals(node, node_table.nodes[i])) {
            free(node);
            return node_table.nodes[i];
        }
    }
    node->id = node_table.max_id++;
    node_table.nodes.push_back(node);
    return node;
}
/*==============================
    CREATE
==============================*/
Node *create_node(enum NodeType type) {
    Node *node = (Node *) malloc(sizeof(Node));
    check_alloc(node);
    node->type = type;
    return node;
}

Node *create_op1_node(enum NodeType type, Node *child) {
    Node *node = create_op_node(type);
    node->children.list[0] = child;
    return node;
}

Node *create_op2_node(enum NodeType type, Node *left, Node *right) {
    Node *node = create_op_node(type);
    node->children.list[0] = left;
    node->children.list[1] = right;
    return node;
}

Node *create_op3_node(enum NodeType type, Node *left, Node *mid, Node *right) {
    Node *node = create_op_node(type);
    node->children.list[0] = left;
    node->children.list[1] = mid;
    node->children.list[2] = right;
    return node;
}

Node *create_op4_node(enum NodeType type, Node *id, Node *from, Node *to, Node *cmds) {
    Node *node = create_op_node(type);
    node->children.list[0] = id;
    node->children.list[1] = from;
    node->children.list[2] = to;
    node->children.list[3] = cmds;
    return node;
}

Node *create_value_node(long long value) {
    Node *node = create_node(N_NUM);
    check_alloc(node);
    node->num = value;
    return node;
}

Node *create_id_node(char *name) {
    Node *node = create_node(N_ID);
    check_alloc(node);
    node->name = strdup(name);
    return node;
}

Node *create_decl_node(char *name) {
    Node *node = create_node(N_DECL);
    check_alloc(node);
    node->name = strdup(name);
    return node;
}

Node *create_block_node(enum NodeType type) {
    Node *node = create_node(type);
    check_alloc(node);
    node->children.max_count = 1;
    node->children.count = 0;
    node->children.list = (Node **) malloc(node->children.max_count * sizeof(Node *));
    return node;
}

Node *create_op_node(enum NodeType type) {
    Node *node = create_node(type);
    check_alloc(node);
    int nchildren = 0;
    switch (type) {
        case N_READ: case N_WRITE: {
            nchildren = 1;
            break;
        }
        case N_WHILE: case N_ASSIGN: case N_PLUS: case N_MINUS: case N_MULT: case N_DIV:
        case N_MOD: case N_CEQ: case N_CNEQ: case N_CGT: case N_CLT: case N_CLEQ: case N_CGEQ: {
            nchildren = 2;
            break;
        }
        case N_IF: {
            nchildren = 3;
            break;
        }
        case N_FOR: case N_FOR_D: {
            nchildren = 4;
            break;
        }
        default: {
            fprintf(stderr, "Unable to create %s node", get_node_type_name(type));
            free(node);
            return NULL;
        }
    }
    if (nchildren > 0) {
        node->children.count = nchildren;
        node->children.max_count = nchildren;
        node->children.list = (Node **)malloc(sizeof(Node *) * nchildren);
    }
    return node;
}

/*==============================
    MODIFY
==============================*/

bool is_leaf(Node *node);

bool add_child(Node *node, Node *child) {
    if (is_leaf(node)) {
        return false;
    }

    if (node->children.count + 1 >= node->children.max_count) {
        Node **new_list = (Node **)realloc(node->children.list, node->children.max_count * 2);
        if (new_list != NULL) {
            node->children.list = new_list;
            node->children.max_count *= 2;
            node->children.list[node->children.count] = child;
            node->children.count++;
            return true;
        }
        return false;
    }

    node->children.list[node->children.count] = child;
    node->children.count++;
    return true;

}

/*==============================
    OUTPUT
==============================*/

void generate(Node *node, FILE *out) {
    switch (node->type) {
        case N_SKIP: {
            fprintf(out, "INC 0\n");
            break;
        }
        default:
            fprintf(out, ":)\n");
    }
}

void print_subtree(Node *node, int depth);

void print_tree(Node *node) {
    print_subtree(node, 0);
}

void print_subtree(Node *node, int depth) {
    if (depth > 0) {
        printf("|");
        for(int i = 0; i < depth; i++) {
            printf("-");
        }
    }
    switch (node->type) {
        case N_READ: case N_WRITE:
        case N_WHILE: case N_ASSIGN: case N_PLUS: case N_MINUS: case N_MULT: case N_DIV:
        case N_MOD: case N_CEQ: case N_CNEQ: case N_CGT: case N_CLT: case N_CLEQ: case N_CGEQ:
        case N_IF:
        case N_FOR: case N_FOR_D:
        case N_BLOCK: case N_DECLS: case N_PROGRAM: {
            printf("%s#%d\n", get_node_type_name(node->type), node->id);
            for(int i = 0; i < node->children.count; i++) {
                print_subtree(node->children.list[i], depth + 1);
            }
            break;
        }
        case N_NUM: {
            printf("%s(%lld)#%d\n", get_node_type_name(node->type), node->num, node->id);
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
/*==============================
    UTIL
==============================*/
const char *get_node_type_name(enum NodeType type) {
    const char* names[] = {
        "N_SKIP", "N_NUM", "N_ID", "N_DECL",
        "N_READ", "N_WRITE",
        "N_WHILE", "N_ASSIGN", "N_PLUS", "N_MINUS", "N_MULT", "N_DIV",
        "N_MOD", "N_CEQ", "N_CNEQ", "N_CGT", "N_CLT", "N_CLEQ", "N_CGEQ",
        "N_IF",
        "N_FOR", "N_FOR_D",
        "N_BLOCK", "N_DECLS", "N_PROGRAM"
    };

    return strdup(names[type]);
}

bool is_leaf(Node *node) {
    return (node->type == N_SKIP
            || node->type == N_NUM
            || node->type == N_ID);
}

bool node_equals(Node *a, Node *b) {
    if (a->type != b->type) return false;
    // same types
    switch (a->type) {
        case N_SKIP: {
            return true;
        }
        case N_NUM: {
            return a->num == b->num;
        }
        case N_ID: {
            return strcmp(a->name, b->name) == 0;
        }
        default: {
            for (int i = 0; i < a->children.count; i++) {
                if (! node_equals(a->children.list[i], b->children.list[i])) {
                    return false;
                }
            }
            return true;
        }
    }

}