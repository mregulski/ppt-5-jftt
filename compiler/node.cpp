#include <sstream>
#include "Node.h"
#include "SymTable.h"
#include "Inter.h"
/* alphabetic order */

using namespace std;

extern SymTable symbols;

/*====================
    PROGRAM
====================*/

string Program::gen_ir() {
    ostringstream os;
    os << decl->gen_ir();
    os << code->gen_ir();
    os << endl;
    return os.str();
}

string Declarations::gen_ir() {
    // declare all symbols
    for(Id *id : ids) {
        symbols.declare(id);
    }
    return "";
}

void Declarations::declare(Id *id) {
    ids.push_back(id);
}

string Skip::gen_ir() {
    return "";
}

/*====================
    COMMANDS
====================*/

string Commands::gen_ir() {
    ostringstream os;
    for(Command *cmd : cmds) {
        os << cmd->gen_ir();
    }
    return os.str();
}

void Commands::add_command(Command *cmd) {
    cmds.push_back(cmd);
}

string Assign::gen_ir() {
    return "";
}
string If::gen_ir() {
    return "";
}
string While::gen_ir() {
    return "";
}
string For::gen_ir() {
    return "";
}
string Read::gen_ir() {
    return "";
}
string Write::gen_ir() {
    return "";
}


/*====================
    EXPRESSION
====================*/
string Expression::gen_ir() {return "";}

string Const::gen_ir() {
    return left->gen_ir();
};

string Plus::gen_ir() {
    ostringstream os;
    os << left->gen_ir() << " + " << right->gen_ir();
    return os.str();
}

string Minus::gen_ir() {
    ostringstream os;
    os << left->gen_ir() << " + " << right->gen_ir();
    return os.str();
}

string Mult::gen_ir() {
    ostringstream os;
    os << left->gen_ir() << " + " << right->gen_ir();
    return os.str();
}

string Div::gen_ir() {
    ostringstream os;
    os << left->gen_ir() << " + " << right->gen_ir();
    return os.str();
}

string Mod::gen_ir() {
    ostringstream os;
    os << left->gen_ir() << " + " << right->gen_ir();
    return os.str();
}

/*====================
    CONDITIONS
====================*/

string Eq::gen_ir() {
    return "";
}
string Neq::gen_ir() {
    return "";
}
string Gt::gen_ir() {
    return "";
}
string Lt::gen_ir() {
    return "";
}
string Leq::gen_ir() {
    return "";
}
string Geq::gen_ir() {
    return "";
}

/*====================
    VARIABLES
====================*/

string Var::gen_ir() {
    return name + "\n";
}

string ConstArray::gen_ir() {
    ostringstream os;
    os << name;
    os << " [" << idx << "]\n";
    return os.str();
}

string VarArray::gen_ir() {
    return "";
}

string Value::gen_ir() {
    ostringstream os;
    if (id != NULL) {
        os << id->gen_ir();
    } else {
        os << value;
    }
    return os.str();
}
