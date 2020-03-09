#ifndef LALRPARSER_H
#define LALRPARSER_H

#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include "args.h"

class LalrParser {
public:
    LalrParser();
    ~LalrParser();
    bool parse(std::vector<Token> tokens);
    std::string get_result() const;
private:
    std::vector<Token> token_list;
    static int num_nonterminal;
    struct Generation {
        int left;
        std::vector<Symbol> right;
        bool (LalrParser::*process)();
    };
    std::vector<Generation> grammar;
    std::map<int, std::vector<int> > nonterminal_grammar;
    std::vector<Token> parsing_stack;
    void grammar_init();
    bool process_default();
    bool process_M1();
    bool process_label();
    std::vector<bool> empty;
    std::vector<std::vector<Symbol> > first;
    int get_matrix_idx(Symbol symbol);
    Symbol get_symbol(int num);
    struct Matrix {
        std::vector<std::vector<bool> > val;
        std::vector<bool> &operator [] (int ind) {
            return val[ind];
        }
        const std::vector<bool> &operator [] (int ind) const {
            return val[ind];
        }
        void reset() {
            val.resize(num_nonterminal + num_terminal);
            for (int i = 0; i < num_nonterminal + num_terminal; i++) {
                val[i].resize(num_nonterminal + num_terminal);
                for (int j = 0; j < num_nonterminal + num_terminal; j++)
                    val[i][j] = false;
            }
        }
        void transitive_closure() {
            for (int k = 0; k < num_nonterminal + num_terminal; k++)
                for (int i = 0; i < num_nonterminal + num_terminal; i++)
                    for (int j = 0; j < num_nonterminal + num_terminal; j++)
                        val[i][j] = val[i][j] || (val[i][k] && val[k][j]);
        }
    };
    Matrix matrix_first;
    void cal_first();
    struct Item {
        int generation_no, dot_pos;
        std::set<Symbol> lookahead;
        Item() {}
        Item(int generation_no, int dot_pos) :
            generation_no(generation_no), dot_pos(dot_pos) {}
        bool operator < (const Item &_) const {
            return generation_no == _.generation_no ? dot_pos < _.dot_pos : generation_no < _.generation_no;
        }
        bool operator == (const Item &_) const {
            return generation_no == _.generation_no && dot_pos == _.dot_pos;
        }
    };
    std::map<std::set<Item>, int> set_of_items_no;
    std::vector<std::set<Item> > set_of_items;
    int state_num;
    std::vector<std::map<Symbol, int> > transfer;
    std::set<Item> closure(const std::set<Item> &s);
    /*
        0: error
        1: shift / goto
        2: reduce
        3: accept
    */
    std::vector<std::map<Symbol, std::pair<int, int> > > parsing_table;
    void cal_collection_of_sets_of_items();
    void cal_parsing_table();
    SymbolTable symbol_table;
    SymbolTable *current_symbol_table;
    std::ostringstream result;
};


#endif