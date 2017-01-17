#include "inter.h"
const OpCode OpCode::GET    = {"GET", 100};
const OpCode OpCode::PUT    = {"PUT", 100};

const OpCode OpCode::LOAD   = {"LOAD", 10};
const OpCode OpCode::STORE  = {"STORE", 10};
const OpCode OpCode::ADD    = {"ADD", 10};
const OpCode OpCode::SUB    = {"SUB", 10};

const OpCode OpCode::COPY   = {"COPY", 1};
const OpCode OpCode::SHR    = {"SHR", 1};
const OpCode OpCode::SHL    = {"SHL", 1};
const OpCode OpCode::INC    = {"INC", 1};
const OpCode OpCode::DEC    = {"DEC", 1};
const OpCode OpCode::ZERO   = {"ZERO", 1};

const OpCode OpCode::JUMP   = {"JUMP", 1};
const OpCode OpCode::JZERO  = {"JZERO", 1};
const OpCode OpCode::JODD   = {"JODD", 1};

const OpCode OpCode::HALT   = {"HALT", 0};
std::ostream & operator<<(std::ostream &stream, const Instruction &instruction) {
        stream << instruction.op.name;
        if(instruction.arg1 != Imp::NoReg) {
            stream << " " << instruction.arg1;
        }
        if(instruction.arg2 != Instruction::NoArg) {
            stream << " " << instruction.arg2;
        }
        return stream << "\n";
}