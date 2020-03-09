#include <iostream>
#include <queue>
#include "args.h"
#include "lalrparser.h"


int LalrParser::num_nonterminal = 8;

LalrParser::LalrParser() {
    symbol_table.parent = NULL;
    current_symbol_table = &symbol_table;
    grammar_init();
    cal_first();
    cal_collection_of_sets_of_items();
    cal_parsing_table();
}

LalrParser::~LalrParser() {}

bool LalrParser::process_default() {
    return true;
}

bool LalrParser::process_M1() {
    result << "#include <bits/stdc++.h>\n";
    return true;
}

bool LalrParser::process_label() {
    std::string label = parsing_stack.back().content;
    if (label.length() > 4) {
        output_error(parsing_stack.back().line, parsing_stack.back().col, parsing_stack.back().pos, "label syntax error");
        return false;
    }
    for (int i = 0; i < label.length(); i++)
        if (! isdigit(label[i])) {
            output_error(parsing_stack.back().line, parsing_stack.back().col, parsing_stack.back().pos, "label syntax error");
            return false;
        }
    current_symbol_table->labels.insert(label);
    return true;
}

void LalrParser::grammar_init() {
    Generation tmp;
    //program' = program
    tmp.left = 0;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 1));
    tmp.process = &LalrParser::process_default;
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
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program = M1 program-block '.'
    tmp.left = 1;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 2));
    tmp.right.push_back(Symbol(-1, 4));
    tmp.right.push_back(Symbol(7, 0));
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //M1 = ε
    tmp.left = 2;
    tmp.right.clear();
    tmp.process = &LalrParser::process_M1;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-heading = 'program' identifier
    tmp.left = 3;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 36));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &LalrParser::process_default;
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
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //identifier-list = identifier
    tmp.left = 5;
    tmp.right.clear();
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //identifier-list = identifier-list ',' identifier
    tmp.left = 5;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 5));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(2, 0));
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //program-block = label-declaration
    tmp.left = 4;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 6));
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-declaration = ε
    tmp.left = 6;
    tmp.right.clear();
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-declaration = 'label' label-list ';'
    tmp.left = 6;
    tmp.right.clear();
    tmp.right.push_back(Symbol(0, 25));
    tmp.right.push_back(Symbol(-1, 7));
    tmp.right.push_back(Symbol(7, 3));
    tmp.process = &LalrParser::process_default;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-list = label
    tmp.left = 7;
    tmp.right.clear();
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &LalrParser::process_label;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
    //label-list = label-list ',' label
    tmp.left = 7;
    tmp.right.clear();
    tmp.right.push_back(Symbol(-1, 7));
    tmp.right.push_back(Symbol(7, 1));
    tmp.right.push_back(Symbol(3, 0));
    tmp.process = &LalrParser::process_label;
    grammar.push_back(tmp);
    nonterminal_grammar[tmp.left].push_back(grammar.size() - 1);
}

int LalrParser::get_matrix_idx(Symbol symbol) {
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

Symbol LalrParser::get_symbol(int num) {
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

void LalrParser::cal_first() {
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

std::set<LalrParser::Item> LalrParser::closure(const std::set<Item> &s) {
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

void LalrParser::cal_collection_of_sets_of_items() {
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

void LalrParser::cal_parsing_table() {
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

bool LalrParser::parse(std::vector<Token> tokens) {
    token_list = tokens;
    return true;
}

std::string LalrParser::get_result() const {
    return result.str();
}