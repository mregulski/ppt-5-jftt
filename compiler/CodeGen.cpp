#include <vector>
#include "CodeGen.h"
#include "Node.h"
#include "Inter.h"
#include "Colors.h"
using namespace std;
namespace Imp {
    vector<Instruction *> CodeGen::generate(Node *root) {
        return root->gen_ir(&instruction_counter, Imp::R1);
    }

    void CodeGen::generate_to(ostream &stream, Node *root) {
        auto code = root->gen_ir(&instruction_counter, Imp::R1);
        if (n_error > 0) {
            cerr << "Compilation failed: " << n_error << " errors found." << endl;
        } else {
            cerr << "output size: " << code.size() << endl;
            for(int i = 0; i < code.size(); i++) {
                // labels debug
                // cerr << code[i]->label << ". " << *code[i];
                stream << *code[i];
            }
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