#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <string>


class ArgParser {
public:
    ArgParser();
    ~ArgParser();
    bool parse_arguments(int argc, char *argv[]) const;
    std::string parse_input_file() const;
};

#endif