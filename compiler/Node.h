#ifndef NODE_H
#define NODE_H 1

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
class Node {
    public:
        int line;
        virtual std::string gen_ir() = 0;
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
    public:
        int lineno;
        std::string name;
        Id(std::string name, int lineno) : name(name), lineno(lineno) {}
};

class Declarations: public Node {
    std::vector<Id*> ids;
    public:
        void declare(Id *id);
        std::string gen_ir();
        std::ostream& dump(std::ostream &stream, int depth) {
            stream << indent(depth) << "Declarations(" << ids.size() << ")\n";
            for (Id *id : ids) {
                id->dump(stream, depth+1);
            }
            return stream;
        }
};

class Command : public Node {};

class Commands : public Node {
    std::vector<Command*> cmds;
    public:
        void add_command(Command *cmd);
        std::string gen_ir();
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
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth){
        stream << "Program()\n";
        decl->dump(stream, depth+1);
        code->dump(stream, depth+1);
        return stream << "\n";
        }
};

class Value : public Node {
    public:
        long long value;
        Id *id; // one or another

        Value(long long val) : value(val), id(NULL) {}
        Value(Id* id) : value(-1), id(id) {}
        std::string gen_ir();
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

        Expression(Value *left, Value *right) : left(left), right(right) {}

        std::string gen_ir();
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

        Condition(Value *left, Value *right) : left(left), right(right) {}

        virtual std::ostream& dump(std::ostream& stream, int depth) {
            return stream << indent(depth) << "Condition(" << op() << ")\n";
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
        Assign(Id *id, Expression *expr) : id(id), expr(expr) {}
        std::string gen_ir();
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
        If(Condition *cond, Commands *do_then, Commands *do_else) :
            cond(cond), do_then(do_then), do_else(do_else) {}
        std::string gen_ir();
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
        While(Condition *cond, Commands *body) : cond(cond), body(body) {}
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth) {
            stream << indent(depth) << "While()\n";
            cond->dump(stream, depth+1);
            body->dump(stream, depth+1);
            return stream;
         }
};

class For : public Command {
    public:
        Id *iterator;
        Value *from;
        Value *to;
        Commands *body;
        bool isDownTo;
        For(Id *iter, Value *from, Value *to, Commands *body, bool isDownTo) : iterator(iter), from(from), to(to), body(body), isDownTo(isDownTo) {}
        std::string gen_ir();
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
        Read(Id *id) : id(id) {}
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth) {
            stream  << indent(depth) << "Read()\n";
            id->dump(stream, depth+1);
            return stream;
        }
};

class Write : public Command {
    public:
        Value *val;
        Write(Value *val): val(val) {}
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth) {
            stream << indent(depth) << "Write()\n";
            val->dump(stream, depth+1);
            return stream;
        }
};

class Skip : public Command {
    public:
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth) {
            return stream << indent(depth) << "Skip()\n";
        }
};

/*===============
    EXPRESSIONS
===============*/

class Const : public Expression {
    public:
        Const(Value *val) : Expression(val, NULL) {}
        std::string gen_ir();
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
        Plus(Value *left, Value *right) : Expression(left, right) {}
        std::string gen_ir();
        std::string op() { return "Plus"; }
};

class Minus : public Expression {
    public:
        Minus(Value *left, Value *right) : Expression(left, right) {}
        std::string gen_ir();
        std::string op() { return "Minus"; }
};

class Mult : public Expression {
    public:
        Mult(Value *left, Value *right) : Expression(left, right) {}
        std::string gen_ir();
        std::string op() { return "Mult"; }
};

class Div : public Expression {
    public:
        Div(Value *left, Value *right) : Expression(left, right) {}
        std::string gen_ir();
        std::string op() { return "Div"; }
};

class Mod : public Expression {
    public:
        Mod(Value *left, Value *right) : Expression(left, right) {}
        std::string gen_ir();
        std::string op() { return "Mod"; }
};

/*===============
    CONDITIONS
===============*/

class Eq : public Condition {
    public:
        Eq(Value *left, Value *right) : Condition(left, right) {}
        std::string gen_ir();
         std::string op() { return "Eq"; }
};

class Neq : public Condition {
    public:
        Neq(Value *left, Value *right) : Condition(left, right) {}
        std::string gen_ir();
         std::string op() { return "Neq"; }
};

class Gt : public Condition {
    public:
        Gt(Value *left, Value *right) : Condition(left, right) {}
        std::string gen_ir();
         std::string op() { return "Gt"; }
};

class Lt : public Condition {
    public:
        Lt(Value *left, Value *right) : Condition(left, right) {}
        std::string gen_ir();
         std::string op() { return "Lt"; }
};

class Leq : public Condition {
    public:
        Leq(Value *left, Value *right) : Condition(left, right) {}
        std::string gen_ir();
        std::string op() { return "Leq"; }
};

class Geq : public Condition {
    public:
        Geq(Value *left, Value *right) : Condition(left, right) {}
        std::string gen_ir();
        std::string op() { return "Geq"; }
};

/*===============
    VARIABLES
===============*/

// Variable declaration or access
class Var : public Id {
    public:
        Var(std::string name, int lineno) : Id(name, lineno) {}
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth) {
            return stream << indent(depth) << "Var(" << name << ")\n";
        }
};

// Array declaration or constant-index access: a[idx]
// In declarations, idx is interpreted as array's size
class ConstArray : public Id {
    public:
        long long idx;
        ConstArray(std::string name, long long idx, int lineno) : Id(name, lineno), idx(idx) {}
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth) {
            return stream << indent(depth) << "ConstArray(" << name << ", " << idx << ")\n";
        }
};

// Varaible-index array access
class VarArray : public Id {
    public:
        Var* idx;
        VarArray(std::string name, Var *idx, int lineno) : Id(name, lineno), idx(idx) {}
        std::string gen_ir();
        virtual std::ostream& dump(std::ostream& stream, int depth) {
            return stream << indent(depth) << "VarArray(" << name << ", " << idx->name << ")\n";
        }
};

#endif