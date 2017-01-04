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
        bool initialized;
        int def_line;
        Symbol() {}
        Symbol(std::string name, int line, int offset)
            : name(name), def_line(line), offset(offset), size(1), initialized(false) { }
        Symbol(std::string name, int line, int offset, long long size)
            : name(name), def_line(line), offset(offset), size(size), initialized(false) { }
};

class SymTable {
    public:
        Symbol get_var(std::string name);
        bool declare(Id *id);
    private:
        long long offset = 0;
        std::unordered_map<std::string, Symbol> table;
};
#endif
