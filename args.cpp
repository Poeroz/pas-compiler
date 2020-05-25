#include <string>
#include <unordered_map>
#include <iostream>
#include "args.h"

int label_cnt = 0;

const int num_keywords = 55, num_data_types = 38, num_rtl_functions = 46, num_operators = 16, num_symbols = 9;
const std::string keywords[55] = {"and", "array", "asm", "begin", "break", "case", "const", "constructor", "continue", "destructor",
                                  "div", "do", "downto", "else", "end", "false", "file", "for", "function", "goto",
                                  "if", "implementation", "in", "inline", "interface", "label", "mod", "nil", "not", "object",
                                  "of", "on", "operator", "or", "packed", "procedure", "program", "record", "repeat", "set",
                                  "shl", "shr", "string", "then", "to", "true", "type", "unit", "until", "uses",
                                  "var", "while", "whith", "xor", "forward"};
const std::string data_types[38] = {"uint8", "byte", "uint16", "word", "uint32", "longword", "dword", "cardinal", "nativeuint", "uint64",
                                    "qword", "int8", "shortint", "int16", "smallint", "integer", "int32", "longint", "nativeint", "int64",
                                    "single", "real", "real48", "double", "extended", "comp", "currency", "boolean", "bytebool", "wordbool",
                                    "longbool", "char", "shortstring", "string", "pchar", "ansistring", "pansichar", "pointer"};
const std::string rtl_functions[46] = {"read", "readln", "readstr", "write", "writeln", "writestr", "sizeof", "exit", "halt", "chr",
                                       "concat", "copy", "delete", "insert", "length", "lowercase", "pos", "setlength", "setstring", "str",
                                       "stringofchar", "upcase", "val", "abs", "arctan", "cos", "dec", "exp", "frac", "inc",
                                       "int", "ln", "odd", "pi", "random", "randomize", "round", "sin", "sqr", "sqrt",
                                       "trunc", "include", "exclude", "fillchar", "fillbyte", "move"};
const int num_terminal = 168;

bool Type::is_base_type() const {
    return category == 0 || category == 1 || (category == 6 && named_type->is_base_type());
}

bool Type::no_constructed_type() const {
    return category == 0 || (category == 1 && pointer_type->no_constructed_type()) || category == 6;
}

bool Type::can_be_defined_in_set() const {
    return category == 0 || (category == 1 && pointer_type->no_constructed_type()) || (category == 6 && named_type->is_base_type());
}

Token::Token() {}

Token::Token(int category, int no, int line, int col, int pos, std::string content) :
    category(category), no(no), line(line), col(col), pos(pos), content(content) {}

Token::Token(Symbol s) :
    category(s.category), no(s.no) {}

Symbol::Symbol() {}

Symbol::Symbol(int category, int no) :
    category(category), no(no) {}

Symbol::Symbol(Token t) :
    category(t.category), no(t.no) {
    if (t.category >= 1 && t.category <= 5)
        no = 0;
}

bool Symbol::operator < (const Symbol &_) const {
    return category == _.category ? no < _.no : category < _.category;
}

bool Symbol::operator == (const Symbol &_) const {
    return category == _.category && no == _.no;
}

bool SymbolTable::defined(int no) const {
    return named_types.count(no) || symbols.count(no) || subtable.count(no) || enum_items.count(no);
}

bool SymbolTable::defined_except_func(int no) const {
    return named_types.count(no) || symbols.count(no) || enum_items.count(no);
}

std::string INPUT_FILE_NAME = "";
std::string input_code = "";
bool USE_LL1_PARSER = false;

int token_num = 0;
std::unordered_map<std::string, int> token_no;
std::unordered_map<int, std::string> no_token;


std::string get_line(int pos) {
    int st = pos, ed = pos;
    for (; st >= 0 && input_code[st] != '\n'; st--);
    st++;
    for (; ed < input_code.length() && input_code[ed] != '\n'; ed++);
    return input_code.substr(st, ed - st);
}

void output_error(int line, int col, int pos, std::string error_info) {
    std::cerr << line << ":" << col << ": " << ERROR_OUT << error_info << std::endl;
    std::cerr << get_line(pos) << std::endl;
    for (int i = 0; i < col - 1; i++)
        std::cerr << ' ';
    std::cerr << ERROR_POINTER << std::endl;
}