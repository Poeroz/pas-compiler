#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <unordered_map>

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
        42: string
        43: then
        44: to
        45: true
        46: type
        47: unit
        48: until
        49: uses
        50: var
        51: while
        52: with
        53: xor
    1: data types
        0: uint8
        1: byte
        2: uint16
        3: word
        4: dword
        5: cardinal
        6: uint32
        7: longword
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
struct Token {
    int category, no, line, col;
    std::string content;
    Token() {}
    Token(int category, int no, int line, int col, std::string content = "") :
        category(category), no(no), line(line), col(col), content(content) {}
};


extern const int num_keywords, num_data_types, num_rtl_functions;
extern const std::string keywords[];
extern const std::string data_types[];
extern const std::string rtl_functions[];

extern std::string INPUT_FILE_NAME;
extern int token_num;
extern std::unordered_map<std::string, int> token_no;
extern std::unordered_map<int, std::string> no_token;


#endif