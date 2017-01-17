#ifndef TYPES_H
#define TYPES_H 1

namespace Imp {
    enum Reg {
        NoReg = -1, R0, R1, R2, R3, R4
    };
    typedef long long arg;
    typedef long long label;
    typedef int64_t number;
}

#endif