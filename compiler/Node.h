#ifndef NODE_H
#define NODE_H 1

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "types.h"
#include "inter.h"
namespace Imp{
    enum NodeType {
        FOR,
        INIT,
        BLOCK,
        DECL,
        NORMAL
    };

    class Node {
        NodeType type;
        public:
            int line;
            Node(int line, NodeType type) : line(line), type(type) {}
            Node(int line) : line(line), type(NORMAL) {}
            Node(NodeType type) : type(type) {}
            Node() {}
            NodeType getType() { return type; }
            virtual std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg) = 0;
            virtual std::ostream& dump(std::ostream& stream, int depth) = 0;
            std::string indent(int depth) {
                std::string ind(1*depth, '.');
                return ind;
            }

    };

    /*===============
        TOP-LEVEL
    ===============*/
    class Id : public Node {
        bool array=false;
        bool iter=false;
        public:
            enum Type { NORMAL, ARRAY, ITER };
            bool isArray() { return array; }
            bool isIter() { return iter; }
            std::string name="";
            Id(std::string name, Type type, int line) : name(name), Node(line) {
                switch(type) {
                    case ARRAY:
                        array = true;
                        break;
                    case ITER:
                        iter = true;
                        break;
                    default:
                        array=iter=false;
                }
            }
    };

    class Declarations: public Node {
        std::vector<Id*> ids;
        unsigned long for_counter = 0;
        public:
            void declare(Id *id);
            long long report_for() {
                return for_counter++;
            }
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::ostream& dump(std::ostream &stream, int depth) {
                stream << indent(depth) << "Declarations(" << ids.size() << ")\n";
                for (Id *id : ids) {
                    id->dump(stream, depth+1);
                }
                return stream;
            }
    };

    class Command : public Node {
        public:
            Command(int line, NodeType type) : Node(line, type) {}
            Command(int line) : Node(line) {}
            Command() {}
    };

    class Commands : public Node {
        std::vector<Command*> cmds;
        public:
            void add_command(Command *cmd);
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "Block(" << cmds.size() << ")\n";
                for (Command *cmd : cmds) {
                    cmd->dump(stream, depth+1);
                }
                return stream;
            }
    };

    class Program : public Node {
        public:
            Declarations *decl;
            Commands *code;
            Program(Declarations *decl, Commands *code) : decl(decl), code(code) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << "Program()\n";
                decl->dump(stream, depth+1);
                code->dump(stream, depth+1);
                return stream;
            }
    };

    class Value : public Node {
        public:
            number value;
            Id *id; // one or another

            Value(number val, int line) : value(val), id(NULL), Node(line) {}
            Value(Id* id, int line) : value(-1), id(id), Node(line) {}

            bool isConst() { return value != -1; }

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);

            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth);
                if (value != -1) {
                    return stream << "Value(" << value << ")\n";
                }
                stream << "Value(id)\n";
                id->dump(stream, depth+1);
                return stream;
            }
    };


    class Expression : public Node {
        public:
            Value *left;
            Value *right; // might be empty

            Expression(Value *left, Value *right, int line) : left(left), right(right), Node(line) {}

            virtual std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg)=0;
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "Expression(" << op() << ")\n";
                left->dump(stream, depth+1);
                right->dump(stream, depth+1);
                return stream;
            }

            virtual std::string op() = 0;

    };

    class Condition : public Node {
        public:
            Value *left;
            Value *right;

            Condition(Value *left, Value *right, int line) : left(left), right(right), Node(line) {}

            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "Condition(" << op() << ")\n";
                left->dump(stream, depth+1);
                right->dump(stream, depth+1);
                return stream;
            }

            virtual std::string op() = 0;
    };

    /*===============
        COMMANDS
    ===============*/

    class Assign : public Command {
        public:
            Id *id;
            Expression *expr;

            Assign(Id *id, Expression *expr, int line) : id(id), expr(expr), Command(line, INIT) {}

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "Assign()\n";
                id->dump(stream, depth+1);
                expr->dump(stream, depth+1);
                return stream;
            }
    };

    class If : public Command {
        public:
            Condition *cond;
            Commands *do_then;
            Commands *do_else;

            If(Condition *cond, Commands *do_then, Commands *do_else, int line) :
                cond(cond), do_then(do_then), do_else(do_else), Command(line) {}

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "If()\n";
                cond->dump(stream, depth+1);
                do_then->dump(stream, depth+1);
                do_else->dump(stream, depth+1);
                return stream;
            }
    };

    class While : public Command {
        public:
            Condition *cond;
            Commands *body;

            While(Condition *cond, Commands *body, int line)
                : cond(cond), body(body), Command(line) {}

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "While()\n";
                cond->dump(stream, depth+1);
                body->dump(stream, depth+1);
                return stream;
            }
    };

    class For : public Command {
        private:
            long long id;
        public:
            Id *iterator;
            Value *from;
            Value *to;
            Commands *body;
            bool isDownTo;

            For(Id *iter, Value *from, Value *to, Commands *body, bool isDownTo, int line, long long id)
                : iterator(iter), from(from), to(to),
                    body(body), isDownTo(isDownTo), Command(line), id(id) {}

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "For(" << (isDownTo ? "DownTo" : "UpTo") << ")\n";
                iterator->dump(stream, depth+1);
                from->dump(stream, depth+1);
                to->dump(stream, depth+1);
                body->dump(stream, depth+1);
                return stream;
            }
    };

    class Read : public Command {
        public:
            Id *id;

            Read(Id *id, int line) : id(id), Command(line) {}

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream  << indent(depth) << "Read()\n";
                id->dump(stream, depth+1);
                return stream;
            }
    };

    class Write : public Command {
        public:
            Value *val;

            Write(Value *val, int line): val(val), Command(line) {}

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "Write()\n";
                val->dump(stream, depth+1);
                return stream;
            }
    };

    class Skip : public Command {
        public:

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                return stream << indent(depth) << "Skip()\n";
            }
    };

    /*===============
        EXPRESSIONS
    ===============*/

    class Const : public Expression {
        public:
            Const(Value *val, int line) : Expression(val, NULL, line) {}

            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Const"; }
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                stream << indent(depth) << "Const(";
                if (left->value != -1) {
                    return stream << left->value << ")\n";
                }
                stream << ")\n";
                left->dump(stream, depth+1);
                return stream;
            }

    };

    class Plus : public Expression {
        public:
            Plus(Value *left, Value *right, int line) : Expression(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Plus"; }
    };

    class Minus : public Expression {
        public:
            Minus(Value *left, Value *right, int line) : Expression(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Minus"; }
    };

    class Mult : public Expression {
        public:
            Mult(Value *left, Value *right, int line) : Expression(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Mult"; }
    };

    class Div : public Expression {
        public:
            Div(Value *left, Value *right, int line) : Expression(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Div"; }
    };

    class Mod : public Expression {
        public:
            Mod(Value *left, Value *right, int line) : Expression(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Mod"; }
    };

    /*===============
        CONDITIONS
    ===============*/

    class Eq : public Condition {
        public:
            Eq(Value *left, Value *right, int line) : Condition(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Eq"; }
    };

    class Neq : public Condition {
        public:
            Neq(Value *left, Value *right, int line) : Condition(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Neq"; }
    };

    class Gt : public Condition {
        public:
            Gt(Value *left, Value *right, int line) : Condition(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Gt"; }
    };

    class Lt : public Condition {
        public:
            Lt(Value *left, Value *right, int line) : Condition(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Lt"; }
    };

    class Leq : public Condition {
        public:
            Leq(Value *left, Value *right, int line) : Condition(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Leq"; }
    };

    class Geq : public Condition {
        public:
            Geq(Value *left, Value *right, int line) : Condition(left, right, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            std::string op() { return "Geq"; }
    };

    /*===============
        VARIABLES
    ===============*/

    // Variable declaration or access
    class Var : public Id {
        public:
            Var(std::string name, int line) : Id(name, NORMAL, line) {}
            Var(std::string name, Type type, int line) : Id(name, type, line) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                return stream << indent(depth) << "Var(" << name << ")\n";
            }
    };

    // Array declaration or constant-index access: a[idx]
    // In declarations, idx is interpreted as array's size
    class ConstArray : public Id {
        public:
            number idx;
            ConstArray(std::string name, number idx, int line) : Id(name, ARRAY, line), idx(idx) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                return stream << indent(depth) << "ConstArray(" << name << ", " << idx << ")\n";
            }
    };

    // Varaible-index array access
    class VarArray : public Id {
        public:
            Var* idx;
            VarArray(std::string name, Var *idx, int line) : Id(name, ARRAY, line), idx(idx) {}
            std::vector<Instruction*> gen_ir(Imp::label *cur_label, Imp::Reg reg);
            virtual std::ostream& dump(std::ostream& stream, int depth) {
                return stream << indent(depth) << "VarArray(" << name << ", " << idx->name << ")\n";
            }
    };
}
#endif