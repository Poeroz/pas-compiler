#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <set>
#include "args.h"

class Parser {
public:
    Parser();
    ~Parser();
    bool parse(std::vector<Token> tokens);
    std::string get_result() const;
private:
    std::vector<Token> token_list;
    static int num_nonterminal;
    struct Generation {
        int left;
        std::vector<Symbol> right;
        bool (Parser::*process)(Token&);
    };
    std::vector<Generation> grammar;
    std::map<int, std::vector<int> > nonterminal_grammar;
    std::vector<std::pair<int, Token> > parsing_stack;
    void grammar_init();
    int indent;
    std::pair<int, std::string> pas_int_to_c(int sign, std::string s) const;
    std::pair<int, std::string> pas_string_to_c(std::string s) const;
    std::string pas_basic_type_to_c(int type_no) const;
    std::string id_with_type(Type *type, std::vector<std::string> id_list, std::string struct_name = "");
    bool type_match(Type *a, Type *b, int match_type = 1) const; // 0 -- exclusive 1 -- match
    bool functype_match(FuncType *a, FuncType *b, int match_type = 1) const; // 0 -- exclusive 1 -- match
    Type *arithmetic_type_check(Type *a, Type *b);
    bool process_default(Token &new_token);
    bool process_newline(Token &new_token);
    bool process_semicolon_newline(Token &new_token);
    bool process_indent(Token &new_token);
    bool process_M1(Token &new_token);
    bool process_label(Token &new_token);
    bool process_int_constant_def(Token &new_token);
    bool process_float_constant_def(Token &new_token);
    bool process_string_constant_def(Token &new_token);
    bool process_bool_constant_def(Token &new_token);
    bool process_typed_constant_def(Token &new_token);
    bool process_type_def(Token &new_token);
    bool process_type_name(Token &new_token);
    bool process_id_type_denoter(Token &new_token);
    bool process_basic_type_denoter(Token &new_token);
    bool process_pointer_type_denoter(Token &new_token);
    bool process_array_type_denoter(Token &new_token);
    bool process_enum_type_denoter(Token &new_token);
    bool process_record_type_denoter(Token &new_token);
    bool process_set_type_denoter(Token &new_token);
    bool process_array_single_subrange_list(Token &new_token);
    bool process_array_subrange_list(Token &new_token);
    bool process_array_subrange(Token &new_token);
    bool process_no_sign_signed_integer(Token &new_token);
    bool process_signed_integer(Token &new_token);
    bool process_no_sign_signed_float(Token &new_token);
    bool process_signed_float(Token &new_token);
    bool process_string(Token &new_token);
    bool process_array_subrange_index(Token &new_token);
    bool process_single_enum_list(Token &new_token);
    bool process_enum_list(Token &new_token);
    bool process_enum_item(Token &new_token);
    bool process_single_field_list(Token &new_token);
    bool process_field_list(Token &new_token);
    bool process_record_section(Token &new_token);
    bool process_single_id_list(Token &new_token);
    bool process_id_list(Token &new_token);
    bool process_const_type_declar(Token &new_token);
    bool process_M2(Token &new_token);
    bool process_num_const_val(Token &new_token);
    bool process_string_const_val(Token &new_token);
    bool process_bool_const_val(Token &new_token);
    bool process_array_const_val(Token &new_token);
    bool process_record_const_val(Token &new_token);
    bool process_M3(Token &new_token);
    bool process_single_array_vals(Token &new_token);
    bool process_array_vals(Token &new_token);
    bool process_array_element_split(Token &new_token);
    bool process_M4(Token &new_token);
    bool process_single_record_field_val_split(Token &new_token);
    bool process_record_field_val_split(Token &new_token);
    bool process_record_field_split(Token &new_token);
    bool process_var_type_declar(Token &new_token);
    bool process_M5(Token &new_token);
    bool process_proc_func_declar(Token &new_token);
    bool process_proc_func_def(Token &new_token);
    bool process_procedure_heading(Token &new_token);
    bool process_function_heading(Token &new_token);
    bool process_formal_parameter_spec(Token &new_token);
    bool process_formal_parameter_spec_var(Token &new_token);
    bool process_formal_parameter_spec_const(Token &new_token);
    bool process_array_type_identifier(Token &new_token);
    bool process_M6(Token &new_token);
    bool process_proc_func_block(Token &new_token);
    bool process_M7(Token &new_token);
    bool process_program_statement(Token &new_token);
    bool process_M8(Token &new_token);
    bool process_label_part(Token &new_token);
    bool process_assign_statement(Token &new_token);
    bool process_id_var_access(Token &new_token);
    bool process_array_var_access(Token &new_token);
    bool process_member_var_access(Token &new_token);
    bool process_pointer_var_access(Token &new_token);
    bool process_M10(Token &new_token);
    bool process_single_array_index_list(Token &new_token);
    bool process_array_index_list(Token &new_token);
    bool process_string_const_expression(Token &new_token);
    bool process_bool_const_expression(Token &new_token);
    bool process_single_expression(Token &new_token);
    bool process_bracket_expression(Token &new_token);
    bool process_not_expression(Token &new_token);
    bool process_address_expression(Token &new_token);
    bool process_unary_add_expression(Token &new_token);
    bool process_unary_subtract_expression(Token &new_token);
    bool process_multiply_expression(Token &new_token);
    bool process_division_expression(Token &new_token);
    bool process_integer_division_expression(Token &new_token);
    bool process_modulo_expression(Token &new_token);
    bool process_and_expression(Token &new_token);
    bool process_left_shift_expression(Token &new_token);
    bool process_right_shift_expression(Token &new_token);
    bool process_add_expression(Token &new_token);
    bool process_subtract_expression(Token &new_token);
    bool process_or_expression(Token &new_token);
    bool process_xor_expression(Token &new_token);
    bool process_equal_expression(Token &new_token);
    bool process_not_equal_expression(Token &new_token);
    bool process_less_than_expression(Token &new_token);
    bool process_greater_than_expression(Token &new_token);
    bool process_less_than_equal_expression(Token &new_token);
    bool process_greater_than_equal_expression(Token &new_token);
    bool process_in_expression(Token &new_token);
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