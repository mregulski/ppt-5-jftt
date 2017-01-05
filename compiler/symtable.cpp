#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>
#include <typeinfo>
#include "Node.h"
#include "SymTable.h"

using namespace std;

extern void yyerror(const char *msg);
extern int yynerrs, yylineno;

void error(string msg, int loc) {
    cerr << "Error: near line " << loc << ": " << msg << endl;
}

Symbol SymTable::get_var(string name) {
    if (table.find(name) != table.end()) {
        return table[name];
    }
    return Symbol();
}

long long SymTable::get_tmp() {
    return ++offset;
}

bool SymTable::declare(Id *id) {
    if(table.find(id->name) != table.end()) {
        // redeclaration
        yynerrs++;
        ostringstream os;
        os << id->name << " already defined on line " << table[id->name].def_line;
        error(os.str(), id->lineno);
        return false;
    }
    else {
        Symbol var;
        const type_info &id_type = typeid(*id);
        if (id_type == typeid(Var)) {
            var = Symbol(id->name, id->lineno, offset);
        } else if (id_type == typeid(ConstArray)) {
            var = Symbol(id->name, id->lineno, offset, ((ConstArray*)id)->idx);
        }
        offset += var.size;
        table[id->name] = var;
        return true;
    }
}
