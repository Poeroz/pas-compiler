#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>


class Lexer {
public:
    Lexer();
    ~Lexer();
    bool analyze(std::string input_code);

private:
    char next_char() const;
    std::string get_line(int pos) const;
    void go_forw();
    void go_cur();
    void go_back_forw();
    bool get_eof() const;
    bool forw_get_eof() const;
    void output_cur_error(std::string error_info) const;
    std::string match_pattern(std::string pattern);
    bool string_analyze();
    bool number_analyze();
    bool word_analyze();
    const std::string integer_pattern = "\\d+|\\$[\\da-fA-F]+|&[0-7]+|%[01]+";
    const std::string float_pattern = "\\d+[eE][\\+\\-]?\\d+|\\d+\\.\\d+([eE][\\+\\-]?\\d+)?";
    std::string buffer;
    std::vector<Token> token_list;
    int cur_ptr, forw_ptr, char_cnt;
    int cur_line, cur_col, forw_line, forw_col;
};

#endif