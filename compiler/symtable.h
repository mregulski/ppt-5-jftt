#ifndef SYMTABLE_H
#define SYMTABLE_H 1
#include <unordered_map>
#include <string>
#include "Node.h"

class Symbol {
    public:
        std::string name;
        long long size;
        long long offset;
        bool initialized;
        long long def_line;
        Symbol() {}
        Symbol(std::string name, long long line, long long offset)
            : name(name), def_line(line), offset(offset), size(1), initialized(false) { }
        Symbol(std::string name, long long line, long long offset, long long size)
            : name(name), def_line(line), offset(offset), size(size), initialized(false) { }
};

class SymTable {
    public:
        Symbol get_var(std::string name);
        long long get_tmp();
        bool declare(Id *id);
    private:
        long long offset = 0;
        std::unordered_map<std::string, Symbol> table;
};
#endif
