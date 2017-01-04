#ifndef CODEGEN_H
#define CODEGEN_H 1

#include <vector>
#include <iostream>
#include "Types.h"
#include "Inter.h"
#include "Node.h"

class CodeGen {
    Imp::label instruction_counter = 0;
    public:
        std::vector<Instruction> generate(Node *root);
        void generate_to(std::ostream &stream, Node *root);
};
#endif