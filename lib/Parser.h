//
// Created by 周霄 on 2023/9/24.
//

#ifndef COMPILER_PARSER_H
#define COMPILER_GRAMMATICAL_ANALYSIS_H


#include <string>
#include <vector>
#include "SymbolTable.h"

class Grammar: public TreeNode{
public:
    int start_id;
    int end_id;
    std::string type;
    Grammar* parent;
    std::vector<Grammar*> child_list;
    Grammar(int start_id, const std::string& type);
};

class Parser {
public:
    static std::vector<std::string> type_list;
    Grammar* grammar_tree;
    TreeNode* grammar_tree_root;
    std::vector<Error> error_list;
    Compiler* compiler;
    explicit Parser(Compiler* compiler);
    void analyse();

private:
    int index;
    Word sym;
    std::ofstream outfile;
    void nextsym();
    void lastsym();
    [[nodiscard]] Word* fade_sym() const;
    [[nodiscard]] Word* fade_sym(int i) const;
    void create_grammar_node(int start_id, const std::string& type);
    void update_grammar_node(int end_id);
    void create_grammar_parent_node(int start_id, const std::string& type);
    void preorder_print();
    void preorder_build_tree();
    void preorder_print2(TreeNode* node);

    void print();
    void debug_print();

    void analyse_CompUnit();
    void analyse_Decl();
    void analyse_ConstDecl();
    void analyse_BType();
    void analyse_ConstDef();
    void analyse_ConstInitVal();
    void analyse_VarDecl();
    void analyse_VarDef();
    void analyse_InitVal();
    void analyse_FuncDef();
    void analyse_MainFuncDef();
    void analyse_FuncType();
    void analyse_FuncFParams();
    void analyse_FuncFParam();
    void analyse_Block();
    void analyse_BlockItem();
    void analyse_Stmt(bool for_flag);
    void analyse_Stmt_for(Word *temp);
    void analyse_Stmt_idenfr(Word *temp);
    void analyse_ForStmt();
    void analyse_Exp();
    void analyse_Cond();
    void analyse_LVal();
    void analyse_PrimaryExp();
    void analyse_Number();
    void analyse_UnaryExp();
    void analyse_UnaryOp();
    void analyse_FuncRParams();
    void analyse_MulExp();
    void analyse_AddExp();
    void analyse_RelExp();
    void analyse_EqExp();
    void analyse_LAndExp();
    void analyse_LOrExp();
    void analyse_ConstExp();

    SymbolTable* current_table;
    Symbol current_symbol;
    Symbol temp_current_symbol;
    int temp_current_line;
    void check_symbol(const std::string& name);
    void check_const_symbol(const std::string& name, int line);
    int get_return_type();
    void push_symbol();
    void push_symbol(Symbol symbol);
    void push_symbol(SymbolTable* table);
};


#endif //COMPILER_PARSER_H
