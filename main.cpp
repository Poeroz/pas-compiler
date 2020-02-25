#include <iostream>
#include <string>
#include "args.h"
#include "argparser.h"

std::string INPUT_FILE_NAME = "";

int main(int argc, char *argv[]) {
    ArgParser arg_parser;
    arg_parser.parse_arguments(argc, argv);
    return 0;
}