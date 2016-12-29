#ifndef MAIN_H
#define MAIN_H 1
#include <cstdio>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "symtable.h"
#define SYMBOLS_MAX 2048

#define check_alloc(ptr) if (ptr == NULL) { return NULL; }
#define check_alloc_v(ptr) if (ptr == NULL) { fprintf(stderr, "Unable to allocate memory."); return NULL; }
enum NodeType {
    /*0*/ N_SKIP, N_NUM, N_ID, N_DECL,
    /*1*/ N_READ, N_WRITE,
    /*2*/ N_WHILE, N_ASSIGN, N_PLUS, N_MINUS, N_MULT, N_DIV,
          N_MOD, N_CEQ, N_CNEQ, N_CGT, N_CLT, N_CLEQ, N_CGEQ,
    /*3*/ N_IF,
    /*4*/ N_FOR, N_FOR_D,
    /*?*/ N_BLOCK, N_DECLS, N_PROGRAM
};

typedef struct _Node {
    enum NodeType type;
    union {
        long long num;
        char *name;
        struct { struct _Node **list; size_t count; size_t max_count; } children;
    };
    int id;
} Node;


typedef struct {
    std::vector<Node*> nodes;
    int max_id;
} NodeTable;

const char *get_node_type_name(enum NodeType type);
void print_tree(Node *node);
Node *register_node(Node *node);
Node *create_node(enum NodeType type);
Node *create_op_node(enum NodeType type);
Node *create_op1_node(enum NodeType type, Node *child);
Node *create_op2_node(enum NodeType type, Node *left, Node *right);
Node *create_op3_node(enum NodeType type, Node *left, Node *mid, Node *right);
Node *create_op4_node(enum NodeType type, Node *id, Node *from, Node *to, Node *cmds);
Node *create_value_node(long long value);
Node *create_id_node(char *name);
Node *create_decl_node(char *name);
Node *create_block_node(enum NodeType type);
bool node_equals(Node *a, Node *b);
bool add_child(Node *node, Node *child);
#endif
