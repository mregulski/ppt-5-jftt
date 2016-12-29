#include <string>
#include "symtable.h"

bool SymTable::declare(char *name, size_t size) {
    fprintf(stderr, "declare: %s[%zu]\n", name, size);
    if (symbols.find(name) != symbols.end()) {
        return false;
    }
    Var var(name, size);
    symbols[std::string(name)] = var;
    return true;
}

bool SymTable::exists(char *name) {
    return (symbols.find(name) != symbols.end());
}