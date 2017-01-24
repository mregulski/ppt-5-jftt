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
        FG_YELLOW   = 33,
        FG_BLUE     = 34,
        FG_MAGENTA  = 35,
        FG_CYAN     = 36,
        FG_WHITE    = 37,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_YELLOW   = 43,
        BG_BLUE     = 44,
        BG_MAGENTA  = 45,
        BG_CYAN     = 46,
        BG_WHITE    = 47,
        BG_DEFAULT  = 49,
        RESET       = 0,
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
    const Modifier green(Color::FG_GREEN);
    const Modifier yellow(Color::FG_YELLOW);
    const Modifier blue(Color::FG_BLUE);
    const Modifier magenta(Color::FG_MAGENTA);
    const Modifier cyan(Color::FG_CYAN);

    const Modifier bred(Color::BG_RED);
    const Modifier bgreen(Color::BG_GREEN);
    const Modifier byellow(Color::BG_YELLOW);
    const Modifier bblue(Color::BG_BLUE);
    const Modifier bmagenta(Color::BG_MAGENTA);
    const Modifier bcyan(Color::BG_CYAN);
    const Modifier bwhite(Color::BG_WHITE);

    const Modifier def(Color::RESET);
}

#endif