#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <climits>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cctype>

#define ERROR_OUT "\033[31m\033[1merror:\033[0m "
#define ERROR_POINTER "\033[32m^\033[0m"


/*
    Token Category & Number:
    0: keywords
        0: and
        1: array
        2: asm
        3: begin
        4: break
        5: case
        6: const
        7: constructor
        8: continue
        9: destructor
        10: div
        11: do
        12: downto
        13: else
        14: end
        15: false
        16: file
        17: for
        18: function
        19: goto
        20: if
        21: implementation
        22: in
        23: inline
        24: interface
        25: label
        26: mod
        27: nil
        28: not
        29: object
        30: of
        31: on
        32: operator
        33: or
        34: packed
        35: procedure
        36: program
        37: record
        38: repeat
        39: set
        40: shl
        41: shr
        42: then
        43: to
        44: true
        45: type
        46: unit
        47: until
        48: uses
        49: var
        50: while
        51: with
        52: xor
        53: forward
    1: data types
        0: uint8
        1: byte
        2: uint16
        3: word
        4: uint32
        5: longword
        6: dword
        7: cardinal
        8: nativeuint
        9: uint64
        10: qword
        11: int8
        12: shortint
        13: int16
        14: smallint
        15: integer
        16: int32
        17: longint
        18: nativeint
        19: int64
        20: single
        21: real
        22: real48
        23: double
        24: extended
        25: comp
        26: currency
        27: boolean
        28: bytebool
        29: wordbool
        30: longbool
        31: char
        32: shortstring
        33: string
        34: pchar
        35: ansistring
        36: pansichar
        37: pointer
    2: identifiers
    3: integer literals
    4: float literals
    5: string literals
    6: operators
        0: =
        1: :=
        2: <>
        3: <
        4: >
        5: <=
        6: >=
        7: +
        8: -
        9: *
        10: /
        11: <<
        12: >>
        13: ><
        14: ^
        15: @
    7: symbols
        0: .
        1: ,
        2: :
        3: ;
        4: (
        5: )
        6: [
        7: ]
        8: ..
    8: RTL functions
        0: read
        1: readln
        2: readstr
        3: write
        4: writeln
        5: writestr
        6: sizeof
        7: exit
        8: halt
        9: chr
        10: concat
        11: copy
        12: delete
        13: insert
        14: length
        15: lowercase
        16: pos
        17: setlength
        18: setstring
        19: str
        20: stringofchar
        21: upcase
        22: val
        23: abs
        24: arctan
        25: cos
        26: dec
        27: exp
        28: frac
        29: inc
        30: int
        31: ln
        32: odd
        33: pi
        34: random
        35: randomize
        36: round
        37: sin
        38: sqr
        39: sqrt
        40: trunc
        41: include
        42: exclude
        43: fillchar
        44: fillbyte
        45: move
*/
extern const int num_terminal;

/*
    0: basic types
    1: pointer
    2: array
    3: enumerate
    4: record
    5: set
    6: named type
*/

struct Type {
    int category;
    union {
        int type_no;
        int named_id_no;
    };
    union {
        Type* pointer_type;
        Type* array_type;
        Type* set_type;
        Type* named_type;
    };
    int array_index_type; // int -- 0 char -- 1 no_index -- -1
    std::string array_uprange, array_bias;
    std::vector<std::pair<int, std::string> > enum_list;
    std::vector<std::pair<std::vector<int>, Type*> > record_list;
    bool is_base_type() const;
    bool no_constructed_type() const;
    bool can_be_defined_in_set() const;
};

struct FuncType {
    int id_no;
    Type* ret_type; // NULL -- procedure
    std::vector<std::pair<int, std::pair<int, Type*> > > param_list; // 0 -- ordinal 1 -- ref 2 -- const
    bool defined;
};

struct Symbol;

struct Token {
    int category, no, line, col, pos;
    std::string content;
    std::vector<int> id_list, id_line, id_col, id_pos;
    std::vector<Type*> expr_type;
    std::vector<bool> expr_const;
    union {
        int str_len;
        int array_len;
        int id_num;
        int id_no;
    };
    std::unordered_set<int> record_defined_ids;
    Type *type;
    bool is_const;
    Token();
    Token(int category, int no, int line = 1, int col = 1, int pos = 0, std::string content = "");
    Token(Symbol s);
};

/*
    -1: nonterminal symbols
*/
#define END_OF_TOKENS INT_MAX
//end of tokens only in lookahead
#define EMPTY_SYMBOL INT_MIN
//empty symbol only in first
struct Symbol {
    int category, no;
    Symbol();
    Symbol(int category, int no);
    Symbol(Token t);
    bool operator < (const Symbol &_) const;
    bool operator == (const Symbol &_) const;
};

extern int label_cnt;

struct SymbolTable {
    SymbolTable* parent;
    FuncType *functype; //for procedures and functions
    std::unordered_map<std::string, int> labels;
    std::unordered_map<int, Type*> named_types;
    std::unordered_map<int, Type*> symbols;
    std::unordered_map<int, bool> is_const;
    std::unordered_map<int, std::string> const_val; //for integers, floats and strings
    std::unordered_map<int, std::vector<SymbolTable*> > subtable;
    std::unordered_map<int, Type*> enum_items;
    bool defined(int no) const;
    bool defined_except_func(int no) const;
};

extern const int num_keywords, num_data_types, num_rtl_functions, num_operators, num_symbols;
extern const std::string keywords[];
extern const std::string data_types[];
extern const std::string rtl_functions[];

extern std::string INPUT_FILE_NAME;
extern std::string input_code;

extern int token_num;
extern std::unordered_map<std::string, int> token_no;
extern std::unordered_map<int, std::string> no_token;


std::string get_line(int pos);
void output_error(int line, int col, int pos, std::string error_info);

#endif