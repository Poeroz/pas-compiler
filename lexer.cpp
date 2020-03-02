#include <iostream>
#include <cctype>
#include <regex>
#include "args.h"
#include "lexer.h"


Lexer::Lexer() {
    char_cnt = 0;
    cur_ptr = forw_ptr = 0;
    cur_line = cur_col = forw_line = forw_col = 1;
}

Lexer::~Lexer() {}

char Lexer::next_char() const {
    return forw_ptr == char_cnt ? '\0' : buffer[forw_ptr];
}

std::string Lexer::get_line(int pos) const {
    int st = pos, ed = pos;
    for (; st >= 0 && buffer[st] != '\n'; st--);
    st++;
    for (; ed < char_cnt && buffer[ed] != '\n'; ed++);
    return buffer.substr(st, ed - st);
}

void Lexer::go_forw() {
    if (forw_ptr < char_cnt) {
        forw_ptr++;
        if (next_char() == '\n') {
            forw_line++;
            forw_col = 0;
        }
        else
            forw_col++;
    }
}

void Lexer::go_cur() {
    cur_ptr = forw_ptr;
    cur_line = forw_line;
    cur_col = forw_col;
}

void Lexer::go_back_forw() {
    forw_ptr = cur_ptr;
    forw_line = cur_line;
    forw_col = cur_col;
}

bool Lexer::get_eof() const {
    return cur_ptr == char_cnt;
}

bool Lexer::forw_get_eof() const {
    return forw_ptr == char_cnt;
}

void Lexer::output_cur_error(std::string error_info) const {
    std::cerr << cur_line << ":" << cur_col << ": " << ERROR_OUT << error_info << std::endl;
    std::cerr << get_line(cur_ptr) << std::endl;
    for (int i = 0; i < cur_col - 1; i++)
        std::cerr << ' ';
    std::cerr << ERROR_POINTER << std::endl;
}

std::string Lexer::match_pattern(std::string pattern) {
    pattern = "(" + pattern + ")[\\s\\S]*";
    std::smatch result;
    std::string buffer_substr = buffer.substr(forw_ptr);
    bool flag = std::regex_match(buffer_substr, result, std::regex(pattern));
    if (flag) {
        for (int i = 0; i < result[1].length(); i++)
            go_forw();
        return result[1];
    }
    else
        return std::string("");
}   

bool Lexer::string_analyze() {
    std::string s;
    bool end_flag = false, error_flag = false;
    int quoted_ptr = 0, quoted_line = 0, quoted_col = 0;
    for (bool quoted_flag = false; ! forw_get_eof();)
        if (! quoted_flag)
            if (next_char() == '\'') {
                s += next_char();
                quoted_ptr = forw_ptr;
                quoted_line = forw_line;
                quoted_col = forw_col;
                go_forw();
                quoted_flag = true;
            }
            else
                if (next_char() == '#') {
                    s += next_char();
                    go_forw();
                    std::string temp = match_pattern(integer_pattern);
                    if (temp == "") {
                        error_flag = true;
                        output_cur_error("illegal char constant");
                        break;
                    }
                    else
                        s += temp;
                }
                else {
                    end_flag = true;
                    break;
                }
        else
            if (next_char() == '\'') {
                s += next_char();
                go_forw();
                if (next_char() == '\'') {
                    s += next_char();
                    go_forw();
                }
                else
                    quoted_flag = false;
            }
            else
                if (next_char() == '\n' || next_char() == '\r') {
                    error_flag = true;
                    output_cur_error("string exceeds line");
                    break;
                }
                else {
                    s += next_char();
                    go_forw();
                }
    if (! error_flag && ! end_flag) {
        cur_ptr = quoted_ptr;
        cur_line = quoted_line;
        cur_col = quoted_col;
        output_cur_error("missing terminating ''' character");
    }
    if (end_flag)
        token_list.push_back(Token(5, 0, cur_line, cur_col, s));
    go_cur();
    return end_flag;
}

bool Lexer::number_analyze() {
    std::string number = match_pattern(float_pattern);
    if (number != "") {
        token_list.push_back(Token(4, 0, cur_line, cur_col, number));
        go_cur();
        return true;
    }
    number = match_pattern(integer_pattern);
    if (number != "") {
        token_list.push_back(Token(3, 0, cur_line, cur_col, number));
        go_cur();
        return true;
    }
    output_cur_error("invalid integer experssion");
    go_forw();
    go_cur();
    return false;
}

bool Lexer::word_analyze() {
    std::string word = "";
    word += tolower(next_char());
    go_forw();
    for (; isalnum(next_char()) || next_char() == '_'; go_forw())
        word += tolower(next_char());
    if (word.length() > 127) {
        output_cur_error("identifier length must less than 128");
        go_cur();
        return false;
    }         
    for (int i = 0; i < num_keywords; i++)
        if (word == keywords[i]) {
            token_list.push_back(Token(0, i, cur_line, cur_col));
            go_cur();
            return true;
        }
    for (int i = 0; i < num_data_types; i++)
        if (word == data_types[i]) {
            token_list.push_back(Token(1, i, cur_line, cur_col));
            go_cur();
            return true;
        }
    for (int i = 0; i < num_rtl_functions; i++)
        if (word == rtl_functions[i]) {
            token_list.push_back(Token(8, i, cur_line, cur_col));
            go_cur();
            return true;
        }
    if (! token_no[word]) {
        token_no[word] = ++token_num;
        no_token[token_num] = word;
    }
    token_list.push_back(Token(2, token_no[word], cur_line, cur_col, word));
    go_cur();
    return true;
}

bool Lexer::analyze(std::string input_code) {
    buffer = input_code;
    char_cnt = input_code.length();
    bool flag = true;
    for (; ! get_eof();) {
        char c = next_char();
        if (isspace(c)) {
            go_forw();
            go_cur();
        }
        else
            if (isdigit(c) || c == '$' || c == '&' || c == '%') {
                if (! number_analyze())
                    flag = false;
            }
            else
                if (isalpha(c) || c == '_') {
                    if (! word_analyze())
                        flag = false;
                }
                else
                    switch (c) {
                        case '{': {
                            go_forw();
                            bool match_flag = false;
                            for (; ! forw_get_eof(); go_forw())
                                if (next_char() == '}') {
                                    go_forw();
                                    go_cur();
                                    match_flag = true;
                                    break;
                                }
                            if (! match_flag) {
                                flag = false;
                                output_cur_error("unterminated { comment");
                                go_cur();
                            }
                            break;
                        }
                        case '(': {
                            go_forw();
                            if (next_char() == '*') {
                                go_forw();
                                bool match_flag = false;
                                for (bool pre_star = false; ! forw_get_eof(); go_forw()) {
                                    if (next_char() == ')' && pre_star) {
                                        go_forw();
                                        go_cur();
                                        match_flag = true;
                                        break;
                                    }
                                    pre_star = next_char() == '*';
                                }
                                if (! match_flag) {
                                    flag = false;
                                    output_cur_error("unterminated (* comment");
                                    go_cur();
                                }
                            }
                            else {
                                token_list.push_back(Token(7, 4, cur_line, cur_col));
                                go_cur();
                            }
                            break;
                        }
                        case '/':
                            go_forw();
                            if (next_char() == '/') {
                                go_forw();
                                for (; ! forw_get_eof() && next_char() != '\n'; go_forw());
                                if (! forw_get_eof())
                                    go_forw();
                                go_cur();
                            }
                            else {
                                token_list.push_back(Token(6, 10, cur_line, cur_col));
                                go_cur();
                            }
                            break;
                        case '\'':
                        case '#':
                            if (! string_analyze())
                                flag = false;
                            break;
                        case '=':
                            go_forw();
                            token_list.push_back(Token(6, 0, cur_line, cur_col));
                            go_cur();
                            break;
                        case ':':
                            go_forw();
                            if (next_char() == '=') {
                                go_forw();
                                token_list.push_back(Token(6, 1, cur_line, cur_col));
                                go_cur();
                            }
                            else {
                                token_list.push_back(Token(7, 2, cur_line, cur_col));
                                go_cur();
                            }
                            break;
                        case '<':
                            go_forw();
                            if (next_char() == '>') {
                                go_forw();
                                token_list.push_back(Token(6, 2, cur_line, cur_col));
                                    go_cur();
                            }
                            else
                                if (next_char() == '=') {
                                    go_forw();
                                    token_list.push_back(Token(6, 5, cur_line, cur_col));
                                    go_cur();
                                }
                                else
                                    if (next_char() == '<') {
                                        go_forw();
                                        token_list.push_back(Token(6, 11, cur_line, cur_col));
                                        go_cur();
                                    }
                                    else {
                                        token_list.push_back(Token(6, 3, cur_line, cur_col));
                                        go_cur();
                                    }
                            break;
                        case '>':
                            go_forw();
                            if (next_char() == '=') {
                                go_forw();
                                token_list.push_back(Token(6, 6, cur_line, cur_col));
                                go_cur();
                            }
                            else
                                if (next_char() == '>') {
                                    go_forw();
                                    token_list.push_back(Token(6, 12, cur_line, cur_col));
                                    go_cur();
                                }
                                else
                                    if (next_char() == '<') {
                                        go_forw();
                                        token_list.push_back(Token(6, 13, cur_line, cur_col));
                                        go_cur();
                                    }
                                    else {
                                        token_list.push_back(Token(6, 4, cur_line, cur_col));
                                        go_cur();
                                    }
                            break;
                        case '+':
                            go_forw();
                            token_list.push_back(Token(6, 7, cur_line, cur_col));
                            go_cur();
                            break;
                        case '-':
                            go_forw();
                            token_list.push_back(Token(6, 8, cur_line, cur_col));
                            go_cur();
                            break;
                        case '*':
                            go_forw();
                            token_list.push_back(Token(6, 9, cur_line, cur_col));
                            go_cur();
                            break;
                        case '^':
                            go_forw();
                            token_list.push_back(Token(6, 14, cur_line, cur_col));
                            go_cur();
                            break;
                        case '@':
                            go_forw();
                            token_list.push_back(Token(6, 15, cur_line, cur_col));
                            go_cur();
                            break;
                        case '.':
                            go_forw();
                            if (next_char() == '.') {
                                go_forw();
                                token_list.push_back(Token(7, 8, cur_line, cur_col));
                                go_cur();
                            }
                            else {
                                token_list.push_back(Token(7, 0, cur_line, cur_col));
                                go_cur();
                            }
                            break;
                        case ',':
                            go_forw();
                            token_list.push_back(Token(7, 1, cur_line, cur_col));
                            go_cur();
                            break;
                        case ';':
                            go_forw();
                            token_list.push_back(Token(7, 3, cur_line, cur_col));
                            go_cur();
                            break;
                        case ')':
                            go_forw();
                            token_list.push_back(Token(7, 5, cur_line, cur_col));
                            go_cur();
                            break;
                        case '[':
                            go_forw();
                            token_list.push_back(Token(7, 6, cur_line, cur_col));
                            go_cur();
                            break;
                        case ']':
                            go_forw();
                            token_list.push_back(Token(7, 7, cur_line, cur_col));
                            go_cur();
                            break;
                        default:
                            go_forw();
                            flag = false;
                            output_cur_error("unknown symbol");
                            go_cur();
                    }
    }
    for (int i = 0; i < token_list.size(); i++)
        std::cout << token_list[i].category << " " << token_list[i].no << " " << token_list[i].content << std::endl;
    return flag;
}