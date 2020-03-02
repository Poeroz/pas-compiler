#include <iostream>
#include <string>
#include <unordered_map>
#include "args.h"
#include "argparser.h"
#include "lexer.h"


int main(int argc, char *argv[]) {
    ArgParser arg_parser;
    if (! arg_parser.parse_arguments(argc, argv))
        return -1;
    Lexer lexer;
    if (! lexer.analyze(arg_parser.parse_input_file())) {
        std::cerr << ERROR_OUT << "lexical error" << std::endl;
        return -1;
    }
    return 0;
}