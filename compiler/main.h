#ifndef MAIN_H
#define MAIN_H
#include <string>

class Var {
    public:
        std::string name;
        size_t size;
        size_t offset;
        Var() {}
        Var(char *name, size_t size) : name(name), size(size) {}
};
#endif
