#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <unordered_map>
#include "main.h"
class SymTable {
    public:
        bool declare(char *name, size_t size);
        bool exists(char *name);
    private:
        std::unordered_map<std::string, Var> symbols;
};
#endif
