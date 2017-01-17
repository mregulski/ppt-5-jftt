#include <algorithm>
#include <iostream>
#include <sstream>
#include "Node.h"
#include "SymTable.h"
#include "Inter.h"
#include "CodeGen.h"
#include "Colors.h"
/* alphabetic order */

using namespace std;
using namespace Imp;
extern SymTable symbols;
extern CodeGen generator;
namespace Imp {
    vector<Instruction*> generate_number(int64_t number, Imp::Reg target_reg, Imp::label *lbl);
    void insert_back(vector<Instruction*>& target, vector<Instruction*>& stuff);

    void insert_back(vector<Instruction*>& target, const vector<Instruction*>& stuff);

    /*====================
        PROGRAM
    ====================*/

    vector<Instruction*> Program::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        // decl->gen_ir(lbl, R0);
        vector<Instruction*> output = code->gen_ir(lbl, R1);
        output.push_back(Instruction::HALT(lbl));
        return output;
    }

    vector<Instruction*> Declarations::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        // declare all the symbols
        for(Id *id : ids) {
            symbols.declare(id);
        }
        return vector<Instruction*>();
    }

    /* @done seriously though, don't actually declare stuff, bison does it. */
    void Declarations::declare(Id *id) {
        ids.push_back(id);
    }

    /* @done do nothing lel */
    vector<Instruction*> Skip::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        return vector<Instruction*>();
    }

    /*====================
        COMMANDS
    ====================*/

    /* @done generate all teh commandses */
    vector<Instruction*> Commands::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> output;
        for(Command *cmd : cmds) {
            vector<Instruction*> cmd_out = cmd->gen_ir(lbl, R1);
            output.insert(output.end(), cmd_out.begin(), cmd_out.end());
        }
        return output;
    }

    void Commands::add_command(Command *cmd) {
        cmds.push_back(cmd);
    }

    /* @done Assign a new value to a variable */
    vector<Instruction*> Assign::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out = expr->gen_ir(lbl, R1);
        if(expr->op() == "Const" && !expr->left->isConst()) {
            Instruction::LOAD(out, R1, lbl);
        }
        vector<Instruction*> location = id->gen_ir(lbl, R0);
        out.insert(out.end(), location.begin(), location.end());
        Instruction::STORE(out, R1, lbl);

        return out;
    }

    /* @done Generate a branch */
    vector<Instruction*> If::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out = cond->gen_ir(lbl, R1);
        // current label = condition's END; r1 == condition.
        // ifZero r1 goto else
        // goto then
        // fill in the target later
        Instruction *jump_else = Instruction::JZERO(R1, Instruction::NoArg, lbl);
        out.push_back(jump_else);

        // do_then
        auto branch_then = do_then->gen_ir(lbl, R1);
        out.insert(out.end(), branch_then.begin(), branch_then.end());
        // fill in the target later
        Instruction *jump_end = Instruction::JUMP(Instruction::NoArg, lbl);
        out.push_back(jump_end);

        // do_else
        auto branch_else = do_else->gen_ir(lbl, R1);
        out.insert(out.end(), branch_else.begin(), branch_else.end());
        // now that we know jump_else's target - update it
        if(branch_else.size() > 0) {
            jump_else->arg2 = branch_else[0]->label;
        } else {
            jump_else->arg2 = *lbl;
        }

        jump_end->arg2 = *lbl;

        return out;
    }

    /* @done generate a while loop */
    vector<Instruction*> While::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out = cond->gen_ir(lbl, R1);
        label start = out[0]->label;
        Instruction *jump_end = Instruction::JZERO(R1, Instruction::NoArg, lbl);
        out.push_back(jump_end);
        auto loop_body = body->gen_ir(lbl, reg);
        out.insert(out.end(), loop_body.begin(), loop_body.end());
        Instruction::JUMP(out, start, lbl);
        jump_end->arg2 = *lbl;
        return out;
    }


    /*TODO*/
    vector<Instruction*> For::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        // for: <Id> iterator, <Value> from, to, <Commands> body
        if (!symbols.declare(iterator)) {
            ostringstream os;
            os  << "Duplicate declaration of " << iterator->name
                << ": first declared in line "
                << symbols.get_var(iterator->name).line
                << endl;
            generator.report(os, line);
            return vector<Instruction*>();
        }

        // TODO: check if body contains assignments to the iterator

        Symbol iter = symbols.get_var(iterator->name);

        // BEGIN LOOP
        // Initialize loop
        vector<Instruction*> out;
        // r1 = FROM
        if (from->isConst()) {
            insert_back(out, generate_number(from->value, R1, lbl));
        } else {
            insert_back(out, from->gen_ir(lbl, R0));
            Instruction::LOAD(out, R1, lbl);
        }
        // iterator = FROM
        insert_back(out, iterator->gen_ir(lbl, R0));
        Instruction::STORE(out, R1, lbl);
        Symbol to_var;
        if (to->isConst()) {
            // initialize _TO
            symbols.declare_tmp("_TO");
            to_var = symbols.get_tmp("_TO");
            // to = r2 = TO
            insert_back(out, generate_number(to_var.offset, R0, lbl));
            insert_back(out, generate_number(to->value, R2, lbl));
            Instruction::STORE(out, R2, lbl);
        } else {
            insert_back(out, to->gen_ir(lbl, R0));
            to_var = symbols.get_var(to->id->name);
        }

        // END initialization
        label loop = *lbl;
        if(isDownTo) {

            insert_back(out, generate_number(to_var.offset, R0, lbl));

            Instruction::LOAD(out, R1, lbl);

            insert_back(out, iterator->gen_ir(lbl, R0));

            Instruction::SUB(out, R1, lbl);
        }
        else {
            // r0 = &iter
            insert_back(out, iterator->gen_ir(lbl, R0));
            // r1 = iter
            Instruction::LOAD(out, R1, lbl);
            // r0 = &_TO
            insert_back(out, generate_number(to_var.offset, R0, lbl));
            // r1 = iter - TO
            Instruction::SUB(out, R1, lbl);
        }
        // skip_init->arg2 = *lbl;
        Instruction *jump_end;
        Instruction::JZERO(out, R1, *lbl+2, lbl);
        jump_end = Instruction::JUMP(Instruction::NoArg, lbl);
        out.push_back(jump_end);

        insert_back(out, body->gen_ir(lbl, R1));

        // increment/decrement counter
        insert_back(out, iterator->gen_ir(lbl, R0));
        Instruction::LOAD(out, R1, lbl);
        Instruction *iter_zero_check = NULL;
        if (isDownTo) {
            iter_zero_check = Instruction::JZERO(R1, Instruction::NoArg, lbl);
            out.push_back(iter_zero_check);
            Instruction::DEC(out, R1, lbl);
        } else {
            Instruction::INC(out, R1, lbl);
        }
        Instruction::STORE(out, R1, lbl);


        Instruction::JUMP(out, loop, lbl);
        jump_end->arg2 = *lbl;
        if(iter_zero_check) {
            iter_zero_check->arg2=*lbl;
        }

        // END LOOP

        symbols.undeclare("_TO");

        symbols.undeclare(iterator->name);
        cerr << "FOR: labels " << out.front()->label << " - "
            << out.back()->label << endl;
        return out;
    }

    /* @done Read input into a variable */
    vector<Instruction*> Read::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        auto id_addr = id->gen_ir(lbl, R0);
        insert_back(out, id_addr);
        // out.insert(out.end(), id_addr.begin(), id_addr.end());
        Instruction::GET(out, reg, lbl);
        Instruction::STORE(out, reg, lbl);
        return out;
    }

    /* @done Write value to the output */
    vector<Instruction*> Write::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        auto value_ref = val->gen_ir(lbl, reg);
        insert_back(out, value_ref);
        // out.insert(out.end(), value_ref.begin(), value_ref.end());
        if(!val->isConst()) { Instruction::LOAD(out, reg, lbl); }
        Instruction::PUT(out, reg, lbl);
        return out;
    }


    /*====================
        EXPRESSION
    ====================*/

    /* @done put value in reg */
    vector<Instruction*> Const::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        return left->gen_ir(lbl, reg);
    };

    /* @done Calculate left+right, result in r1. */
    vector<Instruction*> Plus::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (left->isConst() && right->isConst()) {
            // 2 compile-time constants: calculate value immediately
            number value = left->value + right->value;
            out = generate_number(value < 0 ? 0 : value, reg, lbl);
        } else if (left->isConst() || right->isConst()) {
            // one constant
            out = left->gen_ir(lbl, reg);
            auto arg2 = right->gen_ir(lbl, reg);
            out.insert(out.end(), arg2.begin(), arg2.end());
            Instruction::ADD(out, reg, lbl);

        } else {
            // no constants
            out = left->gen_ir(lbl, reg);
            Instruction::LOAD(out, reg, lbl);

            auto arg2 = right->gen_ir(lbl, reg);
            out.insert(out.end(), arg2.begin(), arg2.end());
            Instruction::ADD(out, reg, lbl);

        }
        return out;
    }

    /* @done Calculate left - right, result in reg. */
    vector<Instruction*> Minus::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (left->isConst() && right->isConst()) {
            // 2 compile-time constants: calculate value immediately
            number value = left->value - right->value;
            out = generate_number(value < 0 ? 0 : value, reg, lbl);
        } else if (left->isConst()) {
            // one constant
            out = left->gen_ir(lbl, reg);
            auto arg2 = right->gen_ir(lbl, R0);
            out.insert(out.end(), arg2.begin(), arg2.end());
            Instruction::SUB(out, reg, lbl);
        } else if (right->isConst()) {
            // create temp variable for the constant
            symbols.declare_tmp("_SUB");
            auto tmp = symbols.get_tmp("_SUB");
            out = generate_number(tmp.offset, R0, lbl);

            //
            auto constant = right->gen_ir(lbl, reg);
            out.insert(out.end(), constant.begin(), constant.end());
            Instruction::STORE(out, reg, lbl);

            // reg <- left
            auto var = left->gen_ir(lbl, R0);
            out.insert(out.end(), var.begin(), var.end());
            Instruction::LOAD(out, reg, lbl);

            auto tmp_addr = generate_number(tmp.offset, R0, lbl);
            out.insert(out.end(), tmp_addr.begin(), tmp_addr.end());

            Instruction::SUB(out, reg, lbl);

        } else {
            // no constants
            out = left->gen_ir(lbl, reg);
            Instruction::LOAD(out, reg, lbl);

            auto arg2 = right->gen_ir(lbl, reg);
            out.insert(out.end(), arg2.begin(), arg2.end());
            Instruction::SUB(out, reg, lbl);

        }
        return out;
    }

    /* @done Calculate left * right, result in r1 */
    vector<Instruction*> Mult::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (left->isConst() && right->isConst()) {
            // 2 compile-time constants: calculate value immediately
            number value = left->value * right->value;
            out = generate_number(value < 0 ? 0 : value, reg, lbl);
        } else if (left->isConst() || right->isConst()) {
            Value *constant = left->isConst() ? left : right;
            Value *ref = left->isConst() ? right : left;
            if (constant->value == 0) { out = generate_number(0, reg, lbl); }
            else {
                // one constant
                // SHL then ADD or SUB, similar approach as in generate_number
                number multiplier = 1;
                number target = constant->value;
                out = ref->gen_ir(lbl, reg);
                Instruction::LOAD(out, reg, lbl);

                while (multiplier * 2 < target) {
                    Instruction::SHL(out, reg, lbl);
                    multiplier *= 2;
                }
                if (multiplier * 2 - target < target - multiplier) {
                    // go down from multiplier * 2 to target
                    Instruction::SHL(out, reg, lbl);

                    multiplier *= 2;
                    while (multiplier > target) {
                        Instruction::SUB(out, reg, lbl);
                        multiplier--;
                    }
                } else {
                    // go up
                    while (multiplier < target) {
                        Instruction::ADD(out, reg, lbl);
                        multiplier++;
                    }
                }
            }

        } else {
            Instruction::ZERO(out, R1, lbl);

            auto left_ref = left->gen_ir(lbl, R0);
            out.insert(out.end(), left_ref.begin(), left_ref.end());
            Instruction::LOAD(out, R2, lbl);

            auto right_ref = right->gen_ir(lbl, R0);
            out.insert(out.end(), right_ref.begin(), right_ref.end());
            Instruction::LOAD(out, R3, lbl);
            if(!symbols.declare_tmp("_RIGHT")) {
                cerr << "[ERROR] (internal) Unable to declare temporary variable" << endl;
                exit(EXIT_FAILURE);
            }
            auto tmp_ref = generate_number(symbols.get_tmp("_RIGHT").offset, R0, lbl);
            out.insert(out.end(), tmp_ref.begin(), tmp_ref.end());
            Instruction::STORE(out, R3, lbl);

            auto loop_start = *lbl;
            Instruction::JODD(out, R2, *lbl+2, lbl);
            Instruction::JUMP(out, *lbl+2, lbl);
            Instruction::ADD(out, R1, lbl);

            Instruction::SHL(out, R3, lbl);
            Instruction::STORE(out, R3, lbl);
            Instruction::SHR(out, R2, lbl);
            Instruction::JZERO(out, R2, *lbl+2, lbl);
            Instruction::JUMP(out, loop_start, lbl);
            symbols.undeclare("_RIGHT");
        }
        return out;
    }

    vector<Instruction*> load_value(Value *value, Reg reg, label *lbl) {
        if (value->isConst()) {
            return value->gen_ir(lbl, reg);
        }
            auto out = value->gen_ir(lbl, R0);
            Instruction::LOAD(out, reg, lbl);
            return out;
    }
    /*TODO*/
    vector<Instruction*> Div::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        // 2 constants
        if (left->isConst() && right->isConst()) {
            return generate_number(left->value / left->value, R1, lbl);
        }
        if (right->isConst()) {
            if (right->value == 1) {
                out = left->gen_ir(lbl, R0);
                Instruction::LOAD(out, R1, lbl);
                return out;
            } else if ((right->value & (right->value - 1)) == 0) {
                out = left->gen_ir(lbl, R0);
                Instruction::LOAD(out, R1, lbl);
                // right is power of 2
                long long val = right->value;
                cerr << "debug: division by " << val << endl;
                while (val > 1) {
                    Instruction::SHR(out, R1, lbl);
                    val /= 2;
                }
                cerr << out.size() << " shifts" << endl;
                return out;
            }
        }

        /*DUM DUM DUM*/
        if (! (symbols.declare_tmp("_SCALED")
            && symbols.declare_tmp("_REMAIN")
            && symbols.declare_tmp("_RESULT")
            && symbols.declare_tmp("_MULT"))) {
                cerr << "[Internal error] Div::gen_ir: Unable to declare temporary variable."
                    << endl;
                exit(EXIT_FAILURE);
        }
        auto a = left;
        auto b = right;
        Symbol scaled = symbols.get_var("_SCALED");
        Symbol remain = symbols.get_var("_REMAIN");
        Symbol result = symbols.get_var("_RESULT");
        Symbol mult = symbols.get_var("_MULT");

        // Wipe the fuck outta' temps
        Instruction::ZERO(out, R1, lbl);
        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        insert_back(out, generate_number(result.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);


        insert_back(out, load_value(b, R1, lbl));

        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);

        insert_back(out, load_value(a, R1, lbl));

        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        Instruction::ZERO(out, R2, lbl);
        Instruction::ZERO(out, R3, lbl);
        Instruction::INC(out, R3, lbl);

        auto loop1 = *lbl;
        insert_back(out, load_value(a, R1, lbl));

        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::LOAD(out, R4, lbl);
        Instruction::SUB(out, R1, lbl);

        // jzero 1 loop2
        auto loop1_jzero = Instruction::JZERO(R1, Instruction::NoArg, lbl);
        out.push_back(loop1_jzero);
        Instruction::SHL(out, R4, lbl);
        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R4, lbl);
        Instruction::SHL(out, R3, lbl);
        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::STORE(out, R3, lbl);
        Instruction::JUMP(out, loop1, lbl);

        auto loop2 = *lbl;
        loop1_jzero->arg2 = loop2;
        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::STORE(out, R3, lbl);

        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R4, lbl);
        Instruction::LOAD(out, R1, lbl);

        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::SUB(out, R1, lbl);
        Instruction::JZERO(out, R1, *lbl+2, lbl);
        auto jump_end = Instruction::JUMP(Instruction::NoArg, lbl);
        out.push_back(jump_end);

        Instruction::LOAD(out, R1, lbl);
        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::SUB(out, R1, lbl);

        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);

        insert_back(out, generate_number(result.offset, R0, lbl));
        Instruction::LOAD(out, R1, lbl);

        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::ADD(out, R1, lbl);
        insert_back(out, generate_number(result.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);

        auto end2 = *lbl;
        jump_end->arg2 = end2;

        Instruction::SHR(out, R3, lbl);
        Instruction::SHR(out, R4, lbl);
        Instruction::JZERO(out, R3, *lbl+2, lbl);
        Instruction::JUMP(out, loop2, lbl);


        insert_back(out, generate_number(result.offset, R0, lbl));
        Instruction::LOAD(out, R1, lbl);
        symbols.undeclare("_SCALED");
        symbols.undeclare("_REMAIN");
        symbols.undeclare("_RESULT");
        symbols.undeclare("_MULT");
        return out;

    }

    /*TODO*/
    vector<Instruction*> Mod::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        if (left->isConst() && right->isConst()) {
            return generate_number(left->value % left->value, R1, lbl);
        }
        if (right->isConst()) {
            if (right->value == 1) {
                auto out = left->gen_ir(lbl, R0);
                Instruction::LOAD(out, R1, lbl);
                return out;
            }
        }
        if (! (symbols.declare_tmp("_SCALED")
            && symbols.declare_tmp("_REMAIN")
            && symbols.declare_tmp("_RESULT")
            && symbols.declare_tmp("_MULT"))) {
                cerr << "[Internal error] Div::gen_ir: Unable to declare temporary variable."
                    << endl;
                exit(EXIT_FAILURE);
        }

        vector<Instruction*> out;
        auto a = left;
        auto b = right;
        Symbol scaled = symbols.get_var("_SCALED");
        Symbol remain = symbols.get_var("_REMAIN");
        Symbol result = symbols.get_var("_RESULT");
        Symbol mult = symbols.get_var("_MULT");

        // Wipe the fuck outta' temps
        Instruction::ZERO(out, R1, lbl);
        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        insert_back(out, generate_number(result.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);

        insert_back(out, load_value(b, R1, lbl));

        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);

        insert_back(out, load_value(a, R1, lbl));

        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);
        Instruction::ZERO(out, R2, lbl);
        Instruction::ZERO(out, R3, lbl);
        Instruction::INC(out, R3, lbl);

        auto loop1 = *lbl;
        insert_back(out, load_value(a, R1, lbl));

        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::LOAD(out, R4, lbl);
        Instruction::SUB(out, R1, lbl);

        // jzero 1 loop2
        auto loop1_jzero = Instruction::JZERO(R1, Instruction::NoArg, lbl);
        out.push_back(loop1_jzero);
        Instruction::SHL(out, R4, lbl);
        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R4, lbl);
        Instruction::SHL(out, R3, lbl);
        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::STORE(out, R3, lbl);
        Instruction::JUMP(out, loop1, lbl);

        auto loop2 = *lbl;
        loop1_jzero->arg2 = loop2;
        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::STORE(out, R3, lbl);

        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::STORE(out, R4, lbl);
        Instruction::LOAD(out, R1, lbl);

        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::SUB(out, R1, lbl);
        Instruction::JZERO(out, R1, *lbl+2, lbl);
        auto jump_end = Instruction::JUMP(Instruction::NoArg, lbl);
        out.push_back(jump_end);

        Instruction::LOAD(out, R1, lbl);
        insert_back(out, generate_number(scaled.offset, R0, lbl));
        Instruction::SUB(out, R1, lbl);

        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);

        insert_back(out, generate_number(result.offset, R0, lbl));
        Instruction::LOAD(out, R1, lbl);

        insert_back(out, generate_number(mult.offset, R0, lbl));
        Instruction::ADD(out, R1, lbl);
        insert_back(out, generate_number(result.offset, R0, lbl));
        Instruction::STORE(out, R1, lbl);

        auto end2 = *lbl;
        jump_end->arg2 = end2;

        Instruction::SHR(out, R3, lbl);
        Instruction::SHR(out, R4, lbl);
        Instruction::JZERO(out, R3, *lbl+2, lbl);
        Instruction::JUMP(out, loop2, lbl);
        insert_back(out, generate_number(remain.offset, R0, lbl));
        Instruction::LOAD(out, R1, lbl);
        symbols.undeclare("_SCALED");
        symbols.undeclare("_REMAIN");
        symbols.undeclare("_RESULT");
        symbols.undeclare("_MULT");
        return out;
    }

    /*====================
        CONDITIONS
    ====================*/

    /* @done Set r1 to 1 if left == right, 0 otherwise*/
    vector<Instruction*> Eq::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        // r1 <- 0
        Instruction::ZERO(out, R1, lbl);

        // r2 <- left - right
        auto sub_lr = Minus(left, right, line).gen_ir(lbl, R2);
        out.insert(out.end(), sub_lr.begin(), sub_lr.end());

        // jump START (target will be set after second subtraction's length is known)
        // Instruction jump_start = Instruction::JUMP(Instruction::NoArg, lbl);
        Instruction::JUMP(out, Instruction::NoArg, lbl);
        Instruction *jump_start = out.back();
        // r2 <- right - left
        auto sub_rl = Minus(right, left, line).gen_ir(lbl, R2);
        out.insert(out.end(), sub_rl.begin(), sub_rl.end());
        label sub2 = sub_rl[0]->label;

        // jump MAYBE
        Instruction::JUMP(out, sub_rl.back()->label + 4, lbl);

        // ifZero r2 jump SUB2
        Instruction *cmp_1 = Instruction::JZERO(R2, sub2, lbl);
        // START is now known; update jump_start
        jump_start->arg2 = cmp_1->label;
        out.push_back(cmp_1);

        // jump END
        Instruction::JUMP(out, 4+*lbl, lbl); // TODO: check if it's not off-by-one

        // ifZero r2 jump TRUE
        Instruction::JZERO(out, R2, 2 + *lbl, lbl);

        // jump END
        Instruction::JUMP(out, 2 + *lbl, lbl);

        // TRUE: inc 1
        Instruction::INC(out, R1, lbl);

        return out;
    }

    /* @done Set R1 to 1 if left != right, 0 otherwise */
    vector<Instruction*> Neq::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        // r1 <- 0
        Instruction::ZERO(out, R1, lbl);
        Instruction::INC(out, R1, lbl);

        // r2 <- left - right
        auto sub_lr = Minus(left, right, line).gen_ir(lbl, R2);
        out.insert(out.end(), sub_lr.begin(), sub_lr.end());

        // jump START (target will be set after second subtraction's length is known)
        // Instruction jump_start = Instruction::JUMP(Instruction::NoArg, lbl);
        Instruction::JUMP(out, Instruction::NoArg, lbl);
        Instruction *jump_start = out.back();
        // r2 <- right - left
        auto sub_rl = Minus(right, left, line).gen_ir(lbl, R2);
        out.insert(out.end(), sub_rl.begin(), sub_rl.end());
        label sub2 = sub_rl[0]->label;

        // jump MAYBE
        Instruction::JUMP(out, sub_rl.back()->label + 4, lbl);

        // ifZero r2 jump SUB2
        Instruction *cmp_1 = Instruction::JZERO(R2, sub2, lbl);
        // START is now known; update jump_start
        jump_start->arg2 = cmp_1->label;
        out.push_back(cmp_1);

        // jump END
        Instruction::JUMP(out, 4+*lbl, lbl); // TODO: check if it's not off-by-one

        // ifZero r2 jump TRUE
        Instruction::JZERO(out, R2, 2 + *lbl, lbl);

        // jump END
        Instruction::JUMP(out, 2 + *lbl, lbl);

        // TRUE: inc 1
        Instruction::DEC(out, R1, lbl);

        return out;
    }

    /* @done set R1 to 1 if left > right, 0 otherwise */
    vector<Instruction*> Gt::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out = Minus(left, right, line).gen_ir(lbl, R2);
        Instruction::ZERO(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::INC(out, R1, lbl);
        return out;
    }

    /* @done set R1 to 1 if left < right, 0 otherwise */
    vector<Instruction*> Lt::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out = Minus(right, left, line).gen_ir(lbl, R2);
        Instruction::ZERO(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::INC(out, R1, lbl);
        return out;
    }

    /* @done set R1 to 1 if left <= right, 0 otherwise */
    vector<Instruction*> Leq::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out = Minus(left, right, line).gen_ir(lbl, R2);
        Instruction::ZERO(out, R1, lbl);
        Instruction::INC(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::DEC(out, R1, lbl);
        return out;

    }

    /* @done set R1 to 1 if left >= right, 0 otherwise */
    vector<Instruction*> Geq::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out = Minus(right, left, line).gen_ir(lbl, R2);
        Instruction::ZERO(out, R1, lbl);
        Instruction::INC(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::DEC(out, R1, lbl);
        return out;
    }

    /*====================
        VARIABLES
    ====================*/
    /* @done reg = value || r0 = value */
    vector<Instruction*> Value::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (value != -1) {
            out = generate_number(value, reg, lbl);
        } else {
            out = id->gen_ir(lbl, R0);
        }
        return out;
    }

    /* @done reg = &value*/
    vector<Instruction*> Var::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        Symbol var = symbols.get_var(name);
        if (var.line == Symbol::Undefined) {
            ostringstream os;
            os << "Undeclared variable " << name << "." << endl;
            generator.report(os, line);
        }
        return generate_number(var.offset, reg, lbl);
    }

    /* @done reg = &arr[idx] */
    vector<Instruction*> ConstArray::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        Symbol arr = symbols.get_var(name);
        if (idx >= arr.size) {
            ostringstream os;
            os << "Attempt to access array " << arr.name
                << " at index " << idx
                << "(size: " << arr.size << ")." << endl;
            generator.report(os, line);
        }

        return generate_number(arr.offset+idx, reg, lbl);

    }

    /* @done reg = &arr[idx] */
    vector<Instruction*> VarArray::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        // if (reg == R0) {
        //     cerr << "possible register overwrite" << endl;
        // }
        Symbol arr = symbols.get_var(name);
        auto out = generate_number(arr.offset, R4, lbl);
        auto idx_val = idx->gen_ir(lbl, R0);
        out.insert(out.end(), idx_val.begin(), idx_val.end());
        Instruction::ADD(out, R4, lbl);
        Instruction::COPY(out, R4, lbl);
        return out;

    }

    /* @done reg = number */
    vector<Instruction*> generate_number(number number, Imp::Reg target_reg, Imp::label *lbl) {
        vector<Instruction*> out;
        Instruction::ZERO(out, target_reg, lbl);
        if (number == 0) return out;

        vector<char> helper;
        while (number > 0) {
            if (number % 2 == 0) {
                helper.push_back('s');
                // Instruction::SHL(out, target_reg, lbl);
                number /= 2;
            } else {
                helper.push_back('i');
                // Instruction::INC(out, target_reg, lbl);
                number -= 1;
            }
        }
        reverse(begin(helper), end(helper));
        for(char c: helper) {
            if(c == 's') {
                Instruction::SHL(out, target_reg, lbl);
            } else if (c == 'i') {
                Instruction::INC(out, target_reg, lbl);
            } else {
                cerr << " WTF just happened?!" << endl;
            }
        }
        return out;
    }

    void insert_back(vector<Instruction*>& target, vector<Instruction*>& stuff) {
        target.insert(target.end(), stuff.begin(), stuff.end());
    }

    void insert_back(vector<Instruction*>& target, const vector<Instruction*>& stuff) {
        target.insert(target.end(), stuff.begin(), stuff.end());
    }
}