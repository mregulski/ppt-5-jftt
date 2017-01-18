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

    Symbol SymTable::get_var(string name) {
        if (table.find(name) != table.end()) {
            return table[name];
        }
        return Symbol();
    }

    Symbol SymTable::get_tmp(string name) {
        return get_var(name);
    }

    bool SymTable::declare_tmp(string name) {
        if(table.find(name) != table.end()) {
            return false;
        }
        Symbol var = Symbol(name, -1, offset);
        offset += var.size;
        table[name] = var;
        return true;
    }

    bool SymTable::undeclare(string name) {
        if(table.find(name) == table.end()) { return true; }
        // if (table[name].offset+table[name].size == offset) {
        //     offset -= table[name].size;
        // }
        table.erase(name);
        return true;
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