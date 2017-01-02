#ifndef SYMTABLE_H
#define SYMTABLE_H 1
#include <unordered_map>
#include <string>
#include "Node.h"

class Symbol {
    public:
        std::string name;
        int size;
        int offset;
        bool defined;
        int def_line;
        Symbol() {}
        Symbol(std::string name, int line) : name(name), size(0), defined(true), def_line(line) { }
        Symbol(std::string name, long long size, int line) : name(name), size(size), defined(true), def_line(line) { }
};

class SymTable {
    public:
        Symbol get_var(std::string name);
        bool declare(Id *id);
    private:
        std::unordered_map<std::string, Symbol> table;
};
#endif
