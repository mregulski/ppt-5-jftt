#ifndef SYMTABLE_H
#define SYMTABLE_H 1
#include <unordered_map>
#include <string>
#include "Node.h"
#include "Types.h"

namespace Imp {
    class Symbol {
        public:
            static const long long Undefined = -10;
            std::string name;
            long long size;
            long long offset;
            bool initialized;
            long long line;
            Symbol() : line(Undefined) {}
            Symbol(std::string name, long long line, long long offset)
                : name(name), line(line), offset(offset), size(1), initialized(false) {}
            Symbol(std::string name, long long line, long long offset, long long size)
                : name(name), line(line), offset(offset), size(size), initialized(false) {}
    };

    class SymTable {
        public:
            Symbol get_var(std::string name);
            Symbol get_tmp(std::string name);
            bool declare_tmp(std::string name);
            bool undeclare(std::string name);
            bool declare(Id *id);
        private:
            long long offset = 0;
            std::unordered_map<std::string, Symbol> table;
            // todo: use map + queue instead of monotone offset
            // std::map<unsigned long long, Symbol> memory;
            // std:dequeue<unsigned long long> mem_locations;
    };
}
#endif
