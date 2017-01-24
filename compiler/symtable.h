#ifndef SYMTABLE_H
#define SYMTABLE_H 1
#include <unordered_map>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include "node.h"
#include "types.h"

namespace Imp {
    class Symbol {
        public:
            static const long long Undefined = -10;
            bool array = false;
            bool initialized = false;
            bool iterator = false;
            bool available = true;
            std::string name;
            long long size;
            long long offset;
            bool isInitialized() { return initialized; }
            bool isArray() { return array; }
            long long line;
            Symbol() : line(Undefined) {}
            Symbol(std::string name, long long line, long long offset)
                : name(name), line(line), offset(offset), size(1), initialized(false) {}
            Symbol(std::string name, long long line, long long offset, long long size)
                : name(name), line(line), offset(offset), size(size), initialized(false) {}
            virtual std::string dump() {
                std::ostringstream stream;
                stream << "{\n\tname: " << name;
                if (array) { stream << " (array :" << size << ")"; }
                stream << "\n\tdefinition: line " << line;
                stream << "\n\tmemory location: " << offset << "\n}" << std::endl;
                return stream.str();
            }
    };

    class SymTable {
        public:
            Symbol get_var(std::string name);
            bool declare(Id *id);
            bool declare_tmp(std::string name);
            bool declare_iterator(Id *iter);
            bool undeclare(std::string name);
            bool undeclare_iter(std::string name);
            void alloc_for_control(long for_count);
            bool is_initialized(Id *id);
            void set_initialized(Id *id);
            bool is_iterator(Id *id);
            void set_iterator(Id *id);
            void set_array(Id *id);
            virtual std::ostream& dump(std::ostream& stream) {
                for(auto &pair: table) {
                    stream << "symbol";
                    stream << std::get<1>(pair).dump();
                }
                return stream << std::endl;
            }
        private:
            long long offset = 0;
            long long for_offset = 0;
            std::unordered_map<std::string, Symbol> table;

            // todo: use map + queue instead of monotone offset?
            // std::map<unsigned long long, Symbol> memory;
            // std:dequeue<unsigned long long> mem_locations;
    };
}
#endif
