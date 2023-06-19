#include "tools/ArgParser.h"


using namespace TP;

int main(int argc, char* argv[]) {
	ArgParser parser(argc, argv);
    auto int_test = parser.parse<int>("int", 'i', 62, "Pass an integer");
    auto char_test = parser.parse<char>("char", 'c', "Pass a character");
    auto str_test = parser.parse<std::string>('s', "beep bop I'm a default", "Pass a string");
    auto indicator = parser.parse<void>('i', "Indicator");
    parser.enableHelp();

    LOG("has int? " << int_test.has());
    if (int_test.has())
        LOG("int: " << int_test.get());
    
    LOG("has char? " << char_test.has());
    if (char_test.has())
        LOG("char: " << char_test.get());
    
    LOG("str: " << str_test.get());
    return 0;
}
