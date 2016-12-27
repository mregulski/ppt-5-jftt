#include <iostream>
#define YYDEBUG 0
extern int yyparse();
extern int yydebug;

int main() {
    yydebug = YYDEBUG;
    int result = yyparse();
    if (result) {
        std::cout << "Invalid input\n";
    } else {
        std::cout << "Valid input\n";
    }
    return result;
}