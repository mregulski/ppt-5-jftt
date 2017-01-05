#include <vector>
#include "CodeGen.h"
#include "Node.h"
#include "Inter.h"

using namespace std;

vector<Instruction *> CodeGen::generate(Node *root) {
    return root->gen_ir(&instruction_counter, Imp::R1);
}

void CodeGen::generate_to(ostream &stream, Node *root) {
    auto code = root->gen_ir(&instruction_counter, Imp::R1);
    for(int i = 0; i < code.size(); i++) {
        cerr << code[i]->label << ". " << *code[i]; // labels debug
        stream << *code[i];
    }
}