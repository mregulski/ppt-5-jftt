#ifndef INTER_H
#define INTER_H 1

#include <string>
#include <ostream>
#include <vector>
#include "types.h"

struct OpCode {

        OpCode(std::string name, int time) : name(name), time(time) {}

        std::string name;
        int time;

        static const OpCode GET, PUT,
                        LOAD, STORE, ADD, SUB,
                        COPY, SHR, SHL, INC, DEC, ZERO,
                        JUMP, JZERO, JODD,
                        HALT;
};

struct Instruction {
    public:
        static const Imp::arg NoArg = -1;

        OpCode op;
        Imp::Reg arg1;
        Imp::arg arg2;

        Imp::label label;

        static Instruction *GET(Imp::Reg arg1, Imp::label *label) {
            // creates instruction with label's OLD value, THEN increments it
            return new Instruction(OpCode::GET, arg1, (*label)++);
        }

        static Instruction *PUT(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::PUT, arg1, (*label)++);
        }

        static Instruction *LOAD(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::LOAD, arg1, (*label)++);
        }

        static Instruction *STORE(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::STORE, arg1, (*label)++);
        }

        static Instruction *ADD(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::ADD, arg1, (*label)++);
        }

        static Instruction *SUB(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::SUB, arg1, (*label)++);
        }

        static Instruction *COPY(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::COPY, arg1, (*label)++);
        }

        static Instruction *SHR(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::SHR, arg1, (*label)++);
        }

        static Instruction *SHL(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::SHL, arg1, (*label)++);
        }

        static Instruction *INC(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::INC, arg1, (*label)++);
        }

        static Instruction *DEC(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::DEC, arg1, (*label)++);
        }

        static Instruction *ZERO(Imp::Reg arg1, Imp::label *label) {
            return new Instruction(OpCode::ZERO, arg1, (*label)++);
        }

        static Instruction *JUMP(Imp::arg arg2, Imp::label *label) {
            return new Instruction(OpCode::JUMP, Imp::NoReg, arg2, (*label)++);
        }

        static Instruction *JZERO(Imp::Reg arg1, Imp::arg arg2, Imp::label *label) {
            return new Instruction(OpCode::JZERO, arg1, arg2, (*label)++);
        }

        static Instruction *JODD(Imp::Reg arg1, Imp::arg arg2, Imp::label *label) {
            return new Instruction(OpCode::JODD, arg1, arg2, (*label)++);
        }

        static Instruction *HALT(Imp::label *label) {
            return new Instruction(OpCode::HALT, (*label)++);
        }

        /*
         * Create instruction directly in output vector
         */

        static void GET(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::GET, arg1, (*label)++));
        }

        static void PUT(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::PUT, arg1, (*label)++));
        }

        static void LOAD(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::LOAD, arg1, (*label)++));
        }

        static void STORE(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::STORE, arg1, (*label)++));
        }

        static void ADD(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::ADD, arg1, (*label)++));
        }

        static void SUB(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::SUB, arg1, (*label)++));
        }

        static void COPY(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::COPY, arg1, (*label)++));
        }

        static void SHR(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::SHR, arg1, (*label)++));
        }

        static void SHL(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::SHL, arg1, (*label)++));
        }

        static void INC(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::INC, arg1, (*label)++));
        }

        static void DEC(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::DEC, arg1, (*label)++));
        }

        static void ZERO(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::label *label) {
            out.push_back(new Instruction(OpCode::ZERO, arg1, (*label)++));
        }

        static void JUMP(std::vector<Instruction*>& out, Imp::arg arg2, Imp::label *label) {
            out.push_back(new Instruction(OpCode::JUMP, Imp::NoReg, arg2, (*label)++));
        }

        static void JZERO(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::arg arg2, Imp::label *label) {
            out.push_back(new Instruction(OpCode::JZERO, arg1, arg2, (*label)++));
        }

        static void JODD(std::vector<Instruction*>& out, Imp::Reg arg1, Imp::arg arg2, Imp::label *label) {
            out.push_back(new Instruction(OpCode::JODD, arg1, arg2, (*label)++));
        }

        static void HALT(std::vector<Instruction*>& out, Imp::label *label) {
            out.push_back(new Instruction(OpCode::HALT, (*label)++));
        }

    private:
        Instruction(OpCode op, Imp::label label)
            : op(op), arg1(Imp::NoReg), arg2(NoArg), label(label) {}
        Instruction(OpCode op, Imp::Reg reg, Imp::label label)
            : op(op), arg1(reg), arg2(NoArg), label(label) {}
        Instruction(OpCode op, Imp::Reg reg, Imp::arg arg2, Imp::label label)
            : op(op), arg1(reg), arg2(arg2), label(label) {}
};

std::ostream & operator<<(std::ostream &stream, const Instruction &instruction);
#endif