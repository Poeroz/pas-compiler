#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "args.h"
#include "argparser.h"

ArgParser::ArgParser() {}

ArgParser::~ArgParser() {}

bool ArgParser::parse_arguments(int argc, char *argv[]) const {
    bool version_flag = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] != '-')
            INPUT_FILE_NAME = argv[i];
        else
            if (arg == "-v") {
                std::cerr << "Pascal-S Complier version 1.0" << std::endl;
                version_flag = true;
            }
    }
    if (INPUT_FILE_NAME == "" && ! version_flag) {
        std::cerr << ERROR_OUT << "no input files" << std::endl;
        return false;
    }
    return true;
}

std::string ArgParser::parse_input_file() const {
    std::ifstream input_file(INPUT_FILE_NAME);
    if (! input_file.is_open()) {
        std::cerr << ERROR_OUT << "file does not exist" << std::endl;
        exit(1);
    }
    std::stringstream input_string_stream;
    input_string_stream << input_file.rdbuf();
    return input_string_stream.str();
}