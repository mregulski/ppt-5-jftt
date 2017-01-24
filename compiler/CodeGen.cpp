#include <vector>
#include "codegen.h"
#include "node.h"
#include "inter.h"
#include "colors.h"
#include "symtable.h"
using namespace std;

extern Imp::SymTable symbols;

namespace Imp {
    vector<Instruction *> CodeGen::generate(Node *root) {
        return root->gen_ir(&instruction_counter, Imp::R1);
    }

    bool CodeGen::generate_to(ostream &stream, Node *root) {
        auto code = root->gen_ir(&instruction_counter, Imp::R1);
        if (n_error > 0) {
            cerr << "Compilation" << Color::red << " failed: " << Color::def <<  n_error << " errors found." << endl;
            return false;
        } else {
            cerr << Color::cyan << string(40, '=') << endl
                << "\tSYMBOLS" << endl
                << string(40, '=') << Color::def << endl;
            symbols.dump(cerr);

            cerr << Color::cyan << string(40, '=') <<  endl
                << "\tOUTPUT" << endl
                << string(40, '=') << Color::def << endl;
            cerr << "output size: " << code.size() << endl;

            for(int i = 0; i < code.size(); i++) {
                // labels debug
                #ifdef DEBUG
                cerr << code[i]->label << ". " << *code[i];
                #endif
                stream << *code[i];
            }
            return true;
        }
    }

    void CodeGen::report(std::string error, long long line) {
        cerr << Color::red << "Error in line " << line << ": " << Color::def
                << error << endl;
        n_error++;
    }

    void CodeGen::report(std::ostringstream& error, long long line) {
        cerr << Color::red << "Error in line " << line << ": " << Color::def
                << error.str() << endl;
        n_error++;
    }


}