#include <algorithm>
#include <iostream>
#include <sstream>
#include "node.h"
#include "symtable.h"
#include "inter.h"
#include "codegen.h"
#include "colors.h"

#ifdef DEBUG
#ifndef INFO
#define INFO 1
#endif
#endif


using namespace std;
using namespace Imp;
extern SymTable symbols;
extern CodeGen generator;
namespace Imp {
    vector<Instruction*> generate_number(int64_t number, Imp::Reg target_reg, Imp::label *lbl);
    void insert_back(vector<Instruction*>& target, vector<Instruction*>& stuff);

    void insert_back(vector<Instruction*>& target, const vector<Instruction*>& stuff);

    bool check_init(Id *id);
    bool check_init(Value *value);
    void log_DEBUG(string msg) {
        #ifdef DEBUG
        cerr << Color::yellow << "DEBUG: " << Color::def << msg << endl;
        #endif
    }
    void log_INFO(string msg) {
        #ifdef INFO
        cerr << Color::cyan << "INFO: " << Color::def << msg << endl;
        #endif
    }

    vector<Instruction*> load_value(Value *value, Reg reg, label *lbl) {
        if (value->isConst()) {
            return value->gen_ir(lbl, reg);
        }
        auto out = value->gen_ir(lbl, R0);
        Instruction::LOAD(out, reg, lbl);
        return out;
    }

    /*========================================
        PROGRAM
    ========================================*/

    vector<Instruction*> Program::gen_ir(Imp::label *lbl, Imp::Reg reg) {

        decl->gen_ir(lbl, R0);
        vector<Instruction*> output = code->gen_ir(lbl, R1);
        output.push_back(Instruction::HALT(lbl));

        return output;
    }

    void err_redeclaration(string tag, Id *id) {
        ostringstream os;
        os  << "[" << tag << "] Duplicate declaration of " << id->name
            << ": first declared in line "
            << symbols.get_var(id->name).line;
        generator.report(os, id->line);
    }

    /*
        Declare all the symbols.
        Memory layout:
        [TMPS | VARS | 2*for_counter cells | ARRAYS ]
        FOR loops declare their identifiers and _TO temporaries in the empty space.
        Kinda wasteful, but better than leaving them behind the arrays.
    */
    vector<Instruction*> Declarations::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        // predeclare tmps
        for(int i = 0; i < 4; i++) {
            symbols.declare_tmp("_TMP"+to_string(i));
        }
        // log_DEBUG("DECLARATIONS");
        vector<Id*> vars;
        vector<Id*> iters;
        vector<Id*> arrays;
        // split ids by type
        for(Id *id : ids) {
            if (id->isArray()) {
                arrays.push_back(id);
            } else if (id->isIter()) {
                iters.push_back(id);
            } else {
                vars.push_back(id);
            }
        }
        // #ifdef DEBUG
        // ostringstream dbg;
        // dbg << "Variables: " << vars.size() << endl
        //     << "\tArrays: " << arrays.size() << endl
        //     << "\tIterators: " << iters.size() << endl;
        // log_DEBUG(dbg.str());
        // #endif
        for (Id *id : vars) {
            if (!symbols.declare(id)) {
                err_redeclaration("DECL", id);
            }
        }
        symbols.alloc_for_control(for_counter);
        for (Id *id : arrays) {
            if(!symbols.declare(id)) {
                err_redeclaration("DECL", id);
            }
            symbols.set_array(id);
        }

        return vector<Instruction*>();
    }

    /* @done seriously though, don't actually declare stuff, bison does it. */
    void Declarations::declare(Id *id) {
        ids.push_back(id);
    }

    /* @done do nothing lel */
    vector<Instruction*> Skip::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        log_DEBUG("SKIP");
        return vector<Instruction*>();
    }

    /*========================================
        COMMANDS
    ========================================*/

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

        vector<Instruction*> out;
        symbols.set_initialized(id);
        if (symbols.is_iterator(id)) {
            ostringstream os;
            os << "Attempt to modify the iterator in a FOR loop: " << id->name;
            generator.report(os, line);
            return out;
        }
        insert_back(out, expr->gen_ir(lbl, R1));
        if(expr->op() == "Const" && !expr->left->isConst()) {
            Instruction::LOAD(out, R1, lbl);
        }

        insert_back(out, id->gen_ir(lbl, R0));
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


    /*@done*/
    vector<Instruction*> For::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        // for: <Id> iterator, <Value> from, to, <Commands> body
        vector<Instruction*> out;
        log_INFO("FOR: " + to_string(id));
        if (!symbols.declare_iterator(iterator)) {
            ostringstream os;
            os  << "Duplicate declaration of " << iterator->name
                << ": first declared in line "
                << symbols.get_var(iterator->name).line;
            generator.report(os, iterator->line);
            return out;
        }

        if (! (check_init(from) && check_init(to))) {
            return out;
        }
        // TODO: check if body contains assignments to the iterator
        // @done in Assign

        Symbol iter = symbols.get_var(iterator->name);
        symbols.set_initialized(iterator);
        symbols.set_iterator(iterator);
        // BEGIN LOOP
        // Initialize loop

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

        // prepare _TO temporary
        Id *_to = new Var("_TO_" + to_string(id), Id::NORMAL, line);
        if(!symbols.declare_iterator(_to)) {
            cerr << "CANNOT DECLARE _TO: LINE "<< line << endl;
            exit(EXIT_FAILURE);
        };
        to_var = symbols.get_var(_to->name);

        if (to->isConst()) {
            // initialize _TO
            // to = r2 = TO
            insert_back(out, generate_number(to_var.offset, R0, lbl));
            insert_back(out, generate_number(to->value, R2, lbl));
            Instruction::STORE(out, R2, lbl);
        } else {
            insert_back(out, to->gen_ir(lbl, R0));
            Instruction::LOAD(out, R2, lbl);
            insert_back(out, generate_number(to_var.offset, R0, lbl));
            Instruction::STORE(out, R2, lbl);
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

        symbols.undeclare_iter(_to->name);

        symbols.undeclare_iter(iterator->name);

        return out;
    }

    /* @done Read input into a variable */
    vector<Instruction*> Read::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        symbols.set_initialized(id);

        vector<Instruction*> out;
        insert_back(out, id->gen_ir(lbl, R0));
        Instruction::GET(out, reg, lbl);
        Instruction::STORE(out, reg, lbl);
        return out;
    }

    /* @done Write value to the output */
    vector<Instruction*> Write::gen_ir(Imp::label *lbl, Imp::Reg reg) {

        vector<Instruction*> out;
        if (! check_init(val)) {
            return out;
        }
        out = load_value(val, R1, lbl);
        // insert_back(out, val->gen_ir(lbl, R0));

        // if(!val->isConst()) { Instruction::LOAD(out, reg, lbl); }
        Instruction::PUT(out, R1, lbl);

        return out;
    }


    /*========================================
        EXPRESSION
    ========================================*/

    /* @done put value in reg */
    vector<Instruction*> Const::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        if (! check_init(left)) {
            return vector<Instruction*>();
        }
        return load_value(left, reg, lbl);
        // out = left->gen_ir(lbl, reg);

    };

    /* @done Calculate left+right, result in r1. */
    vector<Instruction*> Plus::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        if (left->isConst() && right->isConst()) {
            // 2 compile-time constants: calculate value immediately
            number value = left->value + right->value;
            out = generate_number(value < 0 ? 0 : value, R1, lbl);
        } else if (left->isConst() || right->isConst()) {
            // one constant
            auto constant = left->isConst() ? left : right;
            auto variable = left->isConst() ? right : left;
            if (constant->value == 1) {
                insert_back(out, load_value(variable, R1, lbl));
                Instruction::INC(out, R1, lbl);
            } else {
                out = constant->gen_ir(lbl, R1);
                insert_back(out, variable->gen_ir(lbl, R0));
                Instruction::ADD(out, R1, lbl);
            }
        } else {
            // no constants
            out = left->gen_ir(lbl, R0);
            Instruction::LOAD(out, R1, lbl);
            insert_back(out, right->gen_ir(lbl, R0));
            Instruction::ADD(out, R1, lbl);
        }
        return out;
    }

    /* @done Calculate left - right, result in R1. */
    vector<Instruction*> Minus::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        if (left->isConst() && right->isConst()) {
            // 2 compile-time constants: calculate value immediately
            number value = left->value - right->value;
            out = generate_number(value < 0 ? 0 : value, reg, lbl);
        } else if (left->isConst()) {
            // one constant
            out = left->gen_ir(lbl, reg);
            insert_back(out, right->gen_ir(lbl, R0));
            Instruction::SUB(out, reg, lbl);
        } else if (right->isConst()) {
            // use a temporary variable for the constant
            auto tmp = symbols.get_var("_TMP0");
            out = generate_number(tmp.offset, R0, lbl);
            // clear tmp
            Instruction::ZERO(out, reg, lbl);
            Instruction::STORE(out, reg, lbl);

            insert_back(out, right->gen_ir(lbl, reg));

            Instruction::STORE(out, reg, lbl);

            // R1 <- left
            insert_back(out, left->gen_ir(lbl, R0));
            Instruction::LOAD(out, reg, lbl);

            insert_back(out, generate_number(tmp.offset, R0, lbl));

            Instruction::SUB(out, reg, lbl);

        } else {
            // no constants
            out = left->gen_ir(lbl, R0);
            Instruction::LOAD(out, reg, lbl);

            insert_back(out, right->gen_ir(lbl, R0));
            Instruction::SUB(out, reg, lbl);
        }
        return out;
    }

    /* @done Calculate left * right, result in r1 */
    vector<Instruction*> Mult::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        if (left->isConst() && right->isConst()) {
            // 2 compile-time constants: calculate value immediately
            number value = left->value * right->value;
            return generate_number(value < 0 ? 0 : value, R1, lbl);
        } else if (left->isConst() || right->isConst()) {
            Value *constant = left->isConst() ? left : right;
            Value *ref = left->isConst() ? right : left;
            if (constant->value == 0) { out = generate_number(0, R1, lbl); return out; }
            else if (constant->value == 1) {
                out = ref->gen_ir(lbl, R0);
                Instruction::LOAD(out, R1, lbl);
                return out;
            }
            else {
                log_DEBUG("Mult: var/const");
                // one constant
                // SHL then ADD or SUB, similar approach as in generate_number
                number multiplier = 1;
                number target = constant->value;
                out = ref->gen_ir(lbl, R0);
                Instruction::LOAD(out, R1, lbl);

                while (multiplier * 2 < target) {
                    Instruction::SHL(out, R1, lbl);
                    multiplier *= 2;
                }
                if (multiplier * 2 - target < target - multiplier) {
                    // go down from multiplier * 2 to target
                    Instruction::SHL(out, R1, lbl);

                    multiplier *= 2;
                    while (multiplier > target) {
                        Instruction::SUB(out, R1, lbl);
                        multiplier--;
                    }
                } else {
                    // go up
                    while (multiplier < target) {
                        Instruction::ADD(out, R1, lbl);
                        multiplier++;
                    }
                }
                return out;
            }
        } else {
            log_DEBUG("Mult: var/var");
            log_DEBUG("var_left: name: " + left->id->name);
            log_DEBUG("var_right: name: " + right->id->name);
            // // old version: always generates left * right
            // Instruction::ZERO(out, R1, lbl);
            // auto left_ref = left->gen_ir(lbl, R0);
            // out.insert(out.end(), left_ref.begin(), left_ref.end());
            // Instruction::LOAD(out, R2, lbl);

            // auto right_ref = right->gen_ir(lbl, R0);
            // out.insert(out.end(), right_ref.begin(), right_ref.end());
            // Instruction::LOAD(out, R3, lbl);

            // Make sure we're generating smaller * larger

            auto var_left = symbols.get_var(left->id->name);
            auto var_right = symbols.get_var(right->id->name);

            // insert_back(out, generate_number(var_left.offset, R2, lbl));
            // insert_back(out, generate_number(var_right.offset, R3, lbl));
            insert_back(out, left->gen_ir(lbl, R2));
            insert_back(out, right->gen_ir(lbl, R3));

            Instruction::COPY(out, R2, lbl);
            Instruction::LOAD(out, R1, lbl);

            Instruction::COPY(out, R3, lbl);
            Instruction::SUB(out, R1, lbl);

            /*
                Ensure proper ordering of arguments.
                Note: calculation is always R2 * R3.
                Before:
                    R1 = left - right.
                    R2 = &left
                    R3 = &right
                After:
                    R2 = smaller
                    R3 = larger
            */
            auto select = Instruction::JZERO(R1, Instruction::NoArg, lbl);
            out.push_back(select);
            // R1 == 1: left > right -> calculate left * right (no swap)
            /*
                copy higer address, recreate lower
                Before:
                    R2 = &left
                    R3 = &right
                After:
                    R2 <- right
                    R3 <- left
            */
            if (var_left.offset > var_right.offset) {
                Instruction::COPY(out, R2, lbl);
                Instruction::LOAD(out, R3, lbl);
                insert_back(out, right->gen_ir(lbl, R0));
                Instruction::LOAD(out, R2, lbl);
            } else {
                Instruction::COPY(out, R3, lbl);
                Instruction::LOAD(out, R2, lbl);
                insert_back(out, left->gen_ir(lbl, R0));
                Instruction::LOAD(out, R3, lbl);
            }
            auto select_end = Instruction::JUMP(Instruction::NoArg, lbl);
            out.push_back(select_end);
            // R1 == 0: left <= right -> calculate right * left (swap)
            auto case_swap = *lbl;
            select->arg2 = case_swap;
            Instruction::COPY(out, R2, lbl);
            Instruction::LOAD(out, R2, lbl);
            Instruction::COPY(out, R3, lbl);
            Instruction::LOAD(out, R3, lbl);


            auto after_select = *lbl;
            select_end->arg2 = after_select;
            // END select

            // generate actual calculation
            Instruction::ZERO(out, R1, lbl);
            auto tmp = symbols.get_var("_TMP0");
            insert_back(out, generate_number(tmp.offset, R0, lbl));

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

            insert_back(out, generate_number(tmp.offset, R0, lbl));
            Instruction::ZERO(out, R3, lbl);
            Instruction::STORE(out, R3, lbl);
            return out;
        }
    }

    /*TODO*/
    vector<Instruction*> Div::gen_ir(Imp::label *lbl, Imp::Reg reg) {

        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        // 2 constants
        if (left->isConst() && right->isConst()) {

            if (right->value == 0) {
                return generate_number(0, R1, lbl);
            }
            return generate_number(left->value / right->value, R1, lbl);
        }
        if (right->isConst()) {
            if (right->value == 0) {
                Instruction::ZERO(out, R1, lbl);
                return out;
            }
            if (right->value == 1) {
                out = left->gen_ir(lbl, R0);
                Instruction::LOAD(out, R1, lbl);

                return out;
            } else if ((right->value & (right->value - 1)) == 0) {
                // right is power of 2
                out = left->gen_ir(lbl, R0);
                Instruction::LOAD(out, R1, lbl);
                long long val = right->value;
                while (val > 1) {
                    Instruction::SHR(out, R1, lbl);
                    val /= 2;
                }

                return out;
            }
        }

        /*DUM DUM DUM*/
        // if (! (symbols.declare_tmp("_SCALED")
        //     && symbols.declare_tmp("_REMAIN")
        //     && symbols.declare_tmp("_RESULT")
        //     && symbols.declare_tmp("_MULT"))) {
        //         cerr << "[Internal error] Div::gen_ir: Unable to declare temporary variable."
        //             << endl;
        //         exit(EXIT_FAILURE);
        // }
        auto a = left;
        auto b = right;
        Symbol scaled = symbols.get_var("_TMP0");
        Symbol remain = symbols.get_var("_TMP1");
        Symbol result = symbols.get_var("_TMP2");
        Symbol mult = symbols.get_var("_TMP3");

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

        return out;

    }

    /*TODO*/
    vector<Instruction*> Mod::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        if (left->isConst() && right->isConst()) {
            log_DEBUG("Modulo: 2 constants");
            return generate_number(left->value % left->value, R1, lbl);
        }
        if (right->isConst()) {
            log_DEBUG("Modulo: right constant");
            if (right->value == 0) {
                log_DEBUG("Modulo: right == 0");
                Instruction::ZERO(out, R1, lbl);
                return out;
            }
            if (right->value == 2) {
                log_DEBUG("Modulo: right == 2");
                out = load_value(left, R2, lbl); //left->gen_ir(lbl, R0);
                Instruction::ZERO(out, R1, lbl);
                Instruction::JODD(out, R2, *lbl+2, lbl);
                Instruction::JUMP(out, *lbl+2, lbl);
                Instruction::INC(out, R1, lbl);
                return out;
            }
        }

        // if (! (symbols.declare_tmp("_SCALED")
        //     && symbols.declare_tmp("_REMAIN")
        //     && symbols.declare_tmp("_RESULT")
        //     && symbols.declare_tmp("_MULT"))) {
        //         cerr << "[Internal error] Div::gen_ir: Unable to declare temporary variable."
        //             << endl;
        //         exit(EXIT_FAILURE);
        // }

        auto a = left;
        auto b = right;
        Symbol scaled = symbols.get_var("_TMP0");
        Symbol remain = symbols.get_var("_TMP1");
        Symbol result = symbols.get_var("_TMP2");
        Symbol mult = symbols.get_var("_TMP3");

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
        // symbols.undeclare("_SCALED");
        // symbols.undeclare("_REMAIN");
        // symbols.undeclare("_RESULT");
        // symbols.undeclare("_MULT");
        return out;
    }


    /*========================================
        CONDITIONS
    ========================================*/

    /* @done Set r1 to 1 if left == right, 0 otherwise*/
    vector<Instruction*> Eq::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }

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
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
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
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        insert_back(out, Minus(left, right, line).gen_ir(lbl, R2));
        Instruction::ZERO(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::INC(out, R1, lbl);
        return out;
    }

    /* @done set R1 to 1 if left < right, 0 otherwise */
    vector<Instruction*> Lt::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        insert_back(out, Minus(right, left, line).gen_ir(lbl, R2));
        Instruction::ZERO(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::INC(out, R1, lbl);
        return out;
    }

    /* @done set R1 to 1 if left <= right, 0 otherwise */
    vector<Instruction*> Leq::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        insert_back(out, Minus(left, right, line).gen_ir(lbl, R2));
        Instruction::ZERO(out, R1, lbl);
        Instruction::INC(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::DEC(out, R1, lbl);
        return out;

    }

    /* @done set R1 to 1 if left >= right, 0 otherwise */
    vector<Instruction*> Geq::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (! (check_init(left) && check_init(right))) {
            return out;
        }
        insert_back(out, Minus(right, left, line).gen_ir(lbl, R2));
        Instruction::ZERO(out, R1, lbl);
        Instruction::INC(out, R1, lbl);
        Instruction::JZERO(out, R2, *lbl+2, lbl);
        Instruction::DEC(out, R1, lbl);
        return out;
    }

    /*========================================
        VARIABLES
    ========================================*/
    /* @done reg = value || reg = &value */
    vector<Instruction*> Value::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        vector<Instruction*> out;
        if (value != -1) {
            out = generate_number(value, reg, lbl);
        } else {
            out = id->gen_ir(lbl, reg);
        }
        return out;
    }

    /* @done reg = &value*/
    vector<Instruction*> Var::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        Symbol var = symbols.get_var(name);
        if (var.line == Symbol::Undefined) {
            ostringstream os;
            os << "Undeclared variable " << name << ".";
            generator.report(os, line);
            return vector<Instruction*>();
        }
        if (var.isArray()) {
            ostringstream os;
            os << "Attempt to use an array " << name << " as a simple variable";
            generator.report(os, line);
            return vector<Instruction*>();
        }
        return generate_number(var.offset, reg, lbl);
    }

    /* @done reg = &arr[idx] */
    vector<Instruction*> ConstArray::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        Symbol arr = symbols.get_var(name);
        if (arr.line == Symbol::Undefined) {
            ostringstream os;
            os << "Undeclared variable " << name << ".";
            generator.report(os, line);
            return vector<Instruction*>();
        }
        if (!arr.isArray()) {
            ostringstream os;
            os << "Attempt to use a simple variable " << name << " as an array";
            generator.report(os, line);
            return vector<Instruction*>();
        }
        if (idx >= arr.size) {
            ostringstream os;
            os << "Attempt to access array " << name
                << " at index " << idx
                << "(size: " << arr.size << ").";
            generator.report(os, line);
            return vector<Instruction*>();
        }

        return generate_number(arr.offset+idx, reg, lbl);

    }

    /* @done reg = &arr[idx] */
    vector<Instruction*> VarArray::gen_ir(Imp::label *lbl, Imp::Reg reg) {
        ostringstream dbg;
        dbg << "VarArray: " << name << "[" << idx->name << "], reg" << reg;
        log_DEBUG(dbg.str());
        if (reg == R4) {
            cerr << Color::red << "Internal error: attempt to generate VarArray address in R4" << endl;
            exit(EXIT_FAILURE);
        }
        Symbol arr = symbols.get_var(name);
        vector<Instruction*> out;
        if (arr.line == Symbol::Undefined) {
            ostringstream os;
            os << "Undeclared variable " << name << ".";
            generator.report(os, line);
            return out;
        }
        if (!arr.isArray()) {
            ostringstream os;
            os << "Attempt to use a simple variable " << name << " as an array";
            generator.report(os, line);
            return out;
        }
        Imp::Reg tmp_reg = reg == R0 ? R4 : reg;
        out = generate_number(arr.offset, tmp_reg, lbl);
        insert_back(out, idx->gen_ir(lbl, R0));
        Instruction::ADD(out, tmp_reg, lbl);
        if (reg == R0) {
            Instruction::COPY(out, R4, lbl);
        }
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
                cerr << " Internal error: weird stuff happened while generating a constant" << endl;
                exit(EXIT_FAILURE);
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

    bool check_init(Id *id) {
        if (! symbols.is_initialized(id)) {
            ostringstream os;
            os << "Attempt to read uninitialized variable " << id->name;
            generator.report(os, id->line);
            return false;
        }
        return true;
    }

    bool check_init(Value *value) {
        if (value->isConst()) {
            return true;
        }
        return check_init(value->id);
    }
}