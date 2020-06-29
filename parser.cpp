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

int Parser::num_nonterminal = 88;

Parser::Parser() {
    symbol_table.parent = NULL;
    symbol_table.functype = NULL;
    current_symbol_table = &symbol_table;
    indent = loop_cnt = 0;
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
        }
        else
            if (s[0] == '%') {
                s = s.substr(1);
                for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                s = "0b" + s;
            }
            else
                if (s[0] == '&') {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    s = "0" + s;
                }
                else {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    s = "0x" + s;
                }
    else
        if (isdigit(s[0])) {
            for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
        }
        else
            if (s[0] == '%') {
                s = s.substr(1);
                for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                s = "0b" + s;
            }
            else
                if (s[0] == '&') {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    s = "0" + s;
                }
                else {
                    s = s.substr(1);
                    for (; s.length() > 1 && s[0] == '0'; s = s.substr(1));
                    s = "0x" + s;
                }
    res.first = 19;
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
            return std::string("uint8_t");
            break;
        case 1:
            return std::string("unsigned char");
            break;
        case 2:
            return std::string("uint16_t");
            break;
        case 3:
            return std::string("unsigned short");
            break;
        case 4:
            return std::string("uint32_t");
            break;
        case 5:
        case 6:
        case 7:
            return std::string("unsigned int");
            break;
        case 8:
            return std::string("unsigned long");
            break;
        case 9:
            return std::string("uint64_t");
            break;
        case 10:
            return std::string("unsigned long long");
            break;
        case 11:
            return std::string("int8_t");
            break;
        case 12:
            return std::string("char");
            break;
        case 13:
            return std::string("int16_t");
            break;
        case 14:
        case 15:
            return std::string("short");
            break;
        case 16:
            return std::string("int32_t");
            break;
        case 17:
            return std::string("int");
            break;
        case 18:
            return std::string("long");
            break;
        case 19:
            return std::string("int64_t");
            break;
        case 20:
        case 21:
            return std::string("float");
            break;
        case 22:
        case 23:
        case 25:
        case 26:
            return std::string("double");
            break;
        case 24:
            return std::string("long double");
            break;
        case 27:
        case 28:
        case 29:
        case 30:
            return std::string("bool");
            break;
        case 31:
            return std::string("char");
            break;
        case 32:
        case 33:
        case 35:
            return std::string("std::string");
            break;
        case 37:
            return std::string("void");
    }
    return std::string("");
}

std::string Parser::id_with_type(Type *type, std::vector<std::string> id_list, std::string struct_name) {
    std::string res = "";
    if (type == NULL)
        return res;
    switch (type->category) {
        case 0:
            res = pas_basic_type_to_c(type->type_no);
            if (! id_list.empty()) {
                res += " ";
                for (int i = 0; i < id_list.size() - 1; i++)
                    res += id_list[i] + ", ";
                res += id_list.back();
            }
            break;
        case 1:
            for (int i = 0; i < id_list.size(); i++) {
                std::string tmp = "*";
                tmp += id_list[i];
                if (type->pointer_type->category == 2)
                    tmp = "(" + tmp + ")";
                id_list[i] = tmp;
            }
            res = id_with_type(type->pointer_type, id_list);
            if (id_list.empty()) // for set definition
                res += "*";
            break;
        case 2:
            for (int i = 0; i < id_list.size(); i++) {
                std::string tmp = "";
                if (type->array_index_type == -1)
                    tmp = id_list[i] + "[]";
                else
                    if (type->array_index_type == 0)
                        tmp = id_list[i] + "[" + type->array_uprange + " - " + type->array_bias + " + 1]";
                    else
                        tmp = id_list[i] + "['" + type->array_uprange + "' - '" + type->array_bias + "' + 1]";
                id_list[i] = tmp;
            }
            res = id_with_type(type->array_type, id_list);
            break;
        case 3:
            res = "enum {\n";
            indent++;
            for (int i = 0; i < type->enum_list.size(); i++) {
                for (int j = 0; j < indent; j++)
                    res += "\t";
                res += no_token[type->enum_list[i].first];
                if (type->enum_list[i].second != "")
                    res += " = " + type->enum_list[i].second;
                if (i != type->enum_list.size() - 1)
                    res += ",";
                res += "\n";
            }
            indent--;
            for (int i = 0; i < indent; i++)
                res += "\t";
            res += "}";
            if (! id_list.empty()) {
                res += " ";
                for (int i = 0; i < id_list.size() - 1; i++)
                    res += id_list[i] + ", ";
                res += id_list.back();
            }
            break;
        case 4:
            res = "struct ";
            if (struct_name != "")
                res += struct_name + " ";
            res += "{\n";
            indent++;
            for (int i = 0; i < type->record_list.size(); i++) {
                for (int j = 0; j < indent; j++)
                    res += "\t";
                std::vector<std::string> tmp_id_list;
                for (int j = 0; j < type->record_list[i].first.size(); j++)
                    tmp_id_list.push_back(no_token[type->record_list[i].first[j]]);
                res += id_with_type(type->record_list[i].second, tmp_id_list);
                res += ";\n";
            }
            indent--;
            for (int i = 0; i < indent; i++)
                res += "\t";
            res += "}";
            if (! id_list.empty()) {
                res += " ";
                for (int i = 0; i < id_list.size() - 1; i++)
                    res += id_list[i] + ", ";
                res += id_list.back();
            }
            break;
        case 5:
            {
                std::vector<std::string> tmp;
                res = id_with_type(type->set_type, tmp);
            }
            res = "std::set<" + res + ">";
            if (! id_list.empty()) {
                res += " ";
                for (int i = 0; i < id_list.size() - 1; i++)
                    res += id_list[i] + ", ";
                res += id_list.back();
            }
            break;
        case 6:
            res = no_token[type->named_id_no];
            if (! id_list.empty()) {
                res += " ";
                for (int i = 0; i < id_list.size() - 1; i++)
                    res += id_list[i] + ", ";
                res += id_list.back();
            }
            break;
    }
    return res;
}

// 0--exclusive 1--match 2--compatible 3--correspondence

bool Parser::type_match(Type *a, Type *b, int match_type) const {
    if (! a && ! b)
        return true;
    for (; a && a->category == 6; a = a->named_type);
    for (; b && b->category == 6; b = b->named_type);
    if (! a || ! b)
        return false;
    if (a->category != b->category)
        return false;
    if (a->category == 0) {
        if (match_type == 2) {
            if (a->type_no <= 26 && b->type_no <= 26)
                return true;
            if (a->type_no >= 27 && a->type_no <= 30 && b->type_no >= 27 && b->type_no <= 30)
                return true;
            if ((a->type_no == 32 || a->type_no == 33 || a->type_no == 35) && 
                (b->type_no == 32 || b->type_no == 33 || b->type_no == 35))
                return true;
            if (a->type_no == b->type_no)
                return true;
        }
        else
            if (match_type == 3) {
                if (a->type_no <= 19 && b->type_no <= 19)
                    return true;
                if (a->type_no >= 19 && a->type_no <= 26 && b->type_no <= 26)
                    return true;
                if (a->type_no >= 27 && a->type_no <= 30 && b->type_no >= 27 && b->type_no <= 30)
                    return true;
                if ((a->type_no == 32 || a->type_no == 33 || a->type_no == 35) && 
                    (b->type_no == 32 || b->type_no == 33 || b->type_no == 35))
                    return true;
                if (a->type_no == b->type_no)
                    return true;
            }
            else {
                if (a->type_no >= 0 && a->type_no <= 1 && b->type_no >= 0 && b->type_no <= 1)
                    return true;
                if (a->type_no >= 2 && a->type_no <= 3 && b->type_no >= 2 && b->type_no <= 3)
                    return true;
                if (a->type_no >= 4 && a->type_no <= 7 && b->type_no >= 4 && b->type_no <= 7)
                    return true;
                if (a->type_no == 8 && b->type_no == 8)
                    return true;
                if (a->type_no >= 9 && a->type_no <= 10 && b->type_no >= 9 && b->type_no <= 10)
                    return true;
                if (a->type_no >= 13 && a->type_no <= 15 && b->type_no >= 13 && b->type_no <= 15)
                    return true;
                if (a->type_no >= 16 && a->type_no <= 17 && b->type_no >= 16 && b->type_no <= 17)
                    return true;
                if (a->type_no == 18 && b->type_no == 18)
                    return true;
                if (a->type_no == 19 && b->type_no == 19)
                    return true;
                if (match_type == 1) {
                    if (a->type_no >= 11 && a->type_no <= 12 && b->type_no >= 11 && b->type_no <= 12)
                        return true;
                    if (a->type_no == b->type_no)
                        return true;
                }
                else {
                    if ((a->type_no == 11 || a->type_no == 12 || a->type_no == 31) && (b->type_no == 11 || b->type_no == 12 || b->type_no == 31))
                        return true;
                    if (a->type_no >= 20 && a->type_no <= 21)
                        return true;
                    if ((a->type_no == 22 || a->type_no == 23 || a->type_no == 25 || a->type_no == 26) && (b->type_no == 22 || b->type_no == 23 || b->type_no == 25 || b->type_no == 26))
                        return true;
                    if (a->type_no == 24 && b->type_no == 24)
                        return true;
                    if (a->type_no >= 27 && a->type_no <= 30 && b->type_no >= 27 && b->type_no <= 30)
                        return true;
                    if ((a->type_no == 32 || a->type_no == 33 || a->type_no == 35) && (b->type_no == 32 || b->type_no == 33 || b->type_no == 35))
                        return true;
                    if ((a->type_no == 34 || a->type_no == 36) && (b->type_no == 34 || b->type_no == 36))
                        return true;
                }
            }
        return false;
    }
    if (a->category == 1)
        return type_match(a->pointer_type, b->pointer_type, 1);
    if (a->category == 2)
        if (! match_type)
            return type_match(a->array_type, b->array_type, 1);
        else {
            if (match_type == 1) {
                if (a->array_index_type != b->array_index_type)
                    return false;
            }
            else {
                if (a->array_index_type == -1)
                    return true;
                if (a->array_index_type != b->array_index_type)
                    return false;
            }
            if (a->array_index_type == 0) {
                if (std::stoi(a->array_bias, 0, 0) != std::stoi(b->array_bias, 0, 0))
                    return false;
                if (std::stoi(a->array_uprange, 0, 0) != std::stoi(b->array_uprange, 0, 0))
                    return false;
            }
            if (a->array_index_type == 1) {
                char tmp[10];
                sprintf(tmp, a->array_bias.c_str());
                int tmpa = tmp[0];
                sprintf(tmp, b->array_bias.c_str());
                int tmpb = tmp[0];
                if (tmpa != tmpb)
                    return false;
                sprintf(tmp, a->array_uprange.c_str());
                tmpa = tmp[0];
                sprintf(tmp, b->array_uprange.c_str());
                tmpb = tmp[0];
                if (tmpa != tmpb)
                    return false;
            }
            return type_match(a->array_type, b->array_type, match_type);
        }
    if (a->category == 3 || a->category == 4)
        return a == b;
    if (a->category == 5)
        return type_match(a->set_type, b->set_type, match_type);
    return false;
}

bool Parser::functype_match(FuncType *a, FuncType *b, int match_type) const {
    if (a->id_no != b->id_no)
        return false;
    if (match_type && ! type_match(a->ret_type, b->ret_type, 1))
        return false;
    if (a->param_list.size() != b->param_list.size())
        return false;
    for (int i = 0; i < a->param_list.size(); i++) {
        if (match_type && a->param_list[i].first != b->param_list[i].first)
            return false;
        if (match_type && a->param_list[i].second.first != b->param_list[i].second.first)
            return false;
        if (! type_match(a->param_list[i].second.second, b->param_list[i].second.second, match_type))
            return false;
    }
    return true;
}

Type* Parser::arithmetic_type_check(Type *a, Type *b) {
    if (a->type_no <= 19 && b->type_no >= 20)
        return b;
    if (a->type_no >= 20 && b->type_no <= 19)
        return a;
    if (a->type_no >= 20) {
        if (a->type_no == 20 && b->type_no == 20)
            return a;
        else
            if (a->type_no != 20)
                return a;
            else
                return b;
    }
    if (a->type_no > b->type_no)
        std::swap(a, b);
    if (a->type_no >= 0 && a->type_no <= 1) {
        if (b->type_no == 0 || b->type_no == 1 || b->type_no == 11 || b->type_no == 12)
            return a;
        if (b->type_no >= 2 && b->type_no <= 10)
            return b;
        Type *ret_type = new Type;
        ret_type->category = 0;
        if (b->type_no >= 13 && b->type_no <= 15)
            ret_type->type_no = 3;
        if (b->type_no >= 16 && b->type_no <= 17)
            ret_type->type_no = 5;
        if (b->type_no == 18)
            ret_type->type_no = 8;
        if (b->type_no == 19)
            ret_type->type_no = 9;
        return ret_type;
    }
    if (a->type_no >= 2 && a->type_no <= 3) {
        if (b->type_no <= 3 || (b->type_no >= 11 && b->type_no <= 15))
            return a;
        if (b->type_no >= 3 && b->type_no <= 10)
            return b;
        Type *ret_type = new Type;
        ret_type->category = 0;
        if (b->type_no >= 16 && b->type_no <= 17)
            ret_type->type_no = 5;
        if (b->type_no == 18)
            ret_type->type_no = 8;
        if (b->type_no == 19)
            ret_type->type_no = 9;
        return ret_type;
    }
    if (a->type_no >= 4 && a->type_no <= 7) {
        if (b->type_no <= 7 || (b->type_no >= 11 && b->type_no <= 17))
            return a;
        if (b->type_no >= 7 && b->type_no <= 10)
            return b;
        Type *ret_type = new Type;
        ret_type->category = 0;
        if (b->type_no == 18)
            ret_type->type_no = 8;
        if (b->type_no == 19)
            ret_type->type_no = 9;
        return ret_type;
    }
    if (a->type_no == 8) {
        if (b->type_no <= 8 || (b->type_no >= 11 && b->type_no <= 18))
            return a;
        if (b->type_no >= 8 && b->type_no <= 10)
            return b;
        Type *ret_type = new Type;
        ret_type->category = 0;
        ret_type->type_no = 9;
        return ret_type;
    }
    if (a->type_no == 9 || a->type_no == 10)
        return a;
    return b;
}

bool Parser::process_default(Token &new_token) {
    return true;
}

bool Parser::process_newline(Token &new_token) {
    result << "\n";
    return true;
}

bool Parser::process_semicolon_newline(Token &new_token) {
    result << ";\n";
    return true;
}

bool Parser::process_program(Token &new_token) {
    bool flag = true;
    for (int i = 0; i < current_symbol_table->subtable.size(); i++)
        for (int j = 0; j < current_symbol_table->subtable[i].size(); j++)
            if (! current_symbol_table->subtable[i][j]->functype->defined) {
                output_error(current_symbol_table->subtable[i][j]->functype->line, current_symbol_table->subtable[i][j]->functype->col, current_symbol_table->subtable[i][j]->functype->pos, "procedure/function not defined");
                flag = false;
            }
    return flag;
}

bool Parser::process_M1(Token &new_token) {
    result << "#include <bits/stdc++.h>\n\n";
    return true;
}

bool Parser::process_label(Token &new_token) {
    std::string label = parsing_stack.back().second.content;
    for (int i = 0; i < label.length(); i++)
        if (! isdigit(label[i])) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "label syntax error");
            return false;
        }
    current_symbol_table->labels[label] = label_cnt++;
    return true;
}

bool Parser::process_int_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already been defined");
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
    current_symbol_table->is_implicit[id_no] = true;
    current_symbol_table->const_val[id_no] = parsing_stack[parsing_stack.size() - 2].second.content;
    return true;
}

bool Parser::process_float_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already been defined");
        return false;
    }
    std::string constant = parsing_stack[parsing_stack.size() - 2].second.content;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "const double " << no_token[id_no] << " = ";
    result << constant << ";\n";
    current_symbol_table->symbols[id_no] = parsing_stack[parsing_stack.size() - 2].second.type;
    current_symbol_table->is_const[id_no] = true;
    current_symbol_table->is_implicit[id_no] = true;
    current_symbol_table->const_val[id_no] = parsing_stack[parsing_stack.size() - 2].second.content;
    return true;
}

bool Parser::process_string_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already been defined");
        return false;
    }
    for (int i = 0; i < indent; i++)
        result << "\t";
    if (parsing_stack[parsing_stack.size() - 2].second.type->type_no == 31)
        result << "const char " << no_token[id_no] << " = '" << parsing_stack[parsing_stack.size() - 2].second.content << "';\n";
    else
        result << "const std::string " << no_token[id_no] << " = \"" << parsing_stack[parsing_stack.size() - 2].second.content << "\";\n";
    current_symbol_table->symbols[id_no] = parsing_stack[parsing_stack.size() - 2].second.type;
    current_symbol_table->is_const[id_no] = true;
    current_symbol_table->is_implicit[id_no] = true;
    current_symbol_table->const_val[id_no] = parsing_stack[parsing_stack.size() - 2].second.content;
    return true;
}

bool Parser::process_bool_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    if (current_symbol_table->defined(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "identifier has already been defined");
        return false;
    }
    Type *type = new Type;
    type->category = 0;
    type->type_no = 27;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "const bool " << no_token[id_no] << " = ";
    if (parsing_stack[parsing_stack.size() - 2].second.no == 45) {
        result << "true;\n";
        current_symbol_table->const_val[id_no] = "true";
    }
    else {
        result << "false;\n";
        current_symbol_table->const_val[id_no] = "false";
    }
    current_symbol_table->symbols[id_no] = type;
    current_symbol_table->is_const[id_no] = true;
    current_symbol_table->is_implicit[id_no] = true;
    return true;
}

bool Parser::process_typed_constant_def(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 5].second.id_no;
    Type *type = current_symbol_table->symbols[id_no];
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    current_symbol_table->is_implicit[id_no] = false;
    result << ";\n";
    return true;
}

bool Parser::process_type_def(Token &new_token) {
    if (! parsing_stack[parsing_stack.size() - 4].second.type) {
        if (parsing_stack[parsing_stack.size() - 2].second.type)
            delete parsing_stack[parsing_stack.size() - 2].second.type;
        return false;
    }
    int id_no = parsing_stack[parsing_stack.size() - 4].second.type->named_id_no;
    Type *type = current_symbol_table->named_types[id_no];
    type->named_type = parsing_stack[parsing_stack.size() - 2].second.type;
    if (! type->named_type)
        return false;
    std::vector<std::string> tmp;
    for (int i = 0; i < indent; i++)
        result << "\t";
    if (parsing_stack[parsing_stack.size() - 2].second.type->category != 4) {
        result << "typedef ";
        tmp.push_back(no_token[id_no]);
        result << id_with_type(parsing_stack[parsing_stack.size() - 2].second.type, tmp);
        result << ";\n";
    }
    else {
        result << id_with_type(parsing_stack[parsing_stack.size() - 2].second.type, tmp, no_token[id_no]);
        result << ";\n";
    }
    return true;
}

bool Parser::process_type_name(Token &new_token) {
    int id_no = parsing_stack.back().second.no;
    if (current_symbol_table->defined(id_no)) {
        new_token.type = NULL;
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier has already been defined");
        return false;
    }
    Type *type = new Type;
    type->category = 6;
    type->named_id_no = id_no;
    type->named_type = NULL;
    current_symbol_table->named_types[id_no] = type;
    new_token.type = type;
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
        new_token.type = NULL;
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
        new_token.type->pointer_type->type_no = 37;
        return true;
    }
    if (id_no == 34 || id_no == 36) {
        new_token.type->category = 1;
        new_token.type->pointer_type = new Type;
        new_token.type->pointer_type->category = 0;
        new_token.type->pointer_type->type_no = 33;
        return true;
    }
    new_token.type->category = 0;
    if (id_no >= 6 && id_no <= 7)
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
                        if (id_no == 32 || id_no == 35)
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
    if (p) {
        for (; p->array_type; p = p->array_type);
        p->array_type = parsing_stack.back().second.type;
    }
    else
        return false;
    return true;
}

bool Parser::process_enum_type_denoter(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (int i = 0; i < new_token.type->enum_list.size(); i++)
        current_symbol_table->enum_items[new_token.type->enum_list[i].first] = new_token.type;
    return true;
}

bool Parser::process_record_type_denoter(Token &new_token) {
    if (parsing_stack[parsing_stack.size() - 2].second.category == -1)
        new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    else
        new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    return true;
}

bool Parser::process_set_type_denoter(Token &new_token) {
    if (! parsing_stack.back().second.type->can_be_defined_in_set()) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "illegal type declaration of set elements");
        new_token.type = NULL;
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 5;
    new_token.type->set_type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_array_single_subrange_list(Token &new_token) {
    new_token.type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_array_subrange_list(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    if (new_token.type)
        new_token.type->array_type = parsing_stack.back().second.type;
    else
        return false;
    return true;
}

bool Parser::process_array_subrange(Token &new_token) {
    new_token.type = NULL;
    if (! parsing_stack[parsing_stack.size() - 3].second.type || ! parsing_stack.back().second.type)
        return false;
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
        new_token.type->array_type = NULL;
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
        new_token.type->array_type = NULL;
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
    new_token.is_const = true;
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
    new_token.is_const = true;
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
    if (content.first == 1)
        type->type_no = 31;
    else
        type->type_no = 33;
    new_token.type = type;
    new_token.content = content.second;
    new_token.str_len = content.first;
    new_token.is_const = true;
    return true;
}

bool Parser::process_array_subrange_case_index(Token &new_token) {
    if (parsing_stack.back().second.category == 2) {
        int id_no = parsing_stack.back().second.no;
        bool flag = false;
        bool constant_flag = false;
        new_token.type = NULL;
        for (SymbolTable *p = current_symbol_table; p; p = p->parent)
            if (p->defined(id_no)) {
                if (p->is_const.count(id_no)) {
                    constant_flag = true;
                    Type *type = p->symbols[id_no];
                    for (; type && type->category == 6; type = type->named_type);
                    if (! type)
                        return false;
                    if (type->category == 0 && (type->type_no <= 19 || type->type_no == 31) && p->is_implicit[id_no]) {
                        new_token.type = p->symbols[id_no];
                        new_token.content = p->const_val[id_no];
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
                    output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
            return false;
        }
        return true;
    }
    else {
        if (parsing_stack.back().second.category == -1 && parsing_stack.back().second.no == 21) {
            if (parsing_stack.back().second.str_len != 1) {
                output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
                new_token.type = NULL;
                return false;
            }
        }
        new_token.type = parsing_stack.back().second.type;
        new_token.content = parsing_stack.back().second.content;
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
        if (current_symbol_table->defined(parsing_stack.back().second.no)) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier has already been defined");
            return false;
        }
        enum_item = std::make_pair(parsing_stack.back().second.no, "");
    }
    else {
        if (current_symbol_table->defined(parsing_stack[parsing_stack.size() - 3].second.no)) {
            output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "identifier has already been defined");
            return false;
        }
        enum_item = std::make_pair(parsing_stack[parsing_stack.size() - 3].second.no, parsing_stack.back().second.content);
    }
    new_token.type->enum_list.push_back(enum_item);
    return true;
}

bool Parser::process_single_field_list(Token &new_token) {
    new_token.type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_field_list(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    bool flag = true;
    for (int i = 0; i < new_token.type->record_list.size(); i++)
        for (int j = 0; j < new_token.type->record_list[i].first.size(); j++)
            for (int k = 0; k < parsing_stack.back().second.type->record_list[0].first.size(); k++)
                if (new_token.type->record_list[i].first[j] == parsing_stack.back().second.type->record_list[0].first[k]) {
                    output_error(parsing_stack.back().second.id_line[k], parsing_stack.back().second.id_col[k], parsing_stack.back().second.id_pos[k], "duplicate identifier");
                    flag = false;
                }
    new_token.type->record_list.push_back(parsing_stack.back().second.type->record_list[0]);
    delete parsing_stack.back().second.type;
    if (! flag)
        return false;
    return true;
}

bool Parser::process_record_section(Token &new_token) {
    new_token.type = new Type;
    new_token.type->category = 4;
    new_token.type->record_list.push_back(std::make_pair(parsing_stack[parsing_stack.size() - 3].second.id_list, parsing_stack.back().second.type));
    new_token.id_line = parsing_stack[parsing_stack.size() - 3].second.id_line;
    new_token.id_col = parsing_stack[parsing_stack.size() - 3].second.id_col;
    new_token.id_pos = parsing_stack[parsing_stack.size() - 3].second.id_pos;
    return true;
}

bool Parser::process_single_id_list(Token &new_token) {
    new_token.id_list.push_back(parsing_stack.back().second.no);
    new_token.id_line.push_back(parsing_stack.back().second.line);
    new_token.id_col.push_back(parsing_stack.back().second.col);
    new_token.id_pos.push_back(parsing_stack.back().second.pos);
    return true;
}

bool Parser::process_id_list(Token &new_token) {
    new_token.id_list = parsing_stack[parsing_stack.size() - 3].second.id_list;
    new_token.id_line = parsing_stack[parsing_stack.size() - 3].second.id_line;
    new_token.id_col = parsing_stack[parsing_stack.size() - 3].second.id_col;
    new_token.id_pos = parsing_stack[parsing_stack.size() - 3].second.id_pos;
    bool flag = true;
    for (int i = 0; i < new_token.id_list.size(); i++)
        if (new_token.id_list[i] == parsing_stack.back().second.no) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "duplicate identifier");
            flag = false;
        }
    new_token.id_list.push_back(parsing_stack.back().second.no);
    new_token.id_line.push_back(parsing_stack.back().second.line);
    new_token.id_col.push_back(parsing_stack.back().second.col);
    new_token.id_pos.push_back(parsing_stack.back().second.pos);
    if (! flag)
        return false;
    return true;
}

bool Parser::process_const_type_declar(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 3].second.no;
    if (current_symbol_table->defined(id_no)) {
        new_token.type = NULL;
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "identifier has already been defined");
        return false;
    }
    current_symbol_table->symbols[id_no] = parsing_stack.back().second.type;
    current_symbol_table->is_const[id_no] = true;
    new_token.type = parsing_stack.back().second.type;
    std::vector<std::string> tmp;
    tmp.push_back(no_token[id_no]);
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "const " << id_with_type(new_token.type, tmp) << " = ";
    new_token.id_no = id_no;
    return true;
}

bool Parser::process_M2(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (; new_token.type && new_token.type->category == 6; new_token.type = new_token.type->named_type);
    return true;
}

bool Parser::process_num_const_val(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    if (! type)
        return false;
    if (! (type->category == 0 && type->type_no >= 0 && type->type_no <= 26)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    result << parsing_stack.back().second.content;
    new_token.content = parsing_stack.back().second.content;
    return true;
}

bool Parser::process_string_const_val(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    if (! type)
        return false;
    if (! (type->category == 0 && type->type_no >= 31 && type->type_no <= 36)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (type->type_no == 31) {
        if (parsing_stack.back().second.str_len > 1) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
            return false;
        }
        result << "'" << parsing_stack.back().second.content << "'";
        return true;
    }
    result << "\"" << parsing_stack.back().second.content << "\"";
    new_token.content = parsing_stack.back().second.content;
    return true;
}

bool Parser::process_bool_const_val(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    if (! type)
        return false;
    if (! (type->category == 0 && type->type_no >= 27 && type->type_no <= 30)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (parsing_stack.back().second.no == 45)
        result << "true";
    else
        result << "false";
    return true;
}

bool Parser::process_array_const_val(Token &new_token) {
    result << "}";
    return true;
}

bool Parser::process_record_const_val(Token &new_token) {
    result << "\n";
    indent--;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "}";
    return true;
}

bool Parser::process_M3(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    if (! type) {
        new_token.type = NULL;
        return false;
    }
    if (type->category != 2) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        new_token.type = NULL;
        return false;
    }
    new_token.type = type->array_type;
    for (; new_token.type && new_token.type->category == 6; new_token.type = new_token.type->named_type);
    if (type->array_index_type == 0) {
        int l = std::stoi(type->array_bias, 0, 0), r = std::stoi(type->array_uprange, 0, 0);
        new_token.array_len = r - l + 1;
    }
    else {
        char tmp[5];
        sprintf(tmp, type->array_bias.c_str());
        int l = tmp[0];
        sprintf(tmp, type->array_uprange.c_str());
        int r = tmp[0];
        new_token.array_len = r - l + 1;
    }
    result << "{";
    return true;
}

bool Parser::process_single_array_vals(Token &new_token) {
    if (parsing_stack[parsing_stack.size() - 2].second.array_len <= 0) {
        if (parsing_stack[parsing_stack.size() - 2].second.array_len == 0) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "too many initializers for array");
            parsing_stack[parsing_stack.size() - 2].second.array_len--;
        }
        return false;
    }
    parsing_stack[parsing_stack.size() - 2].second.array_len--;
    return true;
}

bool Parser::process_array_vals(Token &new_token) {
    if (parsing_stack[parsing_stack.size() - 4].second.array_len <= 0) {
        if (parsing_stack[parsing_stack.size() - 4].second.array_len == 0) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "too many initializers for array");
            parsing_stack[parsing_stack.size() - 2].second.array_len--;
        }
        return false;
    }
    parsing_stack[parsing_stack.size() - 4].second.array_len--;
    return true;
}

bool Parser::process_array_element_split(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    result << ", ";
    return true;
}

bool Parser::process_M4(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    if (! type) {
        new_token.type = NULL;
        return false;
    }
    if (type->category != 4) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        new_token.type = NULL;
        return false;
    }
    new_token.type = type;
    new_token.record_defined_ids.clear();
    result << "{\n";
    indent++;
    return true;
}

bool Parser::process_single_record_field_val_split(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 2].second.no;
    new_token.type = NULL;
    if (parsing_stack[parsing_stack.size() - 3].second.record_defined_ids.count(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "duplicate identifier");
        return false;
    }
    Type *record_type = parsing_stack[parsing_stack.size() - 3].second.type;
    for (int i = 0; ! new_token.type && i < record_type->record_list.size(); i++)
        for (int j = 0; j < record_type->record_list[i].first.size(); j++)
            if (record_type->record_list[i].first[j] == id_no) {
                new_token.type = record_type->record_list[i].second;
                break;
            }
    if (! new_token.type) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "no such member");
        return false;
    }
    parsing_stack[parsing_stack.size() - 3].second.record_defined_ids.insert(id_no);
    for (; new_token.type && new_token.type->category == 6; new_token.type = new_token.type->named_type);
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << no_token[id_no] << " : ";
    return true;
}

bool Parser::process_record_field_val_split(Token &new_token) {
    int id_no = parsing_stack[parsing_stack.size() - 2].second.no;
    new_token.type = NULL;
    if (parsing_stack[parsing_stack.size() - 5].second.record_defined_ids.count(id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "duplicate identifier");
        return false;
    }
    Type *record_type = parsing_stack[parsing_stack.size() - 5].second.type;
    for (int i = 0; ! new_token.type && i < record_type->record_list.size(); i++)
        for (int j = 0; j < record_type->record_list[i].first.size(); j++)
            if (record_type->record_list[i].first[j] == id_no) {
                new_token.type = record_type->record_list[i].second;
                break;
            }
    if (! new_token.type) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "no such member");
        return false;
    }
    parsing_stack[parsing_stack.size() - 5].second.record_defined_ids.insert(id_no);
    for (; new_token.type && new_token.type->category == 6; new_token.type = new_token.type->named_type);
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << no_token[id_no] << " : ";
    return true;
}

bool Parser::process_record_field_split(Token &new_token) {
    result << ",\n";
    return true;
}

bool Parser::process_var_type_declar(Token &new_token) {
    bool flag = true;
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++) {
        if (current_symbol_table->defined(parsing_stack[parsing_stack.size() - 3].second.id_list[i])) {
            new_token.type = NULL;
            output_error(parsing_stack[parsing_stack.size() - 3].second.id_line[i], parsing_stack[parsing_stack.size() - 3].second.id_col[i], parsing_stack[parsing_stack.size() - 3].second.id_pos[i], "identifier has already been defined");
            flag = false;
        }
        if (flag)
            current_symbol_table->symbols[parsing_stack[parsing_stack.size() - 3].second.id_list[i]] = parsing_stack.back().second.type;
    }
    if (! flag)
        return false;
    new_token.type = parsing_stack.back().second.type;
    new_token.id_num = parsing_stack[parsing_stack.size() - 3].second.id_list.size();
    std::vector<std::string> tmp;
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++)
        tmp.push_back(no_token[parsing_stack[parsing_stack.size() - 3].second.id_list[i]]);
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << id_with_type(new_token.type, tmp);
    return true;
}

bool Parser::process_M5(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (; new_token.type && new_token.type->category == 6; new_token.type = new_token.type->named_type);
    if (parsing_stack[parsing_stack.size() - 2].second.id_num != 1) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "too many variables");
        return false;
    }
    result << " = ";
    return true;
}

bool Parser::process_proc_func_declar(Token &new_token) {
    SymbolTable *p = current_symbol_table;
    current_symbol_table = current_symbol_table->parent;
    if (current_symbol_table->defined_except_func(p->functype->id_no)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "identifier has already been defined");
        return false;
    }
    for (int i = 0; i < current_symbol_table->subtable[p->functype->id_no].size(); i++)
        if (functype_match(p->functype, current_symbol_table->subtable[p->functype->id_no][i]->functype, 0)) {
            output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "duplicate procedure/function identifier");
            return false;
        }
    current_symbol_table->subtable[p->functype->id_no].push_back(p);
    result << ";\n\n";
    return true;
}

bool Parser::process_proc_func_def(Token &new_token) {
    current_symbol_table = current_symbol_table->parent;
    result << "\n";
    return true;
}

bool Parser::process_procedure_heading(Token &new_token) {
    current_symbol_table->functype->ret_type = NULL;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "void ";
    result << no_token[current_symbol_table->functype->id_no];
    result << "(";
    if (! current_symbol_table->functype->param_list.empty()) {
        for (int i = 0; i < current_symbol_table->functype->param_list.size(); i++) {
            if (current_symbol_table->functype->param_list[i].second.first == 2)
                result << "const ";
            std::vector<std::string> tmp;
            tmp.push_back(no_token[current_symbol_table->functype->param_list[i].first]);
            if (current_symbol_table->functype->param_list[i].second.first == 1)
                tmp[0] = "&" + tmp[0];
            result << id_with_type(current_symbol_table->functype->param_list[i].second.second, tmp);
            if (i != current_symbol_table->functype->param_list.size() - 1)
                result << ", ";
        }
    }
    result << ")";
    return true;
}

bool Parser::process_function_heading(Token &new_token) {
    current_symbol_table->functype->ret_type = parsing_stack[parsing_stack.size() - 2].second.type;
    current_symbol_table->symbols[current_symbol_table->functype->id_no] = parsing_stack[parsing_stack.size() - 2].second.type;
    Type *type = current_symbol_table->functype->ret_type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category == 2) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "array return type not supported");
        return false;
    }
    for (int i = 0; i < indent; i++)
        result << "\t";
    std::vector<std::string> tmp;
    tmp.push_back(no_token[current_symbol_table->functype->id_no]);
    result << id_with_type(current_symbol_table->functype->ret_type, tmp);
    result << "(";
    if (! current_symbol_table->functype->param_list.empty()) {
        for (int i = 0; i < current_symbol_table->functype->param_list.size(); i++) {
            if (current_symbol_table->functype->param_list[i].second.first == 2)
                result << "const ";
            std::vector<std::string> tmp;
            tmp.push_back(no_token[current_symbol_table->functype->param_list[i].first]);
            if (current_symbol_table->functype->param_list[i].second.first == 1)
                tmp[0] = "&" + tmp[0];
            result << id_with_type(current_symbol_table->functype->param_list[i].second.second, tmp);
            if (i != current_symbol_table->functype->param_list.size() - 1)
                result << ", ";
        }
    }
    result << ")";
    return true;
}

bool Parser::process_formal_parameter_spec(Token &new_token) {
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++) {
        if (current_symbol_table->defined(parsing_stack[parsing_stack.size() - 3].second.id_list[i])) {
            output_error(parsing_stack[parsing_stack.size() - 3].second.id_line[i], parsing_stack[parsing_stack.size() - 3].second.id_col[i], parsing_stack[parsing_stack.size() - 3].second.id_pos[i], "duplicate identifier");
            return false;
        }
        current_symbol_table->symbols[parsing_stack[parsing_stack.size() - 3].second.id_list[i]] = parsing_stack.back().second.type;
    }
    Type *p = parsing_stack.back().second.type;
    for (; p && p->category == 6; p = p->named_type);
    if (p && p->category == 2) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "val parameters of array type is not supported in C++");
        return false;
    }
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++)
        current_symbol_table->functype->param_list.push_back(std::make_pair(parsing_stack[parsing_stack.size() - 3].second.id_list[i], std::make_pair(0, parsing_stack.back().second.type)));
    return true;
}

bool Parser::process_formal_parameter_spec_var(Token &new_token) {
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++) {
        if (current_symbol_table->defined(parsing_stack[parsing_stack.size() - 3].second.id_list[i])) {
            output_error(parsing_stack[parsing_stack.size() - 3].second.id_line[i], parsing_stack[parsing_stack.size() - 3].second.id_col[i], parsing_stack[parsing_stack.size() - 3].second.id_pos[i], "duplicate identifier");
            return false;
        }
        current_symbol_table->symbols[parsing_stack[parsing_stack.size() - 3].second.id_list[i]] = parsing_stack.back().second.type;
    }
    Type *p = parsing_stack.back().second.type;
    for (; p && p->category == 6; p = p->named_type);
    if (p && p->category == 2)
        for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++)
            current_symbol_table->functype->param_list.push_back(std::make_pair(parsing_stack[parsing_stack.size() - 3].second.id_list[i], std::make_pair(0, parsing_stack.back().second.type)));
    else
        for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++)
            current_symbol_table->functype->param_list.push_back(std::make_pair(parsing_stack[parsing_stack.size() - 3].second.id_list[i], std::make_pair(1, parsing_stack.back().second.type)));
    return true;
}

bool Parser::process_formal_parameter_spec_const(Token &new_token) {
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++) {
        if (current_symbol_table->defined(parsing_stack[parsing_stack.size() - 3].second.id_list[i])) {
            output_error(parsing_stack[parsing_stack.size() - 3].second.id_line[i], parsing_stack[parsing_stack.size() - 3].second.id_col[i], parsing_stack[parsing_stack.size() - 3].second.id_pos[i], "duplicate identifier");
            return false;
        }
        current_symbol_table->symbols[parsing_stack[parsing_stack.size() - 3].second.id_list[i]] = parsing_stack.back().second.type;
        current_symbol_table->is_const[parsing_stack[parsing_stack.size() - 3].second.id_list[i]] = true;
    }
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 3].second.id_list.size(); i++)
        current_symbol_table->functype->param_list.push_back(std::make_pair(parsing_stack[parsing_stack.size() - 3].second.id_list[i], std::make_pair(2, parsing_stack.back().second.type)));
    return true;
}

bool Parser::process_array_type_identifier(Token &new_token) {
    new_token.type = new Type;
    new_token.type->category = 2;
    new_token.type->array_index_type = -1;
    new_token.type->array_type = parsing_stack.back().second.type;
    return true;
}

bool Parser::process_M6(Token &new_token) {
    int id_no = parsing_stack.back().second.no;
    SymbolTable *new_symbol_table = new SymbolTable;
    new_symbol_table->parent = current_symbol_table;
    new_symbol_table->functype = new FuncType;
    new_symbol_table->functype->id_no = id_no;
    new_symbol_table->functype->defined = false;
    new_symbol_table->functype->line = parsing_stack[parsing_stack.size() - 2].second.line;
    new_symbol_table->functype->col = parsing_stack[parsing_stack.size() - 2].second.col;
    new_symbol_table->functype->pos = parsing_stack[parsing_stack.size() - 2].second.pos;
    new_symbol_table->symbols[id_no] = NULL;
    current_symbol_table = new_symbol_table;
    return true;
}

bool Parser::process_proc_func_block(Token &new_token) {
    if (current_symbol_table->functype->ret_type) {
        for (int i = 0; i < indent; i++)
            result << "\t";
        result << "return " << no_token[current_symbol_table->functype->id_no] << "_ret;\n";
    }
    indent--;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "}\n";
    return true;
}

bool Parser::process_M7(Token &new_token) {
    SymbolTable *p = current_symbol_table->parent;
    if (p->defined_except_func(current_symbol_table->functype->id_no)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier has already been defined");
        return false;
    }
    current_symbol_table->functype->defined = true;
    bool flag = false;
    for (int i = 0; i < p->subtable[current_symbol_table->functype->id_no].size(); i++)
        if (functype_match(current_symbol_table->functype, p->subtable[current_symbol_table->functype->id_no][i]->functype, 0)) {
            if (! functype_match(current_symbol_table->functype, p->subtable[current_symbol_table->functype->id_no][i]->functype, 1)) {
                output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "duplicate procedure/function identifier");
                return false;
            }
            if (p->subtable[current_symbol_table->functype->id_no][i]->functype->defined) {
                output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "procedure/function has already been defined");
                return false;
            }
            flag = true;
            delete p->subtable[current_symbol_table->functype->id_no][i];
            p->subtable[current_symbol_table->functype->id_no][i] = current_symbol_table;
        }
    if (! flag)
        p->subtable[current_symbol_table->functype->id_no].push_back(current_symbol_table);
    result << " {\n";
    indent++;
    if (current_symbol_table->functype->ret_type) {
        for (int i = 0; i < indent; i++)
            result << "\t";
        std::vector<std::string> tmp;
        std::string tmp_str = no_token[current_symbol_table->functype->id_no] + "_ret";
        tmp.push_back(tmp_str);
        result << id_with_type(current_symbol_table->functype->ret_type, tmp);
        result << ";\n";
    }
    return true;
}

bool Parser::process_program_statement(Token &new_token) {
    indent--;
    result << "}\n";
    return true;
}

bool Parser::process_M8(Token &new_token) {
    result << "int main() {\n";
    indent++;
    return true;
}

bool Parser::process_label_part(Token &new_token) {
    std::string label = parsing_stack[parsing_stack.size() - 2].second.content;
    for (int i = 0; i < label.length(); i++)
        if (! isdigit(label[i])) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "label syntax error");
            return false;
        }
    int label_no = -1;
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->labels.count(label)) {
            if (p->label_defined[label]) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "label has already been defined");
                return false;
            }
            p->label_defined[label] = true;
            label_no = p->labels[label];
            break;
        }
    if (label_no == -1) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "label has not been declared");
        return false;
    }
    result << "label_" << label_no << ":\n";
    return true;
}

bool Parser::process_empty_statement(Token &new_token) {
    if (parsing_stack.size() - 2 >= 0 && parsing_stack[parsing_stack.size() - 2].second.category == -1 && parsing_stack[parsing_stack.size() - 2].second.no == 73) {
        result << ";\n";
    }
    return true;
}

bool Parser::process_assign_statement(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 3].second.type, *val_type = parsing_stack.back().second.type;
    for (; type && type->category == 6; type = type->named_type);
    for (; val_type && val_type->category == 6; val_type = val_type->named_type);
    if (! type || ! val_type)
        return false;
    if (type->category == 2) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "invalid array assignment");
        return false;
    }
    if (type->category == 1 && parsing_stack.back().second.pointer_const) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 3].second.type, parsing_stack.back().second.type, 2)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (parsing_stack[parsing_stack.size() - 3].second.is_const) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "assignment of constant");
        return false;
    }
    result << parsing_stack[parsing_stack.size() - 3].second.content << " = " << parsing_stack.back().second.content << ";\n";
    return true;
}

bool Parser::process_no_param_proc_func_statement(Token &new_token) {
    int id_no = parsing_stack.back().second.no;
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->defined(id_no))
            if (p->subtable.count(id_no)) {
                bool flag = false;
                for (int i = 0; i < p->subtable[id_no].size(); i++)
                    if (p->subtable[id_no][i]->functype->param_list.empty())
                        flag = true;
                if (! flag) {
                    output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "procedure/function not declared");
                    return false;
                }
            }
            else {
                output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier type is not allowed");
                return false;
            }
    if (! new_token.type) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "procedure/function not declared");
        return false;
    }
    result << no_token[id_no] << "();\n";
    return true;
}

bool Parser::process_proc_func_statement(Token &new_token) {
    bool flag = false;
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
        Type *type = parsing_stack[parsing_stack.size() - 2].second.expr_type[i];
        for (; type && type->category == 6; type = type->named_type);
        if (! type)
            return false;
        if (type->category == 1 && parsing_stack[parsing_stack.size() - 2].second.expr_pointer_const[i]) {
            output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "using constant pointer as parameter not supported");
            return false;
        }
    }
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->subtable.count(id_no)) {
            for (int i = 0; i < p->subtable[id_no].size(); i++)
                if (p->subtable[id_no][i]->functype->param_list.size() == parsing_stack[parsing_stack.size() - 2].second.expr_type.size()) {
                    bool match = true, completely_match = true;
                    for (int j = 0; j < p->subtable[id_no][i]->functype->param_list.size(); j++)
                        if (! (p->subtable[id_no][i]->functype->param_list[j].second.first == 1 && parsing_stack[parsing_stack.size() - 2].second.expr_const[j])) {
                            if (p->subtable[id_no][i]->functype->param_list[j].second.first == 1) {
                                if (! type_match(p->subtable[id_no][i]->functype->param_list[j].second.second, parsing_stack[parsing_stack.size() - 2].second.expr_type[j], 1)) {
                                    match = completely_match = false;
                                    break;
                                }
                            }
                            else
                                if (! type_match(p->subtable[id_no][i]->functype->param_list[j].second.second, parsing_stack[parsing_stack.size() - 2].second.expr_type[j], 2)) {
                                    match = completely_match = false;
                                    break;
                                }
                                else
                                    if (! type_match(p->subtable[id_no][i]->functype->param_list[j].second.second, parsing_stack[parsing_stack.size() - 2].second.expr_type[j], 1))
                                        completely_match = false;
                        }
                        else {
                            match = completely_match = false;
                            break;
                        }
                    if ((! flag && match) || completely_match)
                        flag = true;
                }
            if (flag)
                break;
        }
    if (! flag) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
        return false;
    }
    result << no_token[id_no] + "(" + parsing_stack[parsing_stack.size() - 2].second.content + ");\n";
    return true;
}

bool Parser::process_no_param_rtl_func_statement(Token &new_token) {
    int func_no = parsing_stack.back().second.no;
    switch (func_no) {
        case 1:
            result << "std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');\n";
            break;
        case 4:
            result << "std::cout << std::endl;\n";
            break;
        case 7:
            if (current_symbol_table->functype && current_symbol_table->functype->ret_type) {
                result << "return " << no_token[current_symbol_table->functype->id_no] << "_ret;\n";
            }
            else
                if (! current_symbol_table->functype)
                    result << "return 0;\n";
                else
                    result << "return;\n";
            break;
        case 8:
            result << "exit(0);\n";
            break;
        default:
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "RTL function does not support");
            return false;
    }
    return true;
}

bool Parser::process_rtl_func_statement(Token &new_token) {
    int func_no = parsing_stack[parsing_stack.size() - 4].second.no;
    switch (func_no) {
        case 0:
            result << "std::cin";
            for (int i = 0; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i])
                    return false;
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i]->is_writeable() || parsing_stack[parsing_stack.size() - 2].second.expr_const[i]) {
                    output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                    return false;
                }
                result << " >> " << parsing_stack[parsing_stack.size() - 2].second.expr_content[i];
            }
            result << ";\n";
            break;
        case 1:
            result << "std::cin";
            for (int i = 0; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i])
                    return false;
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i]->is_writeable() || parsing_stack[parsing_stack.size() - 2].second.expr_const[i]) {
                    output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                    return false;
                }
                result << " >> " << parsing_stack[parsing_stack.size() - 2].second.expr_content[i];
            }
            result << ";\n";
            for (int i = 0; i < indent; i++)
                result << "\t";
            result << "std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');\n";
            break;
        case 2:
            if (parsing_stack[parsing_stack.size() - 2].second.expr_type.size() < 2) {
                output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                return false;
            }
            result << "{\n";
            indent++;
            for (int i = 0; i < indent; i++)
                result << "\t";
            if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[0])
                return false;
            if (! (parsing_stack[parsing_stack.size() - 2].second.expr_type[0]->category == 0 && parsing_stack[parsing_stack.size() - 2].second.expr_type[0]->type_no == 33)) {
                output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                return false;
            }
            result << "std::istringstream stream(" << parsing_stack[parsing_stack.size() - 2].second.expr_content[0] << ");\n";
            for (int i = 0; i < indent; i++)
                result << "\t";
            result << "stream";
            for (int i = 1; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i])
                    return false;
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i]->is_writeable() || parsing_stack[parsing_stack.size() - 2].second.expr_const[i]) {
                    output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                    return false;
                }
                result << " >> " << parsing_stack[parsing_stack.size() - 2].second.expr_content[i];
            }
            result << ";\n"; 
            indent--;
            for (int i = 0; i < indent; i++)
                result << "\t";
            result << "}\n";
            break;
        case 3:
            result << "std::cout";
            for (int i = 0; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i])
                    return false;
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i]->is_writeable()) {
                    output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                    return false;
                }
                result << " << " << parsing_stack[parsing_stack.size() - 2].second.expr_content[i];
            }
            result << ";\n";
            break;
        case 4:
            result << "std::cout";
            for (int i = 0; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i])
                    return false;
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i]->is_writeable()) {
                    output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                    return false;
                }
                result << " << " << parsing_stack[parsing_stack.size() - 2].second.expr_content[i];
            }
            result << " << std::endl;\n";
            break;
        case 5:
            if (parsing_stack[parsing_stack.size() - 2].second.expr_type.size() < 2) {
                output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                return false;
            }
            result << "{\n";
            indent++;
            for (int i = 0; i < indent; i++)
                result << "\t";
            if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[0])
                return false;
            if (! (parsing_stack[parsing_stack.size() - 2].second.expr_type[0]->category == 0 && parsing_stack[parsing_stack.size() - 2].second.expr_type[0]->type_no == 33) || parsing_stack[parsing_stack.size() - 2].second.expr_const[0]) {
                output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                return false;
            }
            result << "std::ostringstream stream;\n";
            for (int i = 0; i < indent; i++)
                result << "\t";
            result << "stream";
            for (int i = 1; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i])
                    return false;
                if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[i]->is_writeable()) {
                    output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure/function not declared");
                    return false;
                }
                result << " << " << parsing_stack[parsing_stack.size() - 2].second.expr_content[i];
            }
            result << ";\n";
            for (int i = 0; i < indent; i++)
                result << "\t";
            result << parsing_stack[parsing_stack.size() - 2].second.expr_content[0] << " = stream.str();\n";
            indent--;
            for (int i = 0; i < indent; i++)
                result << "\t";
            result << "}\n";
            break;
        case 7:
            if (parsing_stack[parsing_stack.size() - 2].second.expr_type.size() > 1) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "too many parameters");
                return false;
            }
            if (! current_symbol_table->functype || ! current_symbol_table->functype->ret_type) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "procedures can't return a value");
                return false;
            }
            if (! parsing_stack[parsing_stack.size() - 2].second.expr_type[0])
                return false;
            if (! type_match(current_symbol_table->functype->ret_type, parsing_stack[parsing_stack.size() - 2].second.expr_type[0], 3)) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible return value types");
                return false;
            }
            result << "return " << parsing_stack[parsing_stack.size() - 2].second.expr_content[0] << ";\n";
            break;
        case 27:
            if (parsing_stack[parsing_stack.size() - 2].second.expr_type.size() > 1) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "too many parameters");
                return false;
            }
            {
                Type *type = parsing_stack[parsing_stack.size() - 2].second.expr_type[0];
                for (; type && type->category == 6; type = type->named_type);
                if (! type)
                    return false;
                if (type->category != 0) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
                if (! ((type->type_no >= 0 && type->type_no <= 19) || type->type_no == 31)) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
            }
            if (parsing_stack[parsing_stack.size() - 2].second.expr_const[0]) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "modification of constant");
                return false;
            }
            result << parsing_stack[parsing_stack.size() - 2].second.expr_content[0] << "--;\n";
            break;
        case 30:
            if (parsing_stack[parsing_stack.size() - 2].second.expr_type.size() > 1) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "too many parameters");
                return false;
            }
            {
                Type *type = parsing_stack[parsing_stack.size() - 2].second.expr_type[0];
                for (; type && type->category == 6; type = type->named_type);
                if (! type)
                    return false;
                if (type->category != 0) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
                if (! ((type->type_no >= 0 && type->type_no <= 19) || type->type_no == 31)) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
            }
            if (parsing_stack[parsing_stack.size() - 2].second.expr_const[0]) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "modification of constant");
                return false;
            }
            result << parsing_stack[parsing_stack.size() - 2].second.expr_content[0] << "++;\n";
            break;
        default:
            output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "RTL function does not support");
            return false;
    }
    return true;
}

bool Parser::process_compound_statement(Token &new_token) {
    indent--;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "}\n";
    if (parsing_stack.size() - 6 >= 0 && parsing_stack[parsing_stack.size() - 6].second.category == -1 && parsing_stack[parsing_stack.size() - 6].second.no == 73)
        parsing_stack[parsing_stack.size() - 6].second.format_dealed = false;
    return true;
}

bool Parser::process_goto_statement(Token &new_token) {
    std::string label = parsing_stack.back().second.content;
    for (int i = 0; i < label.length(); i++)
        if (! isdigit(label[i])) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "label syntax error");
            return false;
        }
    int label_no = -1;
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->labels.count(label)) {
            if (! p->label_defined[label]) {
                output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "label has not been defined");
                return false;
            }
            label_no = p->labels[label];
            break;
        }
    if (label_no == -1) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "label has not been declared");
        return false;
    }
    result << "goto label_" << label_no << ";\n";
    return true;
}

bool Parser::process_case_statement(Token &new_token) {
    indent--;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "}\n";
    return true;
}

bool Parser::process_break_statement(Token &new_token) {
    if (! loop_cnt) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "break not allowed");
        return false;
    }
    result << "break;\n";
    return true;
}

bool Parser::process_continue_statement(Token &new_token) {
    if (! loop_cnt) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "continue not allowed");
        return false;
    }
    result << "continue;\n";
    return true;
}

bool Parser::process_for_statement(Token &new_token) {
    if (parsing_stack[parsing_stack.size() - 2].second.format_dealed) {
        indent--;
        parsing_stack[parsing_stack.size() - 2].second.format_dealed = false;
    }
    loop_cnt--;
    return true;
}

bool Parser::process_while_statement(Token &new_token) {
    if (parsing_stack[parsing_stack.size() - 2].second.format_dealed) {
        indent--;
        parsing_stack[parsing_stack.size() - 2].second.format_dealed = false;
    }
    loop_cnt--;
    return true;
}

bool Parser::process_repeat_statement(Token &new_token) {
    loop_cnt--;
    indent--;
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "} while (! (";
    Type *type = parsing_stack.back().second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 0 || ! (type->type_no >= 27 && type->type_no <= 30)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "boolean expression expected");
        return false;
    }
    result << parsing_stack.back().second.content << "));\n";
    return true;
}

bool Parser::process_M9(Token &new_token) {
    if (parsing_stack.size() - 2 >= 0 && parsing_stack[parsing_stack.size() - 2].second.category == -1 && parsing_stack[parsing_stack.size() - 2].second.no == 73 && ! parsing_stack[parsing_stack.size() - 2].second.format_dealed) {
        parsing_stack[parsing_stack.size() - 2].second.format_dealed = true;
        result << "\n";
        indent++;
    }
    for (int i = 0; i < indent; i++)
        result << "\t";
    return true;
}

bool Parser::process_id_var_access(Token &new_token) {
    new_token.type = NULL;
    new_token.pointer_const = false;
    int id_no = parsing_stack.back().second.no;
    new_token.content = no_token[id_no];
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->defined(id_no))
            if (p->symbols.count(id_no)) {
                new_token.type = p->symbols[id_no];
                new_token.is_const = p->is_const[id_no];
                if (p->functype && p->functype->ret_type && p->functype->id_no == id_no)
                    new_token.content = new_token.content + "_ret";
                break;
            }
            else
                if (p->enum_items.count(id_no)) {
                    new_token.type = p->enum_items[id_no];
                    new_token.is_const = true;
                }
                else
                    if (p->subtable.count(id_no)) {
                        bool flag = false;
                        for (int i = 0; i < p->subtable[id_no].size(); i++)
                            if (p->subtable[id_no][i]->functype->param_list.empty()) {
                                flag = true;
                                if (! p->subtable[id_no][i]->functype->ret_type) {
                                    output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "procedure not allowed");
                                    return false;
                                }
                                else
                                    new_token.type = p->subtable[id_no][i]->functype->ret_type;
                                new_token.is_const = true;
                                new_token.content += "()";
                            }
                        if (! flag) {
                            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier type is not allowed");
                            return false;
                        }
                    }
                    else {
                        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier type is not allowed");
                        return false;
                    }
    if (! new_token.type) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "identifier not defined");
        return false;
    }
    return true;
}

bool Parser::process_array_var_access(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    new_token.pointer_const = false;
    if (! new_token.type)
        return false;
    new_token.is_const = parsing_stack[parsing_stack.size() - 5].second.is_const;
    new_token.content = parsing_stack[parsing_stack.size() - 5].second.content + parsing_stack[parsing_stack.size() - 2].second.content;
    return true;
}

bool Parser::process_member_var_access(Token &new_token) {
    new_token.type = NULL;
    new_token.pointer_const = false;
    Type *type = parsing_stack[parsing_stack.size() - 3].second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type || type->category != 4) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "invalid operation");
        return false;
    }
    int id_no = parsing_stack.back().second.no;
    bool flag = false;
    for (int i = 0; i < type->record_list.size() && ! flag; i++)
        for (int j = 0; j < type->record_list[i].first.size(); j++)
            if (type->record_list[i].first[j] == id_no) {
                new_token.type = type->record_list[i].second;
                flag = true;
                break;
            }
    if (! flag) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "record member not found");
        return false;
    }
    new_token.is_const = parsing_stack[parsing_stack.size() - 3].second.is_const;
    if (parsing_stack[parsing_stack.size() - 3].second.content.size() && parsing_stack[parsing_stack.size() - 3].second.content[0] == '*')
        new_token.content = "(" + parsing_stack[parsing_stack.size() - 3].second.content + ")." + no_token[id_no];
    else
        new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + "." + no_token[id_no];
    return true;
}

bool Parser::process_pointer_var_access(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    new_token.pointer_const = false;
    for (; type && type->category == 6; type = type->named_type);
    if (! type || type->category != 1) {
        new_token.type = NULL;
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "invalid operation");
        return false;
    }
    new_token.type = type->pointer_type;
    new_token.is_const = false;
    new_token.content = "*" + parsing_stack[parsing_stack.size() - 2].second.content;
    return true;
}

bool Parser::process_proc_func_access(Token &new_token) {
    new_token.pointer_const = false;
    bool flag = false;
    for (int i = 0; i < parsing_stack[parsing_stack.size() - 2].second.expr_type.size(); i++) {
        Type *type = parsing_stack[parsing_stack.size() - 2].second.expr_type[i];
        for (; type && type->category == 6; type = type->named_type);
        if (! type)
            return false;
        if (type->category == 1 && parsing_stack[parsing_stack.size() - 2].second.expr_pointer_const[i]) {
            output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "using constant pointer as parameter not supported");
            return false;
        }
    }
    int id_no = parsing_stack[parsing_stack.size() - 4].second.no;
    for (SymbolTable *p = current_symbol_table; p; p = p->parent)
        if (p->subtable.count(id_no)) {
            for (int i = 0; i < p->subtable[id_no].size(); i++)
                if (p->subtable[id_no][i]->functype->param_list.size() == parsing_stack[parsing_stack.size() - 2].second.expr_type.size()) {
                    bool match = true, completely_match = true;
                    for (int j = 0; j < p->subtable[id_no][i]->functype->param_list.size(); j++)
                        if (! (p->subtable[id_no][i]->functype->param_list[j].second.first == 1 && parsing_stack[parsing_stack.size() - 2].second.expr_const[j])) {
                            if (p->subtable[id_no][i]->functype->param_list[j].second.first == 1) {
                                if (! type_match(p->subtable[id_no][i]->functype->param_list[j].second.second, parsing_stack[parsing_stack.size() - 2].second.expr_type[j], 1)) {
                                    match = completely_match = false;
                                    break;
                                }
                            }
                            else
                                if (! type_match(p->subtable[id_no][i]->functype->param_list[j].second.second, parsing_stack[parsing_stack.size() - 2].second.expr_type[j], 2)) {
                                    match = completely_match = false;
                                    break;
                                }
                                else
                                    if (! type_match(p->subtable[id_no][i]->functype->param_list[j].second.second, parsing_stack[parsing_stack.size() - 2].second.expr_type[j], 1))
                                        completely_match = false;
                        }
                        else {
                            match = completely_match = false;
                            break;
                        }
                    if ((! flag && match) || completely_match) {
                        flag = true;
                        if (! p->subtable[id_no][i]->functype->ret_type) {
                            new_token.type = NULL;
                            output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "procedure not allowed");
                            return false;
                        }
                        else
                            new_token.type = p->subtable[id_no][i]->functype->ret_type;
                    }
                }
            if (flag)
                break;
        }
    if (! flag) {
        new_token.type = NULL;
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "function not declared");
        return false;
    }
    new_token.is_const = true;
    new_token.content = no_token[id_no] + "(" + parsing_stack[parsing_stack.size() - 2].second.content + ")";
    return true;
}

bool Parser::process_no_param_rtl_func_access(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    int func_no = parsing_stack.back().second.no;
    switch (func_no) {
        
        default:
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "RTL function does not support");
            return false;
    }
    return true;
}

bool Parser::process_rtl_func_access(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    int func_no = parsing_stack[parsing_stack.size() - 4].second.no;
    switch (func_no) {
        case 9:
            if (parsing_stack[parsing_stack.size() - 2].second.expr_type.size() > 1) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "too many parameters");
                return false;
            }
            {
                Type *type = parsing_stack[parsing_stack.size() - 2].second.expr_type[0];
                for (; type && type->category == 6; type = type->named_type);
                if (! type)
                    return false;
                if (type->category != 0) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
                if (! (type->type_no >= 0 && type->type_no <= 19)) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
            }
            new_token.type = new Type;
            new_token.type->category = 0;
            new_token.type->type_no = 31;
            new_token.is_const = true;
            new_token.content = "((char)" + parsing_stack[parsing_stack.size() - 2].second.expr_content[0] + ")";
            break;
        case 10:
            if (parsing_stack[parsing_stack.size() - 2].second.expr_type.size() > 1) {
                output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "too many parameters");
                return false;
            }
            {
                Type *type = parsing_stack[parsing_stack.size() - 2].second.expr_type[0];
                for (; type && type->category == 6; type = type->named_type);
                if (! type)
                    return false;
                if (type->category != 0) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
                if (! ((type->type_no >= 0 && type->type_no <= 19) || type->type_no == 31)) {
                    output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
                    return false;
                }
            }
            new_token.type = new Type;
            new_token.type->category = 0;
            new_token.type->type_no = 17;
            new_token.is_const = true;
            new_token.content = "((int)" + parsing_stack[parsing_stack.size() - 2].second.expr_content[0] + ")";
            break; 
        default:
            output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "RTL function does not support");
            return false;
    }
    return true;
}

bool Parser::process_M10(Token &new_token) {
    new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    return true;
}

bool Parser::process_single_array_index_list(Token &new_token) {
    new_token.type = NULL;
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category == 0 && type->type_no == 33) {
        Type *index_type = parsing_stack.back().second.type;
        for (; index_type && index_type->category == 6; index_type = index_type->named_type);
        if (! index_type)
            return false;
        if (index_type->category != 0 || index_type->type_no > 19) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
            return false;
        }
        new_token.type = new Type;
        new_token.type->category = 0;
        new_token.type->type_no = 31;
        new_token.content = "[" + parsing_stack.back().second.content + " - 1]";
        return true;
    }
    if (type->category != 2 || type->array_index_type == -1) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "invalid operation");
        return false;
    }
    Type *index_type = parsing_stack.back().second.type;
    for (; index_type && index_type->category == 6; index_type = index_type->named_type);
    if (! index_type)
        return false;
    if (type->array_index_type == 0) {
        if (index_type->category != 0 || index_type->type_no > 19) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
            return false;
        }
        new_token.type = type->array_type;
        new_token.content = "[" + parsing_stack.back().second.content + " - " + type->array_bias + "]";
    }
    else {
        if (index_type->category != 0 || index_type->type_no != 31) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
            return false;
        }
        new_token.type = type->array_type;
        new_token.content = "[" + parsing_stack.back().second.content + " - '" + type->array_bias + "']";
    }
    return true;
}

bool Parser::process_array_index_list(Token &new_token) {
    new_token.type = NULL;
    Type *type = parsing_stack[parsing_stack.size() - 3].second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 2 || type->array_index_type == -1) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "invalid operation");
        return false;
    }
    Type *index_type = parsing_stack.back().second.type;
    for (; index_type && index_type->category == 6; index_type = index_type->named_type);
    if (! index_type)
        return false;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content;
    if (type->array_index_type == 0) {
        if (index_type->category != 0 || index_type->type_no > 19) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
            return false;
        }
        new_token.type = type->array_type;
        new_token.content += "[" + parsing_stack.back().second.content + " - " + type->array_bias + "]";
    }
    else {
        if (index_type->category != 0 || index_type->type_no != 31) {
            output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
            return false;
        }
        new_token.type = type->array_type;
        new_token.content += "[" + parsing_stack.back().second.content + " - '" + type->array_bias + "']";
    }
    return true;
}

bool Parser::process_string_const_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = parsing_stack.back().second.type;
    if (new_token.type->type_no == 31)
        new_token.content = "'" + parsing_stack.back().second.content + "'";
    else
        new_token.content = "\"" + parsing_stack.back().second.content + "\"";
    new_token.is_const = parsing_stack.back().second.is_const;
    return true;
}

bool Parser::process_bool_const_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.is_const = true;
    if (parsing_stack.back().second.no == 45)
        new_token.content = "true";
    else
        new_token.content = "false";
    return true;
}

bool Parser::process_single_expression(Token &new_token) {
    new_token.pointer_const = parsing_stack.back().second.pointer_const;
    new_token.type = parsing_stack.back().second.type;
    new_token.content = parsing_stack.back().second.content;
    new_token.is_const = parsing_stack.back().second.is_const;
    return true;
}

bool Parser::process_bracket_expression(Token &new_token) {
    new_token.pointer_const = parsing_stack[parsing_stack.size() - 2].second.pointer_const;
    new_token.type = parsing_stack[parsing_stack.size() - 2].second.type;
    new_token.content = "(" + parsing_stack[parsing_stack.size() - 2].second.content + ")";
    new_token.is_const = parsing_stack[parsing_stack.size() - 2].second.is_const;
    return true;
}

bool Parser::process_not_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *type = parsing_stack.back().second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 0 || ! (type->type_no <= 19 || (type->type_no >= 27 && type->type_no <= 30))) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = type->type_no;
    new_token.is_const = true;
    if (type->type_no <= 19)
        new_token.content = "~ " + parsing_stack.back().second.content;
    else
        new_token.content = "! " + parsing_stack.back().second.content;
    return true;
}

bool Parser::process_address_expression(Token &new_token) {
    new_token.type = NULL;
    Type *type = parsing_stack.back().second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    new_token.type = new Type;
    new_token.type->category = 1;
    if (type->category == 2) {
        new_token.type->pointer_type = type->array_type;
        new_token.content = parsing_stack.back().second.content;
    }
    else {
        new_token.type->pointer_type = parsing_stack.back().second.type;
        new_token.content = "& " + parsing_stack.back().second.content;
    }
    new_token.pointer_const = parsing_stack.back().second.is_const;
    new_token.is_const = true;
    return true;
}

bool Parser::process_unary_add_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *type = parsing_stack.back().second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 0 || type->type_no > 26) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = parsing_stack.back().second.type;
    new_token.is_const = true;
    new_token.content = "+ " + parsing_stack.back().second.content;
    return true;
}

bool Parser::process_unary_subtract_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *type = parsing_stack.back().second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 0 || type->type_no > 26) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = parsing_stack.back().second.type;
    new_token.is_const = true;
    new_token.content = "- " + parsing_stack.back().second.content;
    return true;
}

bool Parser::process_multiply_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 26) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 26) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.is_const = true;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " * " + parsing_stack.back().second.content;
    return true;
}

bool Parser::process_division_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 26) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 26) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (typea->type_no <= 19 && typeb->type_no <= 19) {
        new_token.type = new Type;
        new_token.type->category = 0;
        new_token.type->type_no = 23;
        new_token.content = "(double)(" + parsing_stack[parsing_stack.size() - 3].second.content + ") / " + parsing_stack.back().second.content;
    }
    else {
        new_token.type = arithmetic_type_check(typea, typeb);
        new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " / " + parsing_stack.back().second.content;
    }
    new_token.is_const = true;
    return true;
}

bool Parser::process_integer_division_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 19) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 19) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " / " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_modulo_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 19) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 19) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " % " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_and_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 0 && typea->type_no == 27 && typeb->category == 0 && typeb->type_no == 27) {
        new_token.type = typea;
        new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " && " + parsing_stack.back().second.content;
        new_token.is_const = true;
        return true;
    }
    if (typea->category != 0 || typea->type_no > 19) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 19) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.content = "(" + parsing_stack[parsing_stack.size() - 3].second.content + " & " + parsing_stack.back().second.content + ")";
    new_token.is_const = true;
    return true;
}

bool Parser::process_left_shift_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 19) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 19) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " << " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_right_shift_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 19) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 19) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = parsing_stack[parsing_stack.size() - 3].second.type;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " >> " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_add_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 0 && (typea->type_no == 31 || typea->type_no == 33) && typeb->category == 0 && (typeb->type_no == 31 || typeb->type_no == 33)) {
        new_token.type = new Type;
        new_token.type->category = 0;
        new_token.type->type_no = 33;
        if (typea->type_no == 33 && parsing_stack[parsing_stack.size() - 3].second.content[0] == '"')
            parsing_stack[parsing_stack.size() - 3].second.content = "std::string(" + parsing_stack[parsing_stack.size() - 3].second.content + ")";
        if (typeb->type_no == 33 && parsing_stack.back().second.content[0] == '"')
            parsing_stack.back().second.content = "std::string(" + parsing_stack.back().second.content + ")";
        if (typea->type_no == 31)
            parsing_stack[parsing_stack.size() - 3].second.content = "std::string(1, " + parsing_stack[parsing_stack.size() - 3].second.content + ")";
        if (typeb->type_no == 31)
            parsing_stack.back().second.content = "std::string(1, " + parsing_stack.back().second.content + ")";
        new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " + " + parsing_stack.back().second.content;
        return true; 
    }
    if (typea->category != 0 || typea->type_no > 26) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 26) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.is_const = true;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " + " + parsing_stack.back().second.content;
    return true;
}

bool Parser::process_subtract_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 26) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 26) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.is_const = true;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " - " + parsing_stack.back().second.content;
    return true;
}

bool Parser::process_or_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 0 && typea->type_no == 27 && typeb->category == 0 && typeb->type_no == 27) {
        new_token.type = typea;
        new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " || " + parsing_stack.back().second.content;
        new_token.is_const = true;
        return true;
    }
    if (typea->category != 0 || typea->type_no > 19) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 19) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.content = "(" + parsing_stack[parsing_stack.size() - 3].second.content + " | " + parsing_stack.back().second.content + ")";
    new_token.is_const = true;
    return true;
}

bool Parser::process_xor_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category != 0 || typea->type_no > 19) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category != 0 || typeb->type_no > 19) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = arithmetic_type_check(typea, typeb);
    new_token.content = "(" + parsing_stack[parsing_stack.size() - 3].second.content + " ^ " + parsing_stack.back().second.content + ")";
    new_token.is_const = true;
    return true;
}

bool Parser::process_equal_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 2 || typea->category == 4 || typea->category == 5 || (typea->category == 0 && typea->type_no == 37)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category == 2 || typeb->category == 4 || typeb->category == 5 || (typeb->category == 0 && typeb->type_no == 37)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 3].second.type, parsing_stack.back().second.type, 2)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " == " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_not_equal_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 2 || typea->category == 4 || typea->category == 5 || (typea->category == 0 && typea->type_no == 37)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category == 2 || typeb->category == 4 || typeb->category == 5 || (typeb->category == 0 && typeb->type_no == 37)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 3].second.type, parsing_stack.back().second.type, 2)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " != " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_less_than_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 2 || typea->category == 4 || typea->category == 5 || (typea->category == 0 && typea->type_no == 37)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category == 2 || typeb->category == 4 || typeb->category == 5 || (typeb->category == 0 && typeb->type_no == 37)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 3].second.type, parsing_stack.back().second.type, 2)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " < " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_greater_than_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 2 || typea->category == 4 || typea->category == 5 || (typea->category == 0 && typea->type_no == 37)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category == 2 || typeb->category == 4 || typeb->category == 5 || (typeb->category == 0 && typeb->type_no == 37)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 3].second.type, parsing_stack.back().second.type, 2)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " > " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_less_than_equal_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 2 || typea->category == 4 || typea->category == 5 || (typea->category == 0 && typea->type_no == 37)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category == 2 || typeb->category == 4 || typeb->category == 5 || (typeb->category == 0 && typeb->type_no == 37)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 3].second.type, parsing_stack.back().second.type, 2)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " <= " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_greater_than_equal_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typea->category == 2 || typea->category == 4 || typea->category == 5 || (typea->category == 0 && typea->type_no == 37)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    if (typeb->category == 2 || typeb->category == 4 || typeb->category == 5 || (typeb->category == 0 && typeb->type_no == 37)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 3].second.type, parsing_stack.back().second.type, 2)) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + " >= " + parsing_stack.back().second.content;
    new_token.is_const = true;
    return true;
}

bool Parser::process_in_expression(Token &new_token) {
    new_token.pointer_const = false;
    new_token.type = NULL;
    Type *typea = parsing_stack[parsing_stack.size() - 3].second.type, *typeb = parsing_stack.back().second.type;
    for (; typea && typea->category == 6; typea = typea->named_type);
    for (; typeb && typeb->category == 6; typeb = typeb->named_type);
    if (! typea || ! typeb)
        return false;
    if (typeb->category != 5) {
        output_error(parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos, "incompatible types");
        return false;
    }
    if (! typeb->set_type)
        return false;
    if (! type_match(typeb->set_type, parsing_stack[parsing_stack.size() - 3].second.type, 2)) {
        output_error(parsing_stack[parsing_stack.size() - 3].second.line, parsing_stack[parsing_stack.size() - 3].second.col, parsing_stack[parsing_stack.size() - 3].second.pos, "incompatible types");
        return false;
    }
    new_token.type = new Type;
    new_token.type->category = 0;
    new_token.type->type_no = 27;
    new_token.content = parsing_stack.back().second.content + ".count(" + parsing_stack[parsing_stack.size() - 3].second.content + ")";
    new_token.is_const = true;
    return true;
}

bool Parser::process_single_expression_list(Token &new_token) {
    new_token.expr_type.clear();
    new_token.expr_const.clear();
    new_token.expr_content.clear();
    new_token.expr_pointer_const.clear();
    new_token.expr_type.push_back(parsing_stack.back().second.type);
    new_token.expr_const.push_back(parsing_stack.back().second.is_const);
    new_token.expr_content.push_back(parsing_stack.back().second.content);
    new_token.expr_pointer_const.push_back(parsing_stack.back().second.pointer_const);
    new_token.content = parsing_stack.back().second.content;
    return true;
}

bool Parser::process_expression_list(Token &new_token) {
    new_token.expr_type = parsing_stack[parsing_stack.size() - 3].second.expr_type;
    new_token.expr_const = parsing_stack[parsing_stack.size() - 3].second.expr_const;
    new_token.expr_content = parsing_stack[parsing_stack.size() - 3].second.expr_content;
    new_token.expr_pointer_const = parsing_stack[parsing_stack.size() - 3].second.expr_pointer_const;
    new_token.expr_type.push_back(parsing_stack.back().second.type);
    new_token.expr_const.push_back(parsing_stack.back().second.is_const);
    new_token.expr_content.push_back(parsing_stack.back().second.content);
    new_token.expr_pointer_const.push_back(parsing_stack.back().second.pointer_const);
    new_token.content = parsing_stack[parsing_stack.size() - 3].second.content + ", " + parsing_stack.back().second.content;
    return true;
}

bool Parser::process_M11(Token &new_token) {
    new_token.format_dealed = false;
    return true;
}

bool Parser::process_M12(Token &new_token) {
    if (parsing_stack.size() - 3 >= 0 && parsing_stack[parsing_stack.size() - 3].second.category == -1 && parsing_stack[parsing_stack.size() - 3].second.no == 73 && ! parsing_stack[parsing_stack.size() - 3].second.format_dealed) {
        parsing_stack[parsing_stack.size() - 3].second.format_dealed = true;
        result << " {\n";
    }
    else {
        for (int i = 0; i < indent; i++)
            result << "\t";
        result << "{\n";
    }
    indent++;
    return true;
}

bool Parser::process_M13(Token &new_token) {
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 0 || ! (type->type_no >= 27 && type->type_no <= 30)) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "boolean expression expected");
        return false;
    }
    result << "if (" << parsing_stack[parsing_stack.size() - 2].second.content << ")";
    return true;
}

bool Parser::process_M14(Token &new_token) {
    if (parsing_stack[parsing_stack.size() - 2].second.format_dealed) {
        indent--;
        parsing_stack[parsing_stack.size() - 2].second.format_dealed = false;
    }
    return true;
}

bool Parser::process_M15(Token &new_token) {
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "else";
    return true;
}

bool Parser::process_M16(Token &new_token) {
    new_token.case_type = -1;
    new_token.case_vals.clear();
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 0) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "ordinal or char type expression expected");
        return false;
    }
    if (type->type_no >= 0 && type->type_no <= 19)
        new_token.case_type = 0;
    else
        if (type->type_no == 31)
            new_token.case_type = 1;
        else {
            output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "ordinal or char type expression expected");
            return false;
        }
    result << "switch (" << parsing_stack[parsing_stack.size() - 2].second.content << ") {\n";
    indent++;
    return true;
}

bool Parser::process_case_part(Token &new_token) {
    if (! (parsing_stack[parsing_stack.size() - 3].second.category == 0 && (parsing_stack[parsing_stack.size() - 3].second.no == 13 || parsing_stack[parsing_stack.size() - 3].second.no == 54))) {
        for (int i = 0; i < indent; i++)
            result << "\t";
        result << "break;\n";
    }
    indent--;
    return true;
}

bool Parser::process_M17(Token &new_token) {
    int prev_mark;
    if (parsing_stack[parsing_stack.size() - 3].second.category == -1 && parsing_stack[parsing_stack.size() - 3].second.no == 79)
        prev_mark = parsing_stack.size() - 3;
    else
        prev_mark = parsing_stack.size() - 5;
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    if (! type)
        return false;
    if (parsing_stack[prev_mark].second.case_type == -1)
        return false;
    if (parsing_stack[prev_mark].second.case_type == 0) {
        if (! (parsing_stack[parsing_stack.size() - 2].second.type->type_no <= 19)) {
            output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible type: expected int but found char");
            return false;
        }
        long long case_val = std::stoll(parsing_stack[parsing_stack.size() - 2].second.content, 0, 0);
        if (parsing_stack[prev_mark].second.case_vals.count(case_val)) {
            output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "duplicate case label");
            return false;
        }
        parsing_stack[prev_mark].second.case_vals.insert(case_val);
        for (int i = 0; i < indent; i++)
            result << "\t";
        result << "case " << parsing_stack[parsing_stack.size() - 2].second.content << ":\n";
        indent++;
    }
    else {
        if (! (parsing_stack[parsing_stack.size() - 2].second.type->type_no == 31 || parsing_stack[parsing_stack.size() - 2].second.type->type_no == 33)) {
            output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible type: expected char but found int");
            return false;
        }
        char tmp[10];
        sprintf(tmp, parsing_stack[parsing_stack.size() - 2].second.content.c_str());
        long long case_val = tmp[0];
        if (parsing_stack[prev_mark].second.case_vals.count(case_val)) {
            output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "duplicate case label");
            return false;
        }
        parsing_stack[prev_mark].second.case_vals.insert(case_val);
        for (int i = 0; i < indent; i++)
            result << "\t";
        result << "case '" << parsing_stack[parsing_stack.size() - 2].second.content << "':\n";
        indent++;
    }
    return true;
}

bool Parser::process_M18(Token &new_token) {
    for (int i = 0; i < indent; i++)
        result << "\t";
    result << "default:\n";
    indent++;
    return true;
}

bool Parser::process_M19(Token &new_token) {
    loop_cnt++;
    Type *type = parsing_stack[parsing_stack.size() - 6].second.type, *val_begin_type = parsing_stack[parsing_stack.size() - 4].second.type, *val_end_type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (; type && type->category == 6; type = type->named_type);
    for (; val_begin_type && val_begin_type->category == 6; val_begin_type = val_begin_type->named_type);
    for (; val_end_type && val_end_type->category == 6; val_end_type = val_end_type->named_type);
    if (! type || ! val_begin_type || ! val_end_type)
        return false;
    if (type->category != 0) {
        output_error(parsing_stack[parsing_stack.size() - 6].second.line, parsing_stack[parsing_stack.size() - 6].second.col, parsing_stack[parsing_stack.size() - 6].second.pos, "incompatible types");
        return false;
    }
    if (! ((type->type_no >= 0 && type->type_no <= 19) || type->type_no == 31)) {
        output_error(parsing_stack[parsing_stack.size() - 6].second.line, parsing_stack[parsing_stack.size() - 6].second.col, parsing_stack[parsing_stack.size() - 6].second.pos, "incompatible types");
        return false;
    }
    if (val_begin_type->category != 0) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "incompatible types");
        return false;
    }
    if (! ((val_begin_type->type_no >= 0 && val_begin_type->type_no <= 19) || val_begin_type->type_no == 31)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "incompatible types");
        return false;
    }
    if (val_end_type->category != 0) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
        return false;
    }
    if (! ((val_end_type->type_no >= 0 && val_end_type->type_no <= 19) || val_end_type->type_no == 31)) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
        return false;
    } 
    if (! type_match(parsing_stack[parsing_stack.size() - 6].second.type, parsing_stack[parsing_stack.size() - 4].second.type, 3)) {
        output_error(parsing_stack[parsing_stack.size() - 4].second.line, parsing_stack[parsing_stack.size() - 4].second.col, parsing_stack[parsing_stack.size() - 4].second.pos, "incompatible types");
        return false;
    }
    if (! type_match(parsing_stack[parsing_stack.size() - 6].second.type, parsing_stack[parsing_stack.size() - 2].second.type, 3)) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "incompatible types");
        return false;
    }
    if (parsing_stack[parsing_stack.size() - 6].second.is_const) {
        output_error(parsing_stack[parsing_stack.size() - 6].second.line, parsing_stack[parsing_stack.size() - 6].second.col, parsing_stack[parsing_stack.size() - 6].second.pos, "assignment of constant");
        return false;
    }
    result << "for (" << parsing_stack[parsing_stack.size() - 6].second.content  << " = " << parsing_stack[parsing_stack.size() - 4].second.content << "; " << parsing_stack[parsing_stack.size() - 6].second.content;
    if (parsing_stack[parsing_stack.size() - 3].second.no == 43)
        result << " <= ";
    else
        result << " >= ";
    result << parsing_stack[parsing_stack.size() - 2].second.content << "; " << parsing_stack[parsing_stack.size() - 6].second.content;
    if (parsing_stack[parsing_stack.size() - 3].second.no == 43)
        result << "++)";
    else
        result << "--)";
    return true;
}

bool Parser::process_M20(Token &new_token) {
    loop_cnt++;
    Type *type = parsing_stack[parsing_stack.size() - 2].second.type;
    for (; type && type->category == 6; type = type->named_type);
    if (! type)
        return false;
    if (type->category != 0 || ! (type->type_no >= 27 && type->type_no <= 30)) {
        output_error(parsing_stack[parsing_stack.size() - 2].second.line, parsing_stack[parsing_stack.size() - 2].second.col, parsing_stack[parsing_stack.size() - 2].second.pos, "boolean expression expected");
        return false;
    }
    result << "while (" << parsing_stack[parsing_stack.size() - 2].second.content << ")";
    return true;
}

bool Parser::process_M21(Token &new_token) {
    loop_cnt++;
    result << "do {\n";
    indent++;
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
    tmp.process = &Parser::process_program;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program = M1 program-block '.'
    tmp.left = 1;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 2));
    tmp.right.push_back(Symbol(-1, 4));
    tmp.right.push_back(Symbol(7, 0));
    tmp.process = &Parser::process_program;
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
    //program-heading = 'program' identifier '(' program-identifier-list ')'
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
    //program-identifier-list = identifier
    tmp.left = 5;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-identifier-list = program-identifier-list ',' identifier
    tmp.left = 5;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 5));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-block = definition-part program-statement-part
    tmp.left = 4;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.right.push_back(Symbol(-1, 56));
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
    //definition-part = definition-part variable-definition-part
    tmp.left = 6;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.right.push_back(Symbol(-1, 39));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //definition-part = definition-part procedure-definition-part
    tmp.left = 6;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.right.push_back(Symbol(-1, 44));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //definition-part = definition-part function-definition-part
    tmp.left = 6;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.right.push_back(Symbol(-1, 45));
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
    //label-list = integer-literal
    tmp.left = 8;
    tmp.right.clear();
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_label;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-list = label-list ',' integer-literal
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
    tmp.right.push_back(Symbol(-1, 19));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_int_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' signed-float ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 20));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_float_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' string ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 21));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_string_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-definition = identifier '=' true ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(0, 44));
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
    //const-definition = const-type-declarification '=' M2 const-val ';'
    tmp.left = 11;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 28));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 29));
    tmp.right.push_back(Symbol(-1, 30));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_typed_constant_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-definition-part = 'type' type-definitions
    tmp.left = 12;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 45));
    tmp.right.push_back(Symbol(-1, 13));
    tmp.process = &Parser::process_newline;
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
    //type-definition = type-name '=' type-denoter ';'
    tmp.left = 14;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 15));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_type_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-name = identifier
    tmp.left = 15;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_type_name;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = identifier
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_id_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = origin-type
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(1, 0));
    tmp.process = &Parser::process_basic_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = '^' no-array-type-identifier
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 14));
    tmp.right.push_back(Symbol(-1, 51));
    tmp.process = &Parser::process_pointer_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = array '[' subrange-list ']' of type-denoter
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 1));
    tmp.right.push_back(Symbol(7, 6));
    tmp.right.push_back(Symbol(-1, 17));
    tmp.right.push_back(Symbol(7, 7));
    tmp.right.push_back(Symbol(0, 30));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.process = &Parser::process_array_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = '(' enum-list ')'
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 23));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_enum_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = record field-list end
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 37));
    tmp.right.push_back(Symbol(-1, 25));
    tmp.right.push_back(Symbol(0, 14));
    tmp.process = &Parser::process_record_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = record field-list ';' end
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 37));
    tmp.right.push_back(Symbol(-1, 25));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(0, 14));
    tmp.process = &Parser::process_record_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-denoter = set of type-denoter
    tmp.left = 16;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 39));
    tmp.right.push_back(Symbol(0, 30));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.process = &Parser::process_set_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //subrange-list = subrange
    tmp.left = 17;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 18));
    tmp.process = &Parser::process_array_single_subrange_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //subrange-list = subrange ',' subrange-list
    tmp.left = 17;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 18));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(-1, 17));
    tmp.process = &Parser::process_array_subrange_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //subrange = array-subrange-case-index '..' array-subrange-case-index
    tmp.left = 18;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 22));
    tmp.right.push_back(Symbol(7, 8));
    tmp.right.push_back(Symbol(-1, 22));
    tmp.process = &Parser::process_array_subrange;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-integer = integer-literal
    tmp.left = 19;
    tmp.right.clear();
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_no_sign_signed_integer;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-integer = '+' integer-literal
    tmp.left = 19;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 7));
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_signed_integer;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-integer = '-' integer-literal
    tmp.left = 19;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 8));
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_signed_integer;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-float = float-literal
    tmp.left = 20;
    tmp.right.clear();
    tmp.right.push_back(Symbol(4, 0));
    tmp.process = &Parser::process_no_sign_signed_float;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-float = '+' float-literal
    tmp.left = 20;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 7));
    tmp.right.push_back(Symbol(4, 0));
    tmp.process = &Parser::process_signed_float;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //signed-float = '-' float-literal
    tmp.left = 20;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 8));
    tmp.right.push_back(Symbol(4, 0));
    tmp.process = &Parser::process_signed_float;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //string = string-literal
    tmp.left = 21;
    tmp.right.clear();
    tmp.right.push_back(Symbol(5, 0));
    tmp.process = &Parser::process_string;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-subrange-case-index = identifier
    tmp.left = 22;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_array_subrange_case_index;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-subrange-case-index = signed-integer
    tmp.left = 22;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 19));
    tmp.process = &Parser::process_array_subrange_case_index;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-subrange-case-index = string
    tmp.left = 22;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 21));
    tmp.process = &Parser::process_array_subrange_case_index;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-list = enum-item
    tmp.left = 23;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 24));
    tmp.process = &Parser::process_single_enum_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-list = enum-list ',' enum-item
    tmp.left = 23;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 23));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(-1, 24));
    tmp.process = &Parser::process_enum_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-item = identifier
    tmp.left = 24;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_enum_item;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //enum-item = identifier ':=' signed-integer
    tmp.left = 24;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(6, 1));
    tmp.right.push_back(Symbol(-1, 19));
    tmp.process = &Parser::process_enum_item;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //field-list = record-section
    tmp.left = 25;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 26));
    tmp.process = &Parser::process_single_field_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //field-list = field-list ';' record-section
    tmp.left = 25;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 25));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(-1, 26));
    tmp.process = &Parser::process_field_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //record-section = identifier-list ':' type-denoter
    tmp.left = 26;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 27));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.process = &Parser::process_record_section;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //identifier-list = identifier
    tmp.left = 27;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_single_id_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //identifier-list = identifier-list ',' identifier
    tmp.left = 27;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 27));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_id_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-type-declarification = identifier ':' type-denoter
    tmp.left = 28;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.process = &Parser::process_const_type_declar;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M2 = ε
    tmp.left = 29;
    tmp.right.clear();
    tmp.process = &Parser::process_M2;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = signed-integer
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 19));
    tmp.process = &Parser::process_num_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = signed-float
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 20));
    tmp.process = &Parser::process_num_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = string
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 21));
    tmp.process = &Parser::process_string_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = true
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 44));
    tmp.process = &Parser::process_bool_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = false
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 15));
    tmp.process = &Parser::process_bool_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = '(' M3 array-vals ')'
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 31));
    tmp.right.push_back(Symbol(-1, 32));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_array_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = '(' M4 record-field-vals ')'
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 34));
    tmp.right.push_back(Symbol(-1, 35));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_record_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //const-val = '(' M4 record-field-vals ';' ')'
    tmp.left = 30;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 34));
    tmp.right.push_back(Symbol(-1, 35));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_record_const_val;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M3 = ε
    tmp.left = 31;
    tmp.right.clear();
    tmp.process = &Parser::process_M3;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-vals = const-val
    tmp.left = 32;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 30));
    tmp.process = &Parser::process_single_array_vals;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-vals = array-vals array-element-split const-val
    tmp.left = 32;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 32));
    tmp.right.push_back(Symbol(-1, 33));
    tmp.right.push_back(Symbol(-1, 30));
    tmp.process = &Parser::process_array_vals;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-element-split = ','
    tmp.left = 33;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 1));
    tmp.process = &Parser::process_array_element_split;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M4 = ε
    tmp.left = 34;
    tmp.right.clear();
    tmp.process = &Parser::process_M4;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //record-field-vals = identifier single-record-field-val-split const-val
    tmp.left = 35;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(-1, 36));
    tmp.right.push_back(Symbol(-1, 30));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //record-field-vals = record-field-vals record-field-split identifier record-field-val-split const-val
    tmp.left = 35;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 35));
    tmp.right.push_back(Symbol(-1, 38));
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(-1, 37));
    tmp.right.push_back(Symbol(-1, 30));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //single-record-field-val-split = ':'
    tmp.left = 36;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 2));
    tmp.process = &Parser::process_single_record_field_val_split;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //record-field-val-split = ':'
    tmp.left = 37;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 2));
    tmp.process = &Parser::process_record_field_val_split;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //record-field-split = ';'
    tmp.left = 38;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_record_field_split;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-definition-part = 'var' var-definitions
    tmp.left = 39;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 49));
    tmp.right.push_back(Symbol(-1, 40));
    tmp.process = &Parser::process_newline;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //var-definitions = var-definition
    tmp.left = 40;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 41));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //var-definitions = var-definitions var-definition
    tmp.left = 40;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 40));
    tmp.right.push_back(Symbol(-1, 41));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //var-definition = var-type-declarification ';'
    tmp.left = 41;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 42));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_semicolon_newline;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //var-definition = var-type-declarification '=' M5 const-val ';'
    tmp.left = 41;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 42));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 43));
    tmp.right.push_back(Symbol(-1, 30));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_semicolon_newline;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //var-type-declarification = identifier-list ':' type-denoter
    tmp.left = 42;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 27));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 16));
    tmp.process = &Parser::process_var_type_declar;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M5 = ε
    tmp.left = 43;
    tmp.right.clear();
    tmp.process = &Parser::process_M5;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //procedure-definition-part = procedure-heading forward ';'
    tmp.left = 44;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 46));
    tmp.right.push_back(Symbol(0, 53));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_proc_func_declar;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //procedure-definition-part = procedure-heading proc-func-block ';'
    tmp.left = 44;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 46));
    tmp.right.push_back(Symbol(-1, 53));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_proc_func_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //function-definition-part = function-heading forward ';'
    tmp.left = 45;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 47));
    tmp.right.push_back(Symbol(0, 53));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_proc_func_declar;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //function-definition-part = function-heading proc-func-block ';'
    tmp.left = 45;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 47));
    tmp.right.push_back(Symbol(-1, 53));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_proc_func_def;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //procedure-heading = procedure identifier M6 ';'
    tmp.left = 46;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 35));
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(-1, 52));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_procedure_heading;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //procedure-heading = procedure identifier M6 '(' formal-parameter-list ')' ';'
    tmp.left = 46;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 35));
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(-1, 52));
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 48));
    tmp.right.push_back(Symbol(7, 5));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_procedure_heading;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //function-heading = function identifier M6 ':' no-array-type-identifier ';'
    tmp.left = 47;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 18));
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(-1, 52));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 51));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_function_heading;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //function-heading = function identifier M6 '(' formal-parameter-list ')' ':' no-array-type-identifier ';'
    tmp.left = 47;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 18));
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(-1, 52));
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 48));
    tmp.right.push_back(Symbol(7, 5));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 51));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_function_heading;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //formal-parameter-list = formal-parameter-specification
    tmp.left = 48;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 49));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //formal-parameter-list = formal-parameter-list ';' formal-parameter-specification
    tmp.left = 48;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 48));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(-1, 49));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //formal-parameter-specification = identifier-list ':' no-array-type-identifier
    tmp.left =49;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 27));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 51));
    tmp.process = &Parser::process_formal_parameter_spec;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //formal-parameter-specification = var identifier-list ':' type-identifier
    tmp.left = 49;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 49));
    tmp.right.push_back(Symbol(-1, 27));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 50));
    tmp.process = &Parser::process_formal_parameter_spec_var;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //formal-parameter-specification = const identifier-list ':' type-identifier
    tmp.left = 49;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 6));
    tmp.right.push_back(Symbol(-1, 27));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 50));
    tmp.process = &Parser::process_formal_parameter_spec_const;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-identifier = identifier
    tmp.left = 50;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_id_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-identifier = origin-type
    tmp.left = 50;
    tmp.right.clear();
    tmp.right.push_back(Symbol(1, 0));
    tmp.process = &Parser::process_basic_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //type-identifier = array of type-identifier
    tmp.left = 50;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 1));
    tmp.right.push_back(Symbol(0, 30));
    tmp.right.push_back(Symbol(-1, 50));
    tmp.process = &Parser::process_array_type_identifier;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //no-array-type-identifier = identifier
    tmp.left = 51;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_id_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //no-array-type-identifier = origin-type
    tmp.left = 51;
    tmp.right.clear();
    tmp.right.push_back(Symbol(1, 0));
    tmp.process = &Parser::process_basic_type_denoter;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M6 = ε
    tmp.left = 52;
    tmp.right.clear();
    tmp.process = &Parser::process_M6;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //proc-func-block = M7 proc-func-definition-part proc-func-statement-part
    tmp.left = 53;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 54));
    tmp.right.push_back(Symbol(-1, 55));
    tmp.right.push_back(Symbol(-1, 58));
    tmp.process = &Parser::process_proc_func_block;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M7 = ε
    tmp.left = 54;
    tmp.right.clear();
    tmp.process = &Parser::process_M7;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //proc-func-definition-part = ε
    tmp.left = 55;
    tmp.right.clear();
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //proc-func-definition-part = proc-func-definition-part label-declaration-part
    tmp.left = 55;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 55));
    tmp.right.push_back(Symbol(-1, 7));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //proc-func-definition-part = proc-func-definition-part constant-definition-part
    tmp.left = 55;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 55));
    tmp.right.push_back(Symbol(-1, 9));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //proc-func-definition-part = proc-func-definition-part type-definition-part
    tmp.left = 55;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 55));
    tmp.right.push_back(Symbol(-1, 12));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //proc-func-definition-part = proc-func-definition-part variable-definition-part
    tmp.left = 55;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 55));
    tmp.right.push_back(Symbol(-1, 39));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-statement-part = begin M8 statement-sequence end
    tmp.left = 56;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 3));
    tmp.right.push_back(Symbol(-1, 57));
    tmp.right.push_back(Symbol(-1, 59));
    tmp.right.push_back(Symbol(0, 14));
    tmp.process = &Parser::process_program_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M8 = ε
    tmp.left = 57;
    tmp.right.clear();
    tmp.process = &Parser::process_M8;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //proc-func-statement-part = begin statement-sequence end
    tmp.left = 58;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 3));
    tmp.right.push_back(Symbol(-1, 59));
    tmp.right.push_back(Symbol(0, 14));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement-sequence = statement-with-labels
    tmp.left = 59;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 60));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement-sequence = statement-sequence ';' statement-with-labels
    tmp.left = 59;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 59));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement-with-labels = label-part statement
    tmp.left = 60;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 61));
    tmp.right.push_back(Symbol(-1, 62));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-part = ε
    tmp.left = 61;
    tmp.right.clear();
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-part = label-part M9 integer-literal ':'
    tmp.left = 61;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 61));
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(3, 0));
    tmp.right.push_back(Symbol(7, 2));
    tmp.process = &Parser::process_label_part;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = ε
    tmp.left = 62;
    tmp.right.clear();
    tmp.process = &Parser::process_empty_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 variable-access ':=' expression
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(-1, 64));
    tmp.right.push_back(Symbol(6, 1));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.process = &Parser::process_assign_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 identifier
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_no_param_proc_func_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 identifier '(' expression-list ')'
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 72));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_proc_func_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 RTL-function
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(8, 0));
    tmp.process = &Parser::process_no_param_rtl_func_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 RTL-function '(' expression-list ')'
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(8, 0));
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 72));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_rtl_func_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = begin M12 statement-sequence end
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 3));
    tmp.right.push_back(Symbol(-1, 74));
    tmp.right.push_back(Symbol(-1, 59));
    tmp.right.push_back(Symbol(0, 14));
    tmp.process = &Parser::process_compound_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 goto integer-literal
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 19));
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_goto_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 if expression then M13 M11 statement-with-labels M14 else-part
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 20));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 42));
    tmp.right.push_back(Symbol(-1, 75));
    tmp.right.push_back(Symbol(-1, 73));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.right.push_back(Symbol(-1, 76));
    tmp.right.push_back(Symbol(-1, 77));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 case expression of M16 case-else-part end
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 5));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 30));
    tmp.right.push_back(Symbol(-1, 79));
    tmp.right.push_back(Symbol(-1, 80));
    tmp.right.push_back(Symbol(0, 14));
    tmp.process = &Parser::process_case_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 break
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 4));
    tmp.process = &Parser::process_break_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 continue
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 8));
    tmp.process = &Parser::process_continue_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 for variable-access ':=' expression to expression do M19 M11 statement-with-labels
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 17));
    tmp.right.push_back(Symbol(-1, 64));
    tmp.right.push_back(Symbol(6, 1));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 43));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 11));
    tmp.right.push_back(Symbol(-1, 85));
    tmp.right.push_back(Symbol(-1, 73));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.process = &Parser::process_for_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 for variable-access ':=' expression downto expression do M19 M11 statement-with-labels
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 17));
    tmp.right.push_back(Symbol(-1, 64));
    tmp.right.push_back(Symbol(6, 1));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 12));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 11));
    tmp.right.push_back(Symbol(-1, 85));
    tmp.right.push_back(Symbol(-1, 73));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.process = &Parser::process_for_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 while expression do M20 M11 statement-with-labels
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 50));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 11));
    tmp.right.push_back(Symbol(-1, 86));
    tmp.right.push_back(Symbol(-1, 73));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.process = &Parser::process_while_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //statement = M9 repeat M21 statement-sequence until expression
    tmp.left = 62;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 63));
    tmp.right.push_back(Symbol(0, 38));
    tmp.right.push_back(Symbol(-1, 87));
    tmp.right.push_back(Symbol(-1, 59));
    tmp.right.push_back(Symbol(0, 47));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.process = &Parser::process_repeat_statement;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M9 = ε
    tmp.left = 63;
    tmp.right.clear();
    tmp.process = &Parser::process_M9;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-access = identifier
    tmp.left = 64;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_id_var_access;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-access = variable-access '[' M10 array-index-list ']'
    tmp.left = 64;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 64));
    tmp.right.push_back(Symbol(7, 6));
    tmp.right.push_back(Symbol(-1, 65));
    tmp.right.push_back(Symbol(-1, 66));
    tmp.right.push_back(Symbol(7, 7));
    tmp.process = &Parser::process_array_var_access;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-access = variable-access '.' identifier
    tmp.left = 64;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 64));
    tmp.right.push_back(Symbol(7, 0));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &Parser::process_member_var_access;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-access = variable-access '^'
    tmp.left = 64;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 64));
    tmp.right.push_back(Symbol(6, 14));
    tmp.process = &Parser::process_pointer_var_access;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-access = identifier '(' expression-list ')'
    tmp.left = 64;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 72));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_proc_func_access;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-access = RTL-function
    tmp.left = 64;
    tmp.right.clear();
    tmp.right.push_back(Symbol(8, 0));
    tmp.process = &Parser::process_no_param_rtl_func_access;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //variable-access = RTL-function '(' expression-list ')'
    tmp.left = 64;
    tmp.right.clear();
    tmp.right.push_back(Symbol(8, 0));
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 72));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_rtl_func_access;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M10 = ε
    tmp.left = 65;
    tmp.right.clear();
    tmp.process = &Parser::process_M10;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-index-list = expression
    tmp.left = 66;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.process = &Parser::process_single_array_index_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //array-index-list = array-index-list ',' expression
    tmp.left = 66;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 66));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.process = &Parser::process_array_index_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //basic-expression = integer-literal
    tmp.left = 67;
    tmp.right.clear();
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &Parser::process_no_sign_signed_integer;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //basic-expression = float-literal
    tmp.left = 67;
    tmp.right.clear();
    tmp.right.push_back(Symbol(4, 0));
    tmp.process = &Parser::process_no_sign_signed_float;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //basic-expression = string
    tmp.left = 67;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 21));
    tmp.process = &Parser::process_string_const_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //basic-expression = true
    tmp.left = 67;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 44));
    tmp.process = &Parser::process_bool_const_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //basic-expression = false
    tmp.left = 67;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 15));
    tmp.process = &Parser::process_bool_const_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //basic-expression = variable-access
    tmp.left = 67;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 64));
    tmp.process = &Parser::process_single_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //basic-expression = '(' expression ')'
    tmp.left = 67;
    tmp.right.clear();
    tmp.right.push_back(Symbol(7, 4));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(7, 5));
    tmp.process = &Parser::process_bracket_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //first-expression = basic-expression
    tmp.left = 68;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 67));
    tmp.process = &Parser::process_single_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //first-expression = not first-expression
    tmp.left = 68;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 28));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_not_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //first-expression = '@' variable-access
    tmp.left = 68;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 15));
    tmp.right.push_back(Symbol(-1, 64));
    tmp.process = &Parser::process_address_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //first-expression = '+' first-expression
    tmp.left = 68;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 7));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_unary_add_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //first-expression = '-' first-expression
    tmp.left = 68;
    tmp.right.clear();
    tmp.right.push_back(Symbol(6, 8));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_unary_subtract_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_single_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression '*' first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(6, 9));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_multiply_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression '/' first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(6, 10));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_division_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression div first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(0, 10));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_integer_division_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression mod first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(0, 26));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_modulo_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression and first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(0, 0));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_and_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression shl first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(0, 40));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_left_shift_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression '<<' first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(6, 11));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_left_shift_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression shr first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(0, 41));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_right_shift_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //second-expression = second-expression '>>' first-expression
    tmp.left = 69;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.right.push_back(Symbol(6, 12));
    tmp.right.push_back(Symbol(-1, 68));
    tmp.process = &Parser::process_right_shift_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //third-expression = second-expression
    tmp.left = 70;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 69));
    tmp.process = &Parser::process_single_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //third-expression = third-expression '+' second-expression
    tmp.left = 70;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 70));
    tmp.right.push_back(Symbol(6, 7));
    tmp.right.push_back(Symbol(-1, 69));
    tmp.process = &Parser::process_add_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //third-expression = third-expression '-' second-expression
    tmp.left = 70;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 70));
    tmp.right.push_back(Symbol(6, 8));
    tmp.right.push_back(Symbol(-1, 69));
    tmp.process = &Parser::process_subtract_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //third-expression = third-expression or second-expression
    tmp.left = 70;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 70));
    tmp.right.push_back(Symbol(0, 33));
    tmp.right.push_back(Symbol(-1, 69));
    tmp.process = &Parser::process_or_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //third-expression = third-expression xor second-expression
    tmp.left = 70;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 70));
    tmp.right.push_back(Symbol(0, 52));
    tmp.right.push_back(Symbol(-1, 69));
    tmp.process = &Parser::process_xor_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_single_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = expression '=' third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(6, 0));
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_equal_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = expression '<>' third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(6, 2));
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_not_equal_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = expression '<' third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(6, 3));
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_less_than_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = expression '>' third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(6, 4));
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_greater_than_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = expression '<=' third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(6, 5));
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_less_than_equal_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = expression '>=' third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(6, 6));
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_greater_than_equal_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression = expression in third-expression
    tmp.left = 71;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.right.push_back(Symbol(0, 22));
    tmp.right.push_back(Symbol(-1, 70));
    tmp.process = &Parser::process_in_expression;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression-list = expression
    tmp.left = 72;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 71));
    tmp.process = &Parser::process_single_expression_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //expression-list = expression-list ',' expression
    tmp.left = 72;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 72));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(-1, 71));
    tmp.process = &Parser::process_expression_list;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M11 = ε
    tmp.left = 73;
    tmp.right.clear();
    tmp.process = &Parser::process_M11;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M12 = ε
    tmp.left = 74;
    tmp.right.clear();
    tmp.process = &Parser::process_M12;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M13 = ε
    tmp.left = 75;
    tmp.right.clear();
    tmp.process = &Parser::process_M13;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M14 = ε
    tmp.left = 76;
    tmp.right.clear();
    tmp.process = &Parser::process_M14;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //else-part = ε
    tmp.left = 77;
    tmp.right.clear();
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //else-part = else M15 M11 statement-with-labels M14
    tmp.left = 77;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 13));
    tmp.right.push_back(Symbol(-1, 78));
    tmp.right.push_back(Symbol(-1, 73));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.right.push_back(Symbol(-1, 76));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M15 = ε
    tmp.left = 78;
    tmp.right.clear();
    tmp.process = &Parser::process_M15;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M16 = ε
    tmp.left = 79;
    tmp.right.clear();
    tmp.process = &Parser::process_M16;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //case-else-part = case-part
    tmp.left = 80;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 81));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //case-else-part = case-part ';'
    tmp.left = 80;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 81));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //case-else-part = case-part else-part-of-case
    tmp.left = 80;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 81));
    tmp.right.push_back(Symbol(-1, 82));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //case-else-part = case-part ';' else-part-of-case
    tmp.left = 80;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 81));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(-1, 82));
    tmp.process = &Parser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //case-part = array-subrange-case-index ':' M17 statement-with-labels
    tmp.left = 81;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 22));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 83));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.process = &Parser::process_case_part;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //case-part = case-part ';' array-subrange-case-index ':' M17 statement-with-labels
    tmp.left = 81;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 81));
    tmp.right.push_back(Symbol(7, 3));
    tmp.right.push_back(Symbol(-1, 22));
    tmp.right.push_back(Symbol(7, 2));
    tmp.right.push_back(Symbol(-1, 83));
    tmp.right.push_back(Symbol(-1, 60));
    tmp.process = &Parser::process_case_part;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //else-part-of-case = else M18 statement-sequence
    tmp.left = 82;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 13));
    tmp.right.push_back(Symbol(-1, 84));
    tmp.right.push_back(Symbol(-1, 59));
    tmp.process = &Parser::process_case_part;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //else-part-of-case = otherwise M18 statement-sequence
    tmp.left = 82;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 54));
    tmp.right.push_back(Symbol(-1, 84));
    tmp.right.push_back(Symbol(-1, 59));
    tmp.process = &Parser::process_case_part;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1); 
    //M17 = ε
    tmp.left = 83;
    tmp.right.clear();
    tmp.process = &Parser::process_M17;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M18 = ε
    tmp.left = 84;
    tmp.right.clear();
    tmp.process = &Parser::process_M18;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M19 = ε
    tmp.left = 85;
    tmp.right.clear();
    tmp.process = &Parser::process_M19;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M20 = ε
    tmp.left = 86;
    tmp.right.clear();
    tmp.process = &Parser::process_M20;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M21 = ε
    tmp.left = 87;
    tmp.right.clear();
    tmp.process = &Parser::process_M21;
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
                        if (! parsing_table[i][*it1].first) // 解决悬空-else问题
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
            if (cur_token.category == END_OF_TOKENS)
                output_error(-1, -1, -1, "unexpected end of file");
            else
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
            Token new_token;
            if (! grammar[generation_no].right.empty())
                new_token = Token(-1, grammar[generation_no].left, 
                    parsing_stack[parsing_stack.size() - grammar[generation_no].right.size()].second.line, 
                    parsing_stack[parsing_stack.size() - grammar[generation_no].right.size()].second.col, 
                    parsing_stack[parsing_stack.size() - grammar[generation_no].right.size()].second.pos);
            else
                new_token = Token(-1, grammar[generation_no].left,
                    parsing_stack.back().second.line, parsing_stack.back().second.col, parsing_stack.back().second.pos);
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