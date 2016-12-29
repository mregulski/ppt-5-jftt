#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include "main.h"
#include "symtable.h"

using namespace std;

extern void yyerror(const char *msg);
extern int yynerrs, yylineno;


void SymTable::declare_var(char *name) {
    declare(name, 0);
}

void SymTable::declare_arr(char *name, size_t size) {
    declare(name, size);
}

void SymTable::declare(char *name, size_t size) {
    if (table.find(name) != table.end()) {
        yynerrs++;
        ostringstream os;
        os << name << " already defined on line " << table[name].def_line;
        std::string msg = os.str();
        yyerror(msg.c_str());
    } else {
        Var var(name, size);
        table[name] = var;
    }
}

Var SymTable::get_var(char *name) {
    return table[string(name)];
}