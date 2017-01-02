#ifndef INTER_H
#define INTER_H 1

#include <string>
#include <ostream>
struct OpCode {
    std::string name;
    int time;
    OpCode(std::string name, int time) : name(name), time(time) {}
    static const OpCode GET, PUT,
                        LOAD, STORE, ADD, SUB,
                        COPY, SHR, SHL, INC, DEC, ZERO,
                        JUMP, JZERO, JODD,
                        HALT;
};
// enum OpCode {
//     GET,
//     PUT,
//     LOAD,
//     STORE,
//     ADD,
//     SUB,
//     COPY,
//     SHR,
//     SHL,
//     INC,
//     DEC,
//     ZERO,
//     JUMP,
//     JZERO,
//     JODD,
//     HALT
// };

struct Instruction {
    OpCode op;
    long long arg1;
    long long arg2;
    Instruction(OpCode op, long long arg1, long long arg2) : op(op), arg1(arg1), arg2(arg2) {}
    std::ostream & operator<<(std::ostream &stream) {
        return stream << op.name << " " << arg1 << " " << arg2 << "\n";
    }
};

#endif