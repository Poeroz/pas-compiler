#include <iostream>
#include <cstdio>
#include <cstring>
#include <queue>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "args.h"
#include "parser.h"

int Parser::num_nonterminal = 24;

Parser::Parser() {
    symbol_table.parent = NULL;
    current_symbol_table = &symbol_table;
    indent = 0;
    grammar_init();
    cal_first();
    cal_collection_of_sets_of_items();
    cal_parsing_table();
}

Parser::~Parser() {}

std::pair<int, std::string> Parser::pas_int_to_c(int sign, std::string s) const {
    std::pair<int, std::string> res;
    if (! sign)
        if (isdigit(s[0])) {
            for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
            if (s.length() < 5 || (s.length() == 5 && s < "65536"))
                res.first = 3;
            else
                if (s.length() < 10 || (s.length() == 10 && s < "4294967296"))
                    res.first = 7;
                else
                    res.first = 10;
        }
        else
            if (s[0] == '%') {
                s = s.substr(1);
                for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                if (s.length() <= 16)
                    res.first = 3;
                else
                    if (s.length() <= 32)
                        res.first = 7;
                    else
                        res.first = 10;
                s = "0b" + s;
            }
            else
                if (s[0] == '&') {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    if (s.length() < 6 || (s.length() == 6 && s < "200000"))
                        res.first = 3;
                    else
                        if (s.length() < 11 || (s.length() == 11 && s < "40000000000"))
                            res.first = 7;
                        else
                            res.first = 10;
                    s = "0" + s;
                }
                else {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    if (s.length() <= 4)
                        res.first = 3;
                    else
                        if (s.length() <= 8)
                            res.first = 7;
                        else
                            res.first = 10;
                    s = "0x" + s;
                }
    else
        if (isdigit(s[0])) {
            for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
            if (s.length() < 5 || (s.length() == 5 && s <= "32768"))
                res.first = 14;
            else
                if (s.length() < 10 || (s.length() == 10 && s <= "2147483648"))
                    res.first = 17;
                else
                    res.first = 19;
        }
        else
            if (s[0] == '%') {
                s = s.substr(1);
                for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                if (s.length() < 16 || (s.length() == 16 && s == "1000000000000000"))
                    res.first = 14;
                else
                    if (s.length() < 32 || (s.length() == 32 && s == "10000000000000000000000000000000"))
                        res.first = 17;
                    else
                        res.first = 19;
                s = "0b" + s;
            }
            else
                if (s[0] == '&') {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    if (s.length() < 6 || (s.length() == 6 && s == "100000"))
                        res.first = 14;
                    else
                        if (s.length() < 11 || (s.length() == 11 && s <= "20000000000"))
                            res.first = 17;
                        else
                            res.first = 19;
                    s = "0" + s;
                }
                else {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    if (s.length() < 4 || (s.length() == 4 && s <= "8000"))
                        res.first = 14;
                    else
                        if (s.length() < 8 || (s.length() == 8 && s <= "80000000"))
                            res.first = 17;
                        else
                            res.first = 19;
                    s = "0x" + s;
                }
    res.second = s;
    return res;
}

std::pair<int, std::string> Parser::pas_string_to_c(std::string s) const {
    int cnt = 0;
    std::string res = "";
    for (int i = 0; i < s.length(); i++)
        if (s[i] == '\'') {
            int j = i + 1;
            for (; j < s.length(); j++)
                if (s[j] == '\'')
                    if (j + 1 < s.length() && s[j + 1] == '\'') {
                        res += "\\'";
                        cnt++;
                        j++;
                    }
                    else
                        break;
                else {
                    cnt++;
                    if (s[j] == '"')
                        res += "\\\"";
                    else
                        if (s[j] == '\\')
                            res += "\\\\";
                        else
                            res += s[j];
                }
            i = j;
        }
        else {
            cnt++;
            int j = i + 1;
            for (; j < s.length() && s[j] != '\'' && s[j] != '#'; j++);
            if (isdigit(s[i + 1])) {
                std::string tmp = s.substr(i + 1, j - i - 1);
                for (; tmp.length() > 1 && tmp[0] == '0'; tmp = tmp.substr(1));
                if (tmp.length() < 3 || (tmp.length() == 3 && tmp <= "255")) {
                    int x = std::stoi(tmp);
                    char tmp1[5];
                    memset(tmp1, 0, sizeof(tmp1));
                    sprintf(tmp1, "%02x", x);
                    res += "\\x" + std::string(tmp1);
                }
                else
                    return std::make_pair(-1, std::string(""));
            }
            else
                if (s[i + 1] == '%') {
                    std::string tmp = s.substr(i + 2, j - i - 1);
                    for (; tmp.length() > 1 && tmp[0] == '0'; tmp = tmp.substr(1));
                    if (tmp.length() <= 8) {
                        int x = std::stoi(tmp, 0, 2);
                        char tmp1[5];
                        memset(tmp1, 0, sizeof(tmp1));
                        sprintf(tmp1, "%02x", x);
                        res += "\\x" + std::string(tmp1);
                    }
                    else
                        return std::make_pair(-1, std::string(""));
                }
                else
                    if (s[i + 1] == '&') {
                        std::string tmp = s.substr(i + 2, j - i - 1);
                        for (; tmp.length() > 1 && tmp[0] == '0'; tmp = tmp.substr(1));
                        if (tmp.length() < 3 || (tmp.length() == 3 && tmp <= "377")) {
                            res += "\\";
                            if (tmp.length() == 1)
                                res += "00";
                            else
                                if (tmp.length() == 2)
                                    res += "0";
                            res += tmp;
                        }
                        else
                            return std::make_pair(-1, std::string(""));
                    }
                    else {
                        std::string tmp = s.substr(i + 2, j - i - 1);
                        for (; tmp.length() > 1 && tmp[0] == '0'; tmp = tmp.substr(1));
                        if (tmp.length() <= 2) {
                            res += "\\x";
                            if (tmp.length() == 1)
                                res += "0";
                            res += tmp;
                        }
                        else
                            return std::make_pair(-1, std::string(""));
                    }
            i = j - 1;
        }
    return std::make_pair(cnt, res);
}

std::string Parser::pas_basic_type_to_c(int type_no) const {
    switch (type_no) {
        case 0:
            return std::string("uint8_t ");
            break;
        case 1:
            return std::string("unsigned char ");
            break;
        case 2:
            return std::string("uint16_t ");
            break;
        case 3:
            return std::string("unsigned short ");
            break;
        case 4:
            return std::string("uint32_t ");
            break;
        case 5:
        case 6:
        case 7:
            return std::string("unsigned int ");
            break;
        case 8:
            return std::string("unsigned long ");
            break;
        case 9:
            return std::string("uint64_t ");
            break;
        case 10:
            return std::string("unsigned long long ");
            break;
        case 11:
            return std::string("int8_t ");
            break;
        case 12:
            return std::string("char ");
            break;
        case 13:
            return std::string("int16_t ");
            break;
        case 14:
        case 15:
            return std::string("short ");
            break;
        case 16:
            return std::string("int32_t ");
            break;
        case 17:
            return std::string("int ");
            break;
        case 18:
            return std::string("long ");
            break;
        case 19:
            return std::string("int64_t ");
            break;
        case 20:
        case 21:
            return std::string("float ");
            break;
        case 22:
        case 23:
        case 25:
        case 26:
            return std::string("double ");
            break;
        case 24:
            return std::string("long double ");
            break;
        case 27:
        case 28:
        case 29:
        case 30:
            return std::string("bool ");
            break;
        case 31:
            return std::string("char ");
            break;
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
            return std::string("std::string ");
            break;
        case 37:
            return std::string("char *");
    }
    return std::string("");
}

std::string Parser::id_with_type(Type *type, std::string id) const {
    std::string res = "";
    switch (type->category) {
        case 0:
            res = pas_basic_type_to_c(type->type_no);
            res += id;
            break;
        case 1:
            res = "*";
            res += id;
            if (type->pointer_type->category == 2)
                res = "(" + res + ")";
            res = id_with_type(type->pointer_type, res);
            break;
        case 2:
            if (type->array_index_type == 0)
                res = id + "[" + type->array_uprange + " - " + type->array_bias + " + 1]";
            else
                res = id + "['" + type->array_uprange + "' - '" + type->array_bias + "' + 1]";
            res = id_with_type(type->array_type, res);
            break;
        case 3:
            res = "enum {\n";
            for (int i = 0; i < type->enum_list.size(); i++) {
                for (int j = 0; j < indent + 1; j++)
                    res += "\t";
                res += no_token[type->enum_list[i].first];
                if (type->enum_list[i].second != "")
                    res += " = " + type->enum_list[i].second;
                if (i != type->enum_list.size() - 1)
                    res += ",";
                res += "\n";
            }
            res += "} " + id;
            break;
        case 6:
            res = no_token[type->named_id_no] + " " + id;
            break;
    }
    return res;
}

bool Parser::process_default(Token &new_token) {
    return true;
}

bool Parser::process_newline(Token &new_token) {
    result << "\n";
    return true;
}

bool Parser::process_M1(Token &new_token) {
    result << "#include <bits/stdc++.h>\n\n";
    return true;
}

bool Parser::process_label(Token &new_token) {
    std::string label = parsing_stack.back().second.content;
    if (label.length() > 4) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "label syntax error");
        return false;
    }
    for (int i = 0; i < label.length(); i++)
        if (! isdigit(label[i])) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "label syntax error");
            return false;
        }
    current_symbol_table->labels.insert(label);
    return true;
}

bool Parser::process_int_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already defined");
        return false;
    }
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "const ";
    switch (parsing_stack[parsing_stack.size() - 2].second.type->type_no) {
        case 3:
            result << "unsigned short ";
            break;
        case 7:
            result << "unsigned int ";
            break;
        case 10:
            result << "unsigned long long ";
            break;
        case 14:
            result << "short ";
            break;
        case 17:
            result << "int ";
            break;
        case 19:
            result << "long long ";
    }
    result << no_token[id_no] << " = ";
    result << parsing_stack[parsing_stack.size() - 2].second.content << ";\n";
    current_symbol_table->symbols[id_no] = parsing_stack[parsing_stack.size() - 2].second.type;
    current_symbol_table->is_const[id_no] = true;
    current_symbol_table->const_val[id_no] = parsing_stack[parsing_stack.size() - 2].second.content;
    return true;
}

bool Parser::process_float_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already defined");
        return false;
    }
    std::string constant = parsing_stack[parsing_stack.size() - 2].second.content;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "const double " << no_token[id_no] << " = ";
    result << constant << ";\n";
    current_symbol_table->symbols[id_no] = parsing_stack[parsing_stack.size() - 2].second.type;
    current_symbol_table->is_const[id_no] = true;
    current_symbol_table->const_val[id_no] = parsing_stack[parsing_stack.size() - 2].second.content;
    return true;
}

bool Parser::process_string_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already defined");
        return false;
    }
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "const std::string " << no_token[id_no] << " = \"" << parsing_stack[parsing_stack.size() - 2].second.content << "\";\n";
    current_symbol_table->symbols[id_no] = parsing_stack[parsing_stack.size() - 2].second.type;
    current_symbol_table->is_const[id_no] = true;
    current_symbol_table->const_val[id_no] = parsing_stack[parsing_stack.size() - 2].second.content;
    current_symbol_table->const_strlen[id_no] = parsing_stack[parsing_stack.size() - 2].second.str_len;
    return true;
}

bool Parser::process_bool_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already defined");
        return false;
    }
    Type *type = new Type;
    type->category = 0;
    type->type_no = 27;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "const bool " << no_token[id_no] << " = ";
    if (parsing_stack[parsing_stack.size() - 2].second.no == 45)
        result << "true;\n";
    else
        result << "false;\n";
    current_symbol_table->symbols[id_no] = type;
    current_symbol_table->is_const[id_no] = true;
    return true;
}

bool Parser::process_type_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->defined(id_no)) {
            output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already been defined");
            return false;
        }
    Type *type = new Type;
    type->category = 6;
    type->named_id_no = id_no;
    type->named_type = parsing_stack[parsing_stack.size() - 2].second.type;
    current_symbol_table->named_types[id_no] = type;
    result << "typedef ";
    result << id_with_type(parsing_stack[parsing_stack.size() - 2].second.type, no_token[id_no]);
    result << ";\n";
    return true;
}

bool Parser::process_id_type_denoter(Token &new_token) {
    int id_no = parsing_stack.back().second.no;
    Type *tmp = NULL;
    bool flag = false;
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->defined(id_no)) {
            if (p->named_types.count(id_no))
                tmp = p->named_types[id_no];
            flag = true;
            break;
        }
    if (! tmp) {
        if (! flag)
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier has not been defined");
        else
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier is not a type");
        return false;
    }
    new_token.type = tmp;
    return true;
}

bool Parser::process_basic_type_denoter(Token &new_token) {
    int id_no = parsing_stack.back().second.no;
    new_token.type = new Type;
    if (id_no == 37) {
        new_token.type->category = 1;
        new_token.type->pointer_type = new Type;
        new_token.type->pointer_type->category = 0;
        new_token.type->type_no = 31;
        return true;
    }
    new_token.type->category = 0;
    if (id_no >= 6 && id_no <= 8)
        id_no = 5;
    else
        if (id_no == 15)
            id_no = 14;
        else
            if (id_no == 21)
                id_no = 20;
            else
                if (id_no == 22 || id_no == 25 || id_no == 26)
                    id_no = 23;
                else
                    if (id_no >= 28 && id_no <= 30)
                        id_no = 27;
                    else
                        if (id_no == 32)
                            id_no = 33;
                        else
                            if (id_no >= 34 && id_no <= 36)
                                id_no = 33;
    new_token.type->type_no = id_no;
    return true;
}

bool Parser::process_pointer_type_denoter(Token &new_token) {
    new_token.type = new Type;
    new_token.type->category = 1;
    new_token.type->pointer_type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_array_type_denoter(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 4].second.type;
    Type *p = new_token.type;
    for (; p->array_type; p = p->array_type);
    p->array_type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_enum_type_denoter(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (int i = 0; i < new_token.type->enum_list.size(); i++)
        current_symbol_table->enum_items.insert(new_token.type->enum_list[i].first);
    return true;
}

bool Parser::process_array_single_subrange_list(Token &new_token) {
    new_token.type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_array_subrange_list(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    new_token.type->array_type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_array_subrange(Token &new_token) {
    if (parsing_stack[parsing_stack.size() - 3].second.type->type_no <= 19) {
        if (! (parsing_stack.back().second.type->type_no <= 19)) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible type: expected int but found char");
            return false;
        }
        int l = std::stoi(parsing_stack[parsing_stack.size() - 3].second.content, 0, 0), r = std::stoi(parsing_stack.back().second.content, 0, 0);
        if (l > r) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "high range limit < low range limit");
            return false;
        }
        new_token.type = new Type;
        new_token.type->category = 2;
        new_token.type->array_index_type = 0;
        new_token.type->array_bias = parsing_stack[parsing_stack.size() - 3].second.content;
        new_token.type->array_uprange = parsing_stack.back().second.content;
    }
    else {
        if (! (parsing_stack.back().second.type->type_no == 31 || parsing_stack.back().second.type->type_no == 33)) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible type: expected char but found int");
            return false;
        }
        char tmp[10];
        sprintf(tmp, parsing_stack[parsing_stack.size() - 3].second.content.c_str());
        int l = tmp[0];
        sprintf(tmp, parsing_stack.back().second.content.c_str());
        int r = tmp[0];
        if (l > r) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "high range limit < low range limit");
            return false;
        }
        new_token.type = new Type;
        new_token.type->category = 2;
        new_token.type->array_index_type = 1;
        new_token.type->array_bias = parsing_stack[parsing_stack.size() - 3].second.content;
        new_token.type->array_uprange = parsing_stack.back().second.content;
    }
    return true;
}

bool Parser::process_no_sign_signed_integer(Token &new_token) {
    std::string content = parsing_stack.back().second.content;
    Type *type = new Type;
    type->category = 0;
    std::pair<int, std::string> c_int = pas_int_to_c(0, content);
    type->type_no = c_int.first;
    new_token.type = type;
    new_token.content = c_int.second;
    return true;
}

bool Parser::process_signed_integer(Token &new_token) {
    std::string content = parsing_stack.back().second.content;
    Type *type = new Type;
    type->category = 0;
    std::pair<int, std::string> c_int = pas_int_to_c(parsing_stack[parsing_stack.size() - 2].second.no == 8, content);
    type->type_no = c_int.first;
    new_token.type = type;
    if (parsing_stack[parsing_stack.size() - 2].second.no == 7)
        new_token.content = '+' + c_int.second;
    else
        new_token.content = '-' + c_int.second;
    return true;
}

bool Parser::process_no_sign_signed_float(Token &new_token) {
    new_token.content = parsing_stack.back().second.content;
    Type *type = new Type;
    type->category = 0;
    type->type_no = 23;
    new_token.type = type;
    return true;
}

bool Parser::process_signed_float(Token &new_token) {
    new_token.content = parsing_stack.back().second.content;
    Type *type = new Type;
    type->category = 0;
    type->type_no = 23;
    new_token.type = type;
    if (parsing_stack[parsing_stack.size() - 2].second.no == 7)
        new_token.content = '+' + new_token.content;
    else
        new_token.content = '-' + new_token.content;
    return true;
}

bool Parser::process_string(Token &new_token) {
    std::pair<int, std::string> content = pas_string_to_c(parsing_stack.back().second.content);
    if (content.first == -1) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "string format error");
        return false;
    }
    Type *type = new Type;
    type->category = 0;
    type->type_no = 33;
    new_token.type = type;
    new_token.content = content.second;
    new_token.str_len = content.first;
    return true;
}

bool Parser::process_array_index(Token &new_token) {
    if (parsing_stack.back().second.category == 2) {
        int id_no = parsing_stack.back().second.no;
        bool flag = false;
        bool constant_flag = false;
        new_token.type = NULL;
        for (SymbolTable *p = current_symbol_table; p; p = p->parent)
            if (p->defined(id_no)) {
                if (p->is_const.count(id_no)) {
                    constant_flag = true;
                    if (p->symbols[id_no]->category == 0 && (p->symbols[id_no]->type_no <= 19 || p->symbols[id_no]->type_no == 31 || (p->symbols[id_no]->type_no == 33 && p->const_strlen[id_no] == 1))) {
                        new_token.type = p->symbols[id_no];
                        new_token.content = p->const_val[id_no];
                        if (p->symbols[id_no]->type_no == 33)
                            new_token.str_len = p->const_strlen[id_no];
                    }
                }
                flag = true;
                break;
            }
        if (! new_token.type) {
            if (! flag)
                output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier has not been defined");
            else
                if (! constant_flag)
                    output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier is not a constant");
                else
                    output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible type");
            return false;
        }
        return true;
    }
    else {
        if (parsing_stack.back().second.type->type_no == 33 && parsing_stack.back().second.str_len != 1) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible type");
            return false;
        }
        new_token.type = parsing_stack.back().second.type;
        new_token.content = parsing_stack.back().second.content;
        if (parsing_stack.back().second.type->type_no == 33)
            new_token.str_len = parsing_stack.back().second.str_len;
        return true;
    }
}

bool Parser::process_single_enum_list(Token &new_token) {
    new_token.type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_enum_list(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    for (int i = 0; i < new_token.type->enum_list.size(); i++)
        if (new_token.type->enum_list[i].first == parsing_stack.back().second.type->enum_list[0].first) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "duplicate identifier");
            return false;
        }
    new_token.type->enum_list.push_back(parsing_stack.back().second.type->enum_list[0]);
    delete parsing_stack.back().second.type;
    return true;
}

bool Parser::process_enum_item(Token &new_token) {
    new_token.type = new Type;
    new_token.type->category = 3;
    std::pair<int, std::string> enum_item;
    if (parsing_stack.back().second.category == 2) {
        for (SymbolTable *p = current_symbol_table; p; p = p->parent)
            if (p->defined(parsing_stack.back().second.no)) {
                output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier has already been defined");
                return false;
            }
        enum_item = std::make_pair(parsing_stack.back().second.no, "");
    }
    else {
        for (SymbolTable *p = current_symbol_table; p; p = p->parent)
            if (p->defined(parsing_stack[parsing_stack.size() - 3].second.no)) {
                output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "identifier has already been defined");
                return false;
            }
        enum_item = std::make_pair(parsing_stack[parsing_stack.size() - 3].second.no, parsing_stack.back().second.content);
    }
    new_token.type->enum_list.push_back(enum_item);
    return true;
}

void Parser::grammar_init() {
    Generation tmp;
    //program' = program
    tmp.left = 0;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 1));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program = M1 program-heading ';' program-block '.'
    tmp.left = 1;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 2));
    tmp.right.push_back(Symbol(-1, 3));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(-1, 4));
    tmp.right.push_back(Symbol(7, 0));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program = M1 program-block '.'
    tmp.left = 1;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 2));
    tmp.right.push_back(Symbol(-1, 4));
    tmp.right.push_back(Symbol(7, 0));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M1 = ε
    tmp.left = 2;
    tmp.right.clear();
    tmp.process = &Parser::process_M1;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-heading = 'program' identifier
    tmp.left = 3;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 36));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-heading = 'program' identifier '(' identifier-list ')'
    tmp.left = 3;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 36));
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 5));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //identifier-list = identifier
    tmp.left = 5;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //identifier-list = identifier-list ',' identifier
    tmp.left = 5;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 5));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-block = definition-part
    tmp.left = 4;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //definition-part = ε
    tmp.left = 6;
    tmp.right.clear();
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //definition-part = definition-part label-declaration-part
    tmp.left = 6;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.right.push_back(Symbol(-1, 7));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //definition-part = definition-part constant-definition-part
    tmp.left = 6;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.right.push_back(Symbol(-1, 9));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //definition-part = definition-part type-definition-part
    tmp.left = 6;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.right.push_back(Symbol(-1, 12));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-declaration-part = 'label' label-list ';'
    tmp.left = 7;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 25));
    tmp.right.push_back(Symbol(-1, 8));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-list = label
    tmp.left = 8;
    tmp.right.clear();
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_label;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-list = label-list ',' label
    tmp.left = 8;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 8));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_label;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //constant-definition-part = 'const' const-definitions
    tmp.left = 9;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 6));
    tmp.right.push_back(Symbol(-1, 10));
    tmp.process = &Parser::process_newline;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definitions = const-definition
    tmp.left = 10;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 11));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definitions = const-definitions const-definition
    tmp.left = 10;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 10));
    tmp.right.push_back(Symbol(-1, 11));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' signed-integer ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 18));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_int_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' signed-float ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 19));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_float_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' string ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 20));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_string_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' true ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(0, 45));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_bool_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' false ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(0, 15));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_bool_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-definition-part = 'type' type-definitions
    tmp.left = 12;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 46));
    tmp.right.push_back(Symbol(-1, 13));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-definitions = type-definition
    tmp.left = 13;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 14));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-definitions = type-definitions type-definition
    tmp.left = 13;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 13));
    tmp.right.push_back(Symbol(-1, 14));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-definition = identifier '=' type-denoter ';'
    tmp.left = 14;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 15));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_type_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = identifier
    tmp.left = 15;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_id_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = origin-type
    tmp.left = 15;
    tmp.right.clear();
    tmp.right.push_back(Symbol(1, 0));
    tmp.process = &Parser::process_basic_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = '^' type-denoter
    tmp.left = 15;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 14));
    tmp.right.push_back(Symbol(-1, 15));
    tmp.process = &Parser::process_pointer_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = array '[' subrange-list ']' of type-denoter
    tmp.left = 15;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 1));
    tmp.right.push_back(Symbol(7, 6));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.right.push_back(Symbol(7, 7));
    tmp.right.push_back(Symbol(0, 30));
    tmp.right.push_back(Symbol(-1, 15));
    tmp.process = &Parser::process_array_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = '(' enum-list ')'
    tmp.left = 15;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 22));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_enum_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //subrange-list = subrange
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 17));
    tmp.process = &Parser::process_array_single_subrange_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //subrange-list = subrange ',' subrange-list
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 17));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.process = &Parser::process_array_subrange_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //subrange = array-index '..' array-index
    tmp.left = 17;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 21));
    tmp.right.push_back(Symbol(7, 8));
    tmp.right.push_back(Symbol(-1, 21));
    tmp.process = &Parser::process_array_subrange;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-integer = integer-literal
    tmp.left = 18;
    tmp.right.clear();
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_no_sign_signed_integer;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-integer = '+' integer-literal
    tmp.left = 18;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 7));
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_signed_integer;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-integer = '-' integer-literal
    tmp.left = 18;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 8));
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_signed_integer;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-float = float-literal
    tmp.left = 19;
    tmp.right.clear();
    tmp.right.push_back(Symbol(4, 0));
    tmp.process = &Parser::process_no_sign_signed_float;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-float = '+' float-literal
    tmp.left = 19;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 7));
    tmp.right.push_back(Symbol(4, 0));
    tmp.process = &Parser::process_signed_float;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-float = '-' float-literal
    tmp.left = 19;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 8));
    tmp.right.push_back(Symbol(4, 0));
    tmp.process = &Parser::process_signed_float;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //string = string-literal
    tmp.left = 20;
    tmp.right.clear();
    tmp.right.push_back(Symbol(5, 0));
    tmp.process = &Parser::process_string;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-index = identifier
    tmp.left = 21;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_array_index;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-index = signed-integer
    tmp.left = 21;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 18));
    tmp.process = &Parser::process_array_index;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-index = string
    tmp.left = 21;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 20));
    tmp.process = &Parser::process_array_index;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-list = enum-item
    tmp.left = 22;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 23));
    tmp.process = &Parser::process_single_enum_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-list = enum-list ',' enum-item
    tmp.left = 22;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 22));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(-1, 23));
    tmp.process = &Parser::process_enum_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-item = identifier
    tmp.left = 23;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_enum_item;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-item = identifier ':=' signed-integer
    tmp.left = 23;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 1));
    tmp.right.push_back(Symbol(-1, 18));
    tmp.process = &Parser::process_enum_item;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
}

int Parser::get_matrix_idx(Symbol symbol) {
    switch (symbol.category) {
        case 0:
            return num_nonterminal + symbol.no;
        case 1:
            return num_nonterminal + num_keywords + symbol.no;
        case 2:
            return num_nonterminal + num_keywords + num_data_types;
        case 3:
            return num_nonterminal + num_keywords + num_data_types + 1;
        case 4:
            return num_nonterminal + num_keywords + num_data_types + 2;
        case 5:
            return num_nonterminal + num_keywords + num_data_types + 3;
        case 6:
            return num_nonterminal + num_keywords + num_data_types + 4 + symbol.no;
        case 7:
            return num_nonterminal + num_keywords + num_data_types + 4 + num_operators + symbol.no;
        case 8:
            return num_nonterminal + num_keywords + num_data_types + 4 + num_operators + num_symbols + symbol.no;
    }
    return symbol.no;
}

Symbol Parser::get_symbol(int num) {
    if (num < num_nonterminal)
        return Symbol(-1, num);
    num -= num_nonterminal;
    if (num < num_keywords)
        return Symbol(0, num);
    num -= num_keywords;
    if (num < num_data_types)
        return Symbol(1, num);
    num -= num_data_types;
    if (num == 0)
        return Symbol(2, 0);
    if (num == 1)
        return Symbol(3, 0);
    if (num == 2)
        return Symbol(4, 0);
    if (num == 3)
        return Symbol(5, 0);
    num -= 4;
    if (num < num_operators)
        return Symbol(6, num);
    num -= num_operators;
    if (num < num_symbols)
        return Symbol(7, num);
    num -= num_symbols;
    return Symbol(8, num);
}

void Parser::cal_first() {
    matrix_first.reset();
    empty.resize(num_nonterminal);
    for (int i = 0; i < grammar.size(); i++)
        if (! grammar[i].right.size())
            empty[grammar[i].left] = true;
    for (int t = 0; t < num_nonterminal; t++)
        for (int i = 0; i < grammar.size(); i++)
            if (! empty[grammar[i].left]) {
                bool flag = true;
                for (int j = 0; j < grammar[i].right.size(); j++)
                    if (grammar[i].right[j].category >= 0 || (grammar[i].right[j].category == -1 && ! empty[grammar[i].right[j].no])) {
                        flag = false;
                        break;
                    }
                if (flag) {
                    empty[grammar[i].left] = true;
                    break;
                }
            }
    for (int i = 0; i < grammar.size(); i++)
        if (grammar[i].right.size()) {
            if (grammar[i].right[0].category >= 0)
                matrix_first[grammar[i].left][get_matrix_idx(grammar[i].right[0])] = true;
            else {
                int k = 0;
                for (; k < grammar[i].right.size() && grammar[i].right[k].category == -1 && empty[grammar[i].right[k].no]; k++)
                    matrix_first[grammar[i].left][grammar[i].right[k].no] = true;
                if (k < grammar[i].right.size())
                    matrix_first[grammar[i].left][get_matrix_idx(grammar[i].right[k])] = true;
            }
        }
    matrix_first.transitive_closure();
    first.resize(num_nonterminal);
    for (int i = 0; i < num_nonterminal; i++) {
        if (empty[i])
            first[i].push_back(Symbol(EMPTY_SYMBOL, 0));
        for (int j = num_nonterminal; j < num_nonterminal + num_terminal; j++)
            if (matrix_first[i][j])
                first[i].push_back(get_symbol(j));
        /*std::cout << i << ":\n";
        for (int j = 0; j < first[i].size(); j++)
            std::cout << first[i][j].category << " " << first[i][j].no << "\n";*/
    }
}

std::set<Parser::Item> Parser::closure(const std::set<Item> &s) {
    std::set<Item> res;
    std::queue<Item> q;
    for (std::set<Item>::iterator it = s.begin(); it != s.end(); it++) {
        Item x = *it;
        res.insert(x);
        if (x.dot_pos < grammar[x.generation_no].right.size() && grammar[x.generation_no].right[x.dot_pos].category == -1) {
            std::set<Symbol> lookahead;
            bool right_empty = true;
            for (int k = x.dot_pos + 1; k < grammar[x.generation_no].right.size(); k++)
                if (grammar[x.generation_no].right[k].category >= 0) {
                    lookahead.insert(grammar[x.generation_no].right[k]);
                    right_empty = false;
                    break;
                }
                else {
                    for (int l = 0; l < first[grammar[x.generation_no].right[k].no].size(); l++)
                        lookahead.insert(first[grammar[x.generation_no].right[k].no][l]);
                    if (! empty[grammar[x.generation_no].right[k].no]) {
                        right_empty = false;
                        break;
                    }
                }
            if (right_empty)
                for (std::set<Symbol>::iterator it = x.lookahead.begin(); it != x.lookahead.end(); it++)
                    lookahead.insert(*it);
            std::vector<int> tmp = nonterminal_grammar[grammar[x.generation_no].right[x.dot_pos].no];
            for (int i = 0; i < tmp.size(); i++) {
                Item new_item(tmp[i], 0);
                new_item.lookahead = lookahead;
                q.push(new_item);
            }
        }
    }
    for (; ! q.empty();) {
        Item x = q.front();
        q.pop();
        if (res.count(x)) {
            Item tmp = *(res.find(x));
            res.erase(res.find(x));
            for (std::set<Symbol>::iterator it = x.lookahead.begin(); it != x.lookahead.end();)
                if (! tmp.lookahead.count(*it)) {
                    tmp.lookahead.insert(*it);
                    it++;
                }
                else
                    x.lookahead.erase(it++);
            res.insert(tmp);
            if (x.lookahead.empty())
                continue;
        }
        else
            res.insert(x);
        if (x.dot_pos < grammar[x.generation_no].right.size() && grammar[x.generation_no].right[x.dot_pos].category == -1) {
            std::set<Symbol> lookahead;
            bool right_empty = true;
            for (int k = x.dot_pos + 1; k < grammar[x.generation_no].right.size(); k++)
                if (grammar[x.generation_no].right[k].category >= 0) {
                    lookahead.insert(grammar[x.generation_no].right[k]);
                    right_empty = false;
                    break;
                }
                else {
                    for (int l = 0; l < first[grammar[x.generation_no].right[k].no].size(); l++)
                        lookahead.insert(first[grammar[x.generation_no].right[k].no][l]);
                    if (! empty[grammar[x.generation_no].right[k].no]) {
                        right_empty = false;
                        break;
                    }
                }
            if (right_empty)
                for (std::set<Symbol>::iterator it = x.lookahead.begin(); it != x.lookahead.end(); it++)
                    lookahead.insert(*it);
            std::vector<int> tmp = nonterminal_grammar[grammar[x.generation_no].right[x.dot_pos].no];
            for (int i = 0; i < tmp.size(); i++) {
                Item new_item(tmp[i], 0);
                new_item.lookahead = lookahead;
                q.push(new_item);
            }
        }
    }
    return res;
}

void Parser::cal_collection_of_sets_of_items() {
    Item start(0, 0);
    start.lookahead.insert(Symbol(END_OF_TOKENS, 0));
    std::set<Item> i0;
    i0.insert(start);
    i0 = closure(i0);
    set_of_items.push_back(i0);
    set_of_items_no[i0] = 0;
    std::queue<int> q;
    q.push(0);
    for (; ! q.empty();) {
        int x = q.front();
        q.pop();
        std::set<Symbol> next_symbols;
        for (std::set<Item>::iterator it = set_of_items[x].begin(); it != set_of_items[x].end(); it++)
            if (it->dot_pos < grammar[it->generation_no].right.size())
                next_symbols.insert(grammar[it->generation_no].right[it->dot_pos]);
        for (std::set<Symbol>::iterator it = next_symbols.begin(); it != next_symbols.end(); it++) {
            Symbol next_symbol = *it;
            std::set<Item> next_set_of_items;
            for (std::set<Item>::iterator it = set_of_items[x].begin(); it != set_of_items[x].end(); it++)
                if (it->dot_pos < grammar[it->generation_no].right.size() && grammar[it->generation_no].right[it->dot_pos] == next_symbol) {
                    Item y(it->generation_no, it->dot_pos + 1);
                    y.lookahead = it->lookahead;
                    next_set_of_items.insert(y);
                }
            next_set_of_items = closure(next_set_of_items);
            if (! set_of_items_no.count(next_set_of_items)) {
                set_of_items.push_back(next_set_of_items);
                int no = set_of_items.size() - 1;
                set_of_items_no[next_set_of_items] = no;
                transfer.resize(no + 1);
                q.push(no);
            }
            else {
                bool flag = false;
                int tmp = set_of_items_no[next_set_of_items];
                for (std::set<Item>::iterator it = next_set_of_items.begin(); it != next_set_of_items.end(); it++) {
                    std::set<Item>::iterator it1 = set_of_items[tmp].find(*it);
                    Item tmp1 = *it1;
                    bool flag1 = false;
                    for (std::set<Symbol>::iterator i = it->lookahead.begin(); i != it->lookahead.end(); i++)
                        if (! it1->lookahead.count(*i)) {
                            flag = flag1 = true;
                            tmp1.lookahead.insert(*i);
                        }
                    if (flag1) {
                        set_of_items[tmp].erase(it1);
                        set_of_items[tmp].insert(tmp1);
                    }
                }
                if (flag) {
                    set_of_items_no.erase(set_of_items_no.find(next_set_of_items));
                    next_set_of_items = set_of_items[tmp];
                    set_of_items_no[next_set_of_items] = tmp;
                    q.push(tmp);
                }
            }
            transfer[x][next_symbol] = set_of_items_no[next_set_of_items];
        }
    }
    state_num = set_of_items.size();
    /*std::cout << state_num << std::endl;
    for (int i = 0; i < state_num; i++) {
        std::cout << i << "   "<< transfer[i].size() << std::endl;
        for (auto j = transfer[i].begin(); j != transfer[i].end(); j++)
            std::cout << j->first.category << " " << j->first.no << "  " << j->second << std::endl;
    }*/
}

void Parser::cal_parsing_table() {
    parsing_table.resize(state_num);
    for (int i = 0; i < state_num; i++) {
        for (std::map<Symbol, int>::iterator it = transfer[i].begin(); it != transfer[i].end(); it++)
            parsing_table[i][it->first] = std::make_pair(1, it->second);
        for (std::set<Item>::iterator it = set_of_items[i].begin(); it != set_of_items[i].end(); it++)
            if (it->dot_pos == grammar[it->generation_no].right.size()) {
                if (it->generation_no == 0)
                    parsing_table[i][Symbol(END_OF_TOKENS, 0)] = std::make_pair(3, 0);
                else
                    for (std::set<Symbol>::iterator it1 = it->lookahead.begin(); it1 != it->lookahead.end(); it1++)
                        parsing_table[i][*it1] = std::make_pair(2, it->generation_no);
            }
    }
}

bool Parser::parse(std::vector<Token> tokens) {
    bool flag = true;
    token_list = tokens;
    parsing_stack.push_back(std::make_pair(0, Token(EMPTY_SYMBOL, 0)));
    int cur_ptr = 0;
    for (; ;) {
        Token cur_token;
        if (cur_ptr == token_list.size())
            cur_token = Token(END_OF_TOKENS, 0);
        else
            cur_token = token_list[cur_ptr];
        if (parsing_table[parsing_stack.back().first][Symbol(cur_token)].first == 0) {
            output_error(cur_token.line, cur_token.col, cur_token.pos, "unexpected token");
            flag = false;
            break;
        }
        if (parsing_table[parsing_stack.back().first][Symbol(cur_token)].first == 3)
            break;
        if (parsing_table[parsing_stack.back().first][Symbol(cur_token)].first == 1) {
            parsing_stack.push_back(std::make_pair(parsing_table[parsing_stack.back().first][Symbol(cur_token)].second, cur_token));
            cur_ptr++;
        }
        else {
            int generation_no = parsing_table[parsing_stack.back().first][Symbol(cur_token)].second;
            Token new_token(-1, grammar[generation_no].left, 
                parsing_stack[parsing_stack.size() - grammar[generation_no].right.size()].second.line, 
                parsing_stack[parsing_stack.size() - grammar[generation_no].right.size()].second.col, 
                parsing_stack[parsing_stack.size() - grammar[generation_no].right.size()].second.pos);
            if (! (this->*(grammar[generation_no].process))(new_token))
                flag = false;
            for (int i = 0; i < grammar[generation_no].right.size(); i++)
                parsing_stack.pop_back();
            parsing_stack.push_back(std::make_pair(parsing_table[parsing_stack.back().first][Symbol(new_token)].second, new_token));
        }
    }
    return flag;
}

std::string Parser::get_result() const {
    return result.str();
}