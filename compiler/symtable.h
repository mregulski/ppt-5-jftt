#ifndef SYMTABLE_H
#define SYMTABLE_H
#include <unordered_map>
#include <string>
extern int yylineno;
class Var {
    public:
        std::string name;
        int size;
        int offset;
        bool initialized;
        int def_line;
        Var() {}
        Var(char *name) : name(name), size(0), initialized(false), def_line(yylineno) { }
        Var(char *name, size_t size) : name(name), size(size), initialized(false), def_line(yylineno) { }
};

class SymTable {
    public:
        void declare_var(char *name);
        void declare_arr(char *name, size_t size);
        Var get_var(char *name);
    private:
        std::unordered_map<std::string, Var> table;
        void declare(char *name, size_t size);
};
#endif
