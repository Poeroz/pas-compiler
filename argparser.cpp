#include <string>
#include <iostream>
#include "args.h"
#include "argparser.h"

void ArgParser::parse_arguments(int argc, char *argv[]) const {
    bool version_flag = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg[0] != '-')
            INPUT_FILE_NAME = arg[0];
        else
            if (arg == "-v") {
                std::cout << "XXX Pascal-S Complier version 0.0.1" << std::endl;
                version_flag = true;
            }
    }
    if (INPUT_FILE_NAME == "" && ! version_flag) {
        std::cout << "\033[31m\033[1merror:\033[0m no input files" << std::endl;
        exit(1);
    }
}