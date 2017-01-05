#include <sstream>
#include "Node.h"
#include "SymTable.h"
#include "Inter.h"
#include "CodeGen.h"
/* alphabetic order */

using namespace std;
using namespace Imp;
extern SymTable symbols;


vector<Instruction*> generate_number(long long number, Imp::Reg target_reg, Imp::label *cur_label);
void inc(label *label) { (*label)++; }
/*====================
    PROGRAM
====================*/

vector<Instruction*> Program::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    decl->gen_ir(cur_label, R0);
    vector<Instruction*> output = code->gen_ir(cur_label, R1);
    output.push_back(Instruction::HALT(cur_label));
    return output;
}

vector<Instruction*> Declarations::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    // declare all symbols
    for(Id *id : ids) {
        symbols.declare(id);
    }
    return vector<Instruction*>();
}

void Declarations::declare(Id *id) {
    ids.push_back(id);
}

vector<Instruction*> Skip::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    out.push_back(Instruction::JUMP(1+*cur_label, cur_label));
}

/*====================
    COMMANDS
====================*/

vector<Instruction*> Commands::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> output;
    for(Command *cmd : cmds) {
        vector<Instruction*> cmd_out = cmd->gen_ir(cur_label, R1);
        output.insert(output.end(), cmd_out.begin(), cmd_out.end());
    }
    return output;
}

void Commands::add_command(Command *cmd) {
    cmds.push_back(cmd);
}

/* Assign a new value to a variable */
vector<Instruction*> Assign::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out = expr->gen_ir(cur_label, R1);
    if(expr->op() == "Const" && !expr->left->isConst()) {
        out.push_back(Instruction::LOAD(reg, cur_label));

    }
    vector<Instruction*> location = id->gen_ir(cur_label, R0);
    out.insert(out.end(), location.begin(), location.end());
    out.push_back(Instruction::STORE(reg, cur_label));

    return out;
}

/* Generate a branch */
vector<Instruction*> If::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out = cond->gen_ir(cur_label, R1);
    // current label = condition's END; r1 == condition.
    // ifZero r1 goto else
    // goto then
    Instruction *jump_else = Instruction::JZERO(R1, Instruction::NoArg, cur_label); // fill in the target later
    out.push_back(jump_else);

    // do_then
    auto branch_then = do_then->gen_ir(cur_label, R1);
    out.insert(out.end(), branch_then.begin(), branch_then.end());
    Instruction *jump_end = Instruction::JUMP(Instruction::NoArg, cur_label); // fill in the target later
    out.push_back(jump_end);

    // do_else
    auto branch_else = do_else->gen_ir(cur_label, R1);
    out.insert(out.end(), branch_else.begin(), branch_else.end());
    // now that we know jump_else's target - update it
    jump_else->arg2 = branch_else[0]->label;

    jump_end->arg2 = *cur_label;

    return out;
}

/*TODO*/
vector<Instruction*> While::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    // create jump without target, save reference
    // create body with jump to beginning
    // knowing number of instructions in the body, update the first jump
    return vector<Instruction*>();
}

/*TODO*/
vector<Instruction*> For::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}

/* Read input into a variable */
vector<Instruction*> Read::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    vector<Instruction*> id_addr = id->gen_ir(cur_label, R0);
    out.insert(out.end(), id_addr.begin(), id_addr.end());
    out.push_back(Instruction::GET(reg, cur_label));
    out.push_back(Instruction::STORE(reg, cur_label));
    return out;
}

/* Write value to the output */
vector<Instruction*> Write::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    vector<Instruction*> value_ref = val->gen_ir(cur_label, reg);
    out.insert(out.end(), value_ref.begin(), value_ref.end());
    if(!val->isConst()) { out.push_back(Instruction::LOAD(reg, cur_label)); }
    out.push_back(Instruction::PUT(reg, cur_label));
    return out;
}


/*====================
    EXPRESSION
====================*/

/* put value in r1 */
vector<Instruction*> Const::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return left->gen_ir(cur_label, reg);
};

/* Calculate left+right, result in r1. */
vector<Instruction*> Plus::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    if (left->isConst() && right->isConst()) {
        // 2 compile-time constants: calculate value immediately
        long long value = left->value + right->value;
        out = generate_number(value < 0 ? 0 : value, reg, cur_label);
    } else if (left->isConst() || right->isConst()) {
        // one constant
        out = left->gen_ir(cur_label, reg);
        auto arg2 = right->gen_ir(cur_label, reg);
        out.insert(out.end(), arg2.begin(), arg2.end());
        out.push_back(Instruction::ADD(reg, cur_label));

    } else {
        // no constants
        out = left->gen_ir(cur_label, reg);
        out.push_back(Instruction::LOAD(reg, cur_label));

        auto arg2 = right->gen_ir(cur_label, reg);
        out.insert(out.end(), arg2.begin(), arg2.end());
        out.push_back(Instruction::ADD(reg, cur_label));

    }
    return out;
}

/* Calculate left - right, result in reg. */
vector<Instruction*> Minus::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    if (left->isConst() && right->isConst()) {
        // 2 compile-time constants: calculate value immediately
        long long value = left->value - right->value;
        out = generate_number(value < 0 ? 0 : value, reg, cur_label);
    } else if (left->isConst()) {
        // one constant
        out = left->gen_ir(cur_label, reg);
        auto arg2 = right->gen_ir(cur_label, R0);
        out.insert(out.end(), arg2.begin(), arg2.end());
        out.push_back(Instruction::SUB(reg, cur_label));
    } else if (right->isConst()) {
        // create temp variable for the constant
        long long tmp = symbols.get_tmp();
        out = generate_number(tmp, R0, cur_label);

        //
        auto constant = right->gen_ir(cur_label, reg);
        out.insert(out.end(), constant.begin(), constant.end());
        out.push_back(Instruction::STORE(reg, cur_label));

        // reg <- left
        auto var = left->gen_ir(cur_label, R0);
        out.insert(out.end(), var.begin(), var.end());
        out.push_back(Instruction::LOAD(reg, cur_label));

        auto tmp_addr = generate_number(tmp, R0, cur_label);
        out.insert(out.end(), tmp_addr.begin(), tmp_addr.end());

        out.push_back(Instruction::SUB(reg, cur_label));

    } else {
        // no constants
        out = left->gen_ir(cur_label, reg);
        out.push_back(Instruction::LOAD(reg, cur_label));

        auto arg2 = right->gen_ir(cur_label, reg);
        out.insert(out.end(), arg2.begin(), arg2.end());
        out.push_back(Instruction::SUB(reg, cur_label));

    }
    return out;
}

/* TODO Calculate left * right, result in r1 */
vector<Instruction*> Mult::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    if (left->isConst() && right->isConst()) {
        // 2 compile-time constants: calculate value immediately
        long long value = left->value * right->value;
        out = generate_number(value < 0 ? 0 : value, reg, cur_label);
    } else if (left->isConst() || right->isConst()) {
        Value *constant = left->isConst() ? left : right;
        Value *ref = left->isConst() ? right : left;
        if (constant->value == 0) { out = generate_number(0, reg, cur_label); }
        else {
            // one constant
            // SHL then ADD or SUB, similar approach as in generate_number
            long long multiplier = 1;
            long long target = constant->value;
            out = ref->gen_ir(cur_label, reg);
            out.push_back(Instruction::LOAD(reg, cur_label));

            while (multiplier * 2 < target) {
                out.push_back(Instruction::SHL(reg, cur_label));
                multiplier *= 2;
            }
            if (multiplier * 2 - target < target - multiplier) {
                // go down from multiplier * 2 to target
                out.push_back(Instruction::SHL(reg, cur_label));

                multiplier *= 2;
                while (multiplier > target) {
                    out.push_back(Instruction::SUB(reg, cur_label));
                    multiplier--;
                }
            } else {
                // go up
                while (multiplier < target) {
                    out.push_back(Instruction::ADD(reg, cur_label));
                    multiplier++;
                }
            }
        }

    } else {
        // no constants - requires jumps
    }
    return out;
}

/*TODO*/
vector<Instruction*> Div::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}

/*TODO*/
vector<Instruction*> Mod::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}

/*====================
    CONDITIONS
====================*/

/* Set r1 to 1 if left == right, 0 otherwise*/
vector<Instruction*> Eq::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    // r1 <- 0
    out.push_back(Instruction::ZERO(R1, cur_label));

    // r2 <- left - right
    auto sub_lr = Minus(left, right).gen_ir(cur_label, R2);
    out.insert(out.end(), sub_lr.begin(), sub_lr.end());

    // jump START (target will be set after second subtraction's length is known)
    // Instruction jump_start = Instruction::JUMP(Instruction::NoArg, cur_label);
    out.push_back(Instruction::JUMP(Instruction::NoArg, cur_label));
    Instruction *jump_start = out.back();
    // r2 <- right - left
    auto sub_rl = Minus(right, left).gen_ir(cur_label, R2);
    out.insert(out.end(), sub_rl.begin(), sub_rl.end());
    label sub2 = sub_rl[0]->label;

    // jump MAYBE
    out.push_back(Instruction::JUMP(sub_rl.back()->label + 4, cur_label)); // TODO: check for off-by-one

    // ifZero r2 jump SUB2
    Instruction *cmp_1 = Instruction::JZERO(R2, sub2, cur_label);
    // START is now known; update jump_start
    jump_start->arg2 = cmp_1->label;
    out.push_back(cmp_1);

    // jump END
    out.push_back(Instruction::JUMP(4+*cur_label, cur_label)); // TODO: check if it's not off-by-one

    // ifZero r2 jump TRUE
    out.push_back(Instruction::JZERO(R2, 2 + *cur_label, cur_label));

    // jump END
    out.push_back(Instruction::JUMP(2 + *cur_label, cur_label));

    // TRUE: inc 1
    out.push_back(Instruction::INC(R1, cur_label));

    return out;
}

/* Set r1 to 1 if left != right, 0 otherwise */
vector<Instruction*> Neq::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    // r1 <- 0
    out.push_back(Instruction::ZERO(R1, cur_label));
    out.push_back(Instruction::INC(R1, cur_label));

    // r2 <- left - right
    auto sub_lr = Minus(left, right).gen_ir(cur_label, R2);
    out.insert(out.end(), sub_lr.begin(), sub_lr.end());

    // jump START (target will be set after second subtraction's length is known)
    // Instruction jump_start = Instruction::JUMP(Instruction::NoArg, cur_label);
    out.push_back(Instruction::JUMP(Instruction::NoArg, cur_label));
    Instruction *jump_start = out.back();
    // r2 <- right - left
    auto sub_rl = Minus(right, left).gen_ir(cur_label, R2);
    out.insert(out.end(), sub_rl.begin(), sub_rl.end());
    label sub2 = sub_rl[0]->label;

    // jump MAYBE
    out.push_back(Instruction::JUMP(sub_rl.back()->label + 4, cur_label)); // TODO: check for off-by-one

    // ifZero r2 jump SUB2
    Instruction *cmp_1 = Instruction::JZERO(R2, sub2, cur_label);
    // START is now known; update jump_start
    jump_start->arg2 = cmp_1->label;
    out.push_back(cmp_1);

    // jump END
    out.push_back(Instruction::JUMP(4+*cur_label, cur_label)); // TODO: check if it's not off-by-one

    // ifZero r2 jump TRUE
    out.push_back(Instruction::JZERO(R2, 2 + *cur_label, cur_label));

    // jump END
    out.push_back(Instruction::JUMP(2 + *cur_label, cur_label));

    // TRUE: inc 1
    out.push_back(Instruction::DEC(R1, cur_label));

    return out;
}


vector<Instruction*> Gt::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}


vector<Instruction*> Lt::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}


vector<Instruction*> Leq::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}


vector<Instruction*> Geq::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}

/*====================
    VARIABLES
====================*/
vector<Instruction*> Value::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    vector<Instruction*> out;
    if (value != -1) {
        out = generate_number(value, reg, cur_label);
    } else {
        out = id->gen_ir(cur_label, R0);
    }
    return out;
}

vector<Instruction*> Var::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    Symbol var = symbols.get_var(name);
    return generate_number(var.offset, reg, cur_label);
}

vector<Instruction*> ConstArray::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}

vector<Instruction*> VarArray::gen_ir(Imp::label *cur_label, Imp::Reg reg) {
    return vector<Instruction*>();
}

vector<Instruction*> generate_number(long long number, Imp::Reg target_reg, Imp::label *cur_label) {
    vector<Instruction*> out;
    out.push_back(Instruction::ZERO(target_reg, cur_label));

    long long counter = 0;
    if (counter == number) return out;
    out.push_back(Instruction::INC(target_reg, cur_label));

    counter = 1;
    if (counter == number) return out;

    while (counter * 2 < number) {
        out.push_back(Instruction::SHL(target_reg, cur_label));

        counter *= 2;
    }
    if (counter * 2 - number < number-counter) {
        // go down from counter*2
        out.push_back(Instruction::SHL(target_reg, cur_label));

        counter *= 2;
        while (counter > number) {
            out.push_back(Instruction::DEC(target_reg, cur_label));

            counter--;
        }
    } else {
        // go up from counter
        while (counter < number) {
            out.push_back(Instruction::INC(target_reg, cur_label));

            counter++;
        }
    }
    return out;


}