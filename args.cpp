#include <string>
#include <unordered_map>
#include "args.h"

const int num_keywords = 54, num_data_types = 38, num_rtl_functions = 46;
const std::string keywords[54] = {"and", "array", "asm", "begin", "break", "case", "const", "constructor", "continue", "destructor",
                                  "div", "do", "downto", "else", "end", "false", "file", "for", "function", "goto",
                                  "if", "implementation", "in", "inline", "interface", "label", "mod", "nil", "not", "object",
                                  "of", "on", "operator", "or", "packed", "procedure", "program", "record", "repeat", "set",
                                  "shl", "shr", "string", "then", "to", "true", "type", "unit", "until", "uses",
                                  "var", "while", "whith", "xor"};
const std::string data_types[38] = {"uint8", "byte", "uint16", "word", "dword", "cardinal", "uint32", "longword", "nativeuint", "uint64",
                                    "qword", "int8", "shortint", "int16", "smallint", "integer", "int32", "longint", "nativeint", "int64",
                                    "single", "real", "real48", "double", "extended", "comp", "currency", "boolean", "bytebool", "wordbool",
                                    "longbool", "char", "shortstring", "string", "pchar", "ansistring", "pansichar", "pointer"};
const std::string rtl_functions[46] = {"read", "readln", "readstr", "write", "writeln", "writestr", "sizeof", "exit", "halt", "chr",
                                       "concat", "copy", "delete", "insert", "length", "lowercase", "pos", "setlength", "setstring", "str",
                                       "stringofchar", "upcase", "val", "abs", "arctan", "cos", "dec", "exp", "frac", "inc",
                                       "int", "ln", "odd", "pi", "random", "randomize", "round", "sin", "sqr", "sqrt",
                                       "trunc", "include", "exclude", "fillchar", "fillbyte", "move"};

std::string INPUT_FILE_NAME = "";
int token_num = 0;
std::unordered_map<std::string, int> token_no;
std::unordered_map<int, std::string> no_token;