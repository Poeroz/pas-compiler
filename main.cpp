#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "args.h"
#include "argparser.h"
#include "lexer.h"
#include "parser.h"


int main(int argc, char *argv[]) {
    ArgParser arg_parser;
    if (! arg_parser.parse_arguments(argc, argv))
        return -1;
    Lexer lexer;
    input_code = arg_parser.parse_input_file();
    if (! lexer.analyze(input_code)) {
        std::cerr << ERROR_OUT << "compilation error" << std::endl;
        return -1;
    }
    if (USE_LL1_PARSER);
    else {
        Parser parser;
        if (! parser.parse(lexer.get_token_list())) {
            std::cerr << ERROR_OUT << "compilation error" << std::endl;
            return -1;
        }
        std::string output_file_name = INPUT_FILE_NAME.substr(0, INPUT_FILE_NAME.find_last_of('.')) + ".cpp";
        std::ofstream output_file(output_file_name);
        output_file << parser.get_result();
    }
    return 0;
}