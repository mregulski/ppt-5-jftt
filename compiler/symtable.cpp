#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
#include "node.h"
#include "symtable.h"

using namespace std;

extern void yyerror(const char *msg);
extern int yynerrs, yylineno;
namespace Imp {
    void error(string msg, int loc) {
        cerr << "Error: near line " << loc << ": " << msg << endl;
    }

    bool SymTable::declare(Id *id) {
        if(table.find(id->name) != table.end()) {
            return false;
        }
        else {
            Symbol var;
            const type_info &id_type = typeid(*id);
            if (id_type == typeid(Var)) {
                var = Symbol(id->name, id->line, offset);
            } else if (id_type == typeid(ConstArray)) {
                var = Symbol(id->name, id->line, offset, ((ConstArray*)id)->idx);
            }
            offset += var.size;
            table[id->name] = var;
            return true;
        }
    }

    bool SymTable::declare_iterator(Id *iter) {
        if(table.find(iter->name) != table.end() && !table[iter->name].available) {
            return false;
        }
        else {
            Symbol var;
            var = Symbol(iter->name, iter->line, for_offset);
            for_offset += var.size;
            table[iter->name] = var;
            table[iter->name].available = false;
            return true;
        }
    }

    bool SymTable::declare_tmp(string name) {
        if(table.find(name) != table.end()) {
            return false;
        }
        Symbol var = Symbol(name, 0, offset);
        offset += var.size;
        table[name] = var;
        return true;
    }

    void SymTable::alloc_for_control(long for_count) {
        for_offset = offset;
        offset += for_count*2;
    }

    Symbol SymTable::get_var(string name) {
        if (table.find(name) != table.end()) {
            return table[name];
        }
        return Symbol();
    }

    // to be used with FOR-related vars
    bool SymTable::undeclare_iter(string name) {
        if(table.find(name) == table.end()) { return true; }
        table[name].available = true;
        return true;
    }

    bool SymTable::undeclare(string name) {
        if(table.find(name) == table.end() ) { return true; }
        table.erase(name);
        return true;
    }


    bool SymTable::is_initialized(Id *id) {
        if(table.find(id->name) == table.end()) {
            return false;
        }
        return table[id->name].initialized;
    }

    void SymTable::set_initialized(Id *id) {
        if(table.find(id->name) == table.end()) {
            return;
        } else {
            table[id->name].initialized = true;
        }
    }

    bool SymTable::is_iterator(Id *id) {
        if(table.find(id->name) == table.end()) {
            return false;
        }
        return table[id->name].iterator;
    }
    void SymTable::set_iterator(Id *id) {
        if(table.find(id->name) == table.end()) {
            return;
        } else {
            table[id->name].iterator = true;
        }
    }

    void SymTable::set_array(Id *id) {
        if(table.find(id->name) == table.end()) {
            return;
        } else {
            table[id->name].array = true;
            // treat all arrays as initialized
            table[id->name].initialized = true;
        }
    }
}