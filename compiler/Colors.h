#ifndef COLORS_H
#define COLORS_H 1

/*
https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
*/
#include <ostream>
namespace Color {
    enum Code {
        FG_RED      = 31,
        FG_GREEN    = 32,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
    class Modifier {
        Code code;
        public:
            Modifier(Code pCode) : code(pCode) {}
            friend std::ostream&
            operator<<(std::ostream& os, const Modifier& mod) {
                return os << "\033[" << mod.code << "m";
            }
    };

    const Modifier red(Color::FG_RED);
    const Modifier blue(Color::FG_BLUE);
    const Modifier green(Color::FG_GREEN);
    const Modifier def(Color::FG_DEFAULT);
}

#endif