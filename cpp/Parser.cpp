//
// Created by 周霄 on 2023/9/24.
//

#include "../lib/Parser.h"
#include "stack"
#include <algorithm>
#include "iterator"

Grammar::Grammar(int start_id, const std::string &type) {
    node_type = 1;
    this->start_id = start_id;
    this->end_id = -1;
    this->type = type;
    this->parent = nullptr;
}

std::vector<std::string> Parser::type_list = {
        "<CompUnit>", "<Decl>", "<ConstDecl>", "<BType>", "<ConstDef>", "<ConstInitVal>", "<VarDecl>", "<VarDef>",
        "<InitVal>", "<FuncDef>", "<MainFuncDef>", "<FuncType>", "<FuncFParams>", "<FuncFParam>",
        "<Block>", "<Stmt>", "<ForStmt>", "<Exp>",
        "<Cond>", "<LVal>", "<PrimaryExpr>", "<Number>", "<UnaryOp>", "<UnaryExp>", "<FuncRParams>", "<MulExp>",
        "<AddExp>", "<RelExp>", "<EqExp>", "<LAndExp>", "<LOrExp>", "<ConstExp>"
};

Parser::Parser(Compiler* compiler) {
    this->compiler = compiler;
    this->current_table = compiler->symbol_table;
    this->index = 0;
    this->temp_current_symbol = Symbol();
    this->temp_current_symbol.word = new Word();
    this->grammar_tree = nullptr;
    this->grammar_tree_root = nullptr;
    temp_current_line = 0;
}

void Parser::debug_print() {
    
}

void Parser::print() {
    try {
        outfile.open(compiler->lexer->outfile_path);
//    preorder_print();
        preorder_build_tree();
        preorder_print2(this->grammar_tree_root);
        outfile.close();
    } catch (TraceException &e) {
        e.add_trace_back("print");
        throw;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("print");
        throw TraceException(e2);
    }
}

void Parser::nextsym() {
    sym = compiler->lexer->wordlist[index++];
}

void Parser::lastsym() {
    sym = compiler->lexer->wordlist[(--index)-1];
}

Word* Parser::fade_sym() const {
    return &compiler->lexer->wordlist[index];
}

Word* Parser::fade_sym(int i) const {
    return &compiler->lexer->wordlist[index + i];
}

void Parser::create_grammar_node(int start_id, const std::string &type) {
    auto* grammar_ = new Grammar(index, type);
    if (this->grammar_tree == nullptr){
        this->grammar_tree = grammar_;
        grammar_->parent = grammar_;
    }else{
        grammar_->parent = this->grammar_tree;
        this->grammar_tree->child_list.push_back(grammar_);
        this->grammar_tree = grammar_;
    }
}

void Parser::update_grammar_node(int end_id) {
    this->grammar_tree->end_id = end_id;
    this->grammar_tree = this->grammar_tree->parent;
}

void Parser::create_grammar_parent_node(int start_id, const std::string &type) {
    auto* grammar_ = new Grammar(start_id, type);
    this->grammar_tree->parent->child_list.pop_back();
    grammar_->parent = this->grammar_tree->parent;
    this->grammar_tree->parent->child_list.push_back(grammar_);
    this->grammar_tree->parent = grammar_;
    grammar_->child_list.push_back(this->grammar_tree);
    this->grammar_tree = grammar_;
}

void Parser::preorder_print() {
    std::stack<Grammar *> stack0;
    std::stack<int> stack1;
    Grammar *temp_grammar = nullptr;
    stack0.push(this->grammar_tree);
    stack1.push(0);
    while (!stack0.empty()) {
        if (stack0.top()->child_list.empty() || stack1.top() == stack0.top()->child_list.size()) {
            if (stack0.top()->child_list.empty()) {
                for (int i = stack0.top()->start_id; i <= stack0.top()->end_id; i++) {
                        outfile << compiler->lexer->wordlist[i].type << " " << compiler->lexer->wordlist[i].value
//                                << "  " << i << "  1"<< std::endl;
                                << std::endl;
                }
            } else {
                for (int i = stack0.top()->child_list.back()->end_id + 1; i <= stack0.top()->end_id; i++) {
                        outfile << compiler->lexer->wordlist[i].type << " " << compiler->lexer->wordlist[i].value
//                                << "  " << i << "  2"<< std::endl;
                                << std::endl;
                }
            }
            std::string str = stack0.top()->type;
            if (str != "<BlockItem>" && str != "<Decl>" && str != "<BType>") {
//                outfile << str << "  " << stack0.top()->start_id << "  "<< stack0.top()->end_id << std::endl;
                outfile << str << std::endl;
            }
            temp_grammar = stack0.top();
            stack0.pop();
            stack1.pop();
        } else {
            stack1.top()++;
            stack0.push(stack0.top()->child_list[stack1.top() - 1]);
            stack1.push(0);
            if (temp_grammar != nullptr) {
                if (stack0.top()->start_id - temp_grammar->end_id >= 2) {
                    for (int i = temp_grammar->end_id + 1; i < stack0.top()->start_id; i++) {
                        outfile << compiler->lexer->wordlist[i].type << " " << compiler->lexer->wordlist[i].value
//                                << "  " << i << "  3"<< std::endl;
                                << std::endl;
                    }
                } else if (temp_grammar->start_id < stack0.top()->start_id && temp_grammar->end_id>=stack0.top()->end_id){
                    for (int i = temp_grammar->start_id ; i < stack0.top()->start_id; i++) {
                        outfile << compiler->lexer->wordlist[i].type << " " << compiler->lexer->wordlist[i].value
//                                << "  " << i << "  4 : temp_grammar : "<<
//                                temp_grammar->start_id << "  " << temp_grammar->end_id << " stack0_top : " <<
//                                stack0.top()->start_id << "  " << stack0.top()->end_id << std::endl;
                                << std::endl;
                    }
                }
                temp_grammar = stack0.top();
            }
        }
    }
}

void Parser::preorder_build_tree() {
    this->grammar_tree_root = this->grammar_tree;
    std::stack<int> stack1;
    Grammar *temp_grammar = this->grammar_tree;
    Grammar *temp_temp_grammar = nullptr;
    stack1.push(-1);
    while (!stack1.empty()) {
        if(!temp_grammar->child_list.empty()){
            if (stack1.top() == -1){
                temp_temp_grammar = temp_grammar->child_list[0];
                for (int i = temp_grammar->start_id; i < temp_temp_grammar->start_id; i++) {
                    this->grammar_tree_root->child_node_list.push_back(&compiler->lexer->wordlist[i]);
                }
                stack1.top()++;
                this->grammar_tree_root->child_node_list.push_back(temp_temp_grammar);
                this->grammar_tree_root = temp_temp_grammar;
                temp_grammar = temp_temp_grammar;
                stack1.push(-1);
            }else if (stack1.top() < temp_grammar->child_list.size()-1) {
                temp_temp_grammar = temp_grammar->child_list[stack1.top() + 1];
                for (int i = temp_grammar->child_list[stack1.top()]->end_id + 1; i < temp_temp_grammar->start_id; i++) {
                    this->grammar_tree_root->child_node_list.push_back(&compiler->lexer->wordlist[i]);
                }
                stack1.top()++;
                this->grammar_tree_root->child_node_list.push_back(temp_temp_grammar);
                this->grammar_tree_root = temp_temp_grammar;
                temp_grammar = temp_temp_grammar;
                stack1.push(-1);
            }else{
                temp_temp_grammar = temp_grammar->child_list[stack1.top()];
                for (int i = temp_grammar->child_list[stack1.top()]->end_id + 1; i <= temp_grammar->end_id; i++) {
                    this->grammar_tree_root->child_node_list.push_back(&compiler->lexer->wordlist[i]);
                }
                stack1.pop();
                temp_grammar = temp_grammar->parent;
                this->grammar_tree_root = temp_grammar;
            }
        }else{
            for (int i = temp_grammar->start_id; i <= temp_grammar->end_id; i++) {
                this->grammar_tree_root->child_node_list.push_back(&compiler->lexer->wordlist[i]);
            }
            stack1.pop();
            temp_grammar = temp_grammar->parent;
            this->grammar_tree_root = temp_grammar;
        }
    }
    while (((Grammar*)this->grammar_tree_root)->parent != this->grammar_tree_root){
        this->grammar_tree_root = ((Grammar*)this->grammar_tree_root)->parent;
    }
}

void Parser::preorder_print2(TreeNode* node) {
    if (node->child_node_list.empty()){
        std::cout << 1111 << std::endl;
        return;
    }
    for (auto & i : node->child_node_list) {
        if(i->node_type == 1) {
            preorder_print2(i);
        }else{
            outfile << ((Word*)i)->type << " " << ((Word*)i)->value  << std::endl;
        }
    }
    std::string str = ((Grammar*) node)->type;
    if (str != "<BlockItem>" && str != "<Decl>" && str != "<BType>") {
        outfile << str << std::endl;
    }

}


void Parser::analyse() {
    try {
        analyse_CompUnit();
        print();
    }catch (TraceException &e) {
        e.add_trace_back("compile");
        throw;
    }catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("compile");
        throw TraceException(e2);
    }
//    debug_print();
}

void Parser::analyse_CompUnit() {
    try {
        create_grammar_node(index, "<CompUnit>");
        Word *temp = fade_sym();
        while (true) {
            int i = 0;
            temp = fade_sym();
            if (temp->type == "CONSTTK") {
                analyse_Decl();
            } else if (temp->type == "VOIDTK") {
                analyse_FuncDef();
            } else if (temp->type == "INTTK") {
                Word *temp2 = fade_sym(1);
                if (temp2->type == "IDENFR") {
                    temp2 = fade_sym(2);
                    if (temp2->type == "LPARENT") {
                        analyse_FuncDef();
                    } else {
                        analyse_Decl();
                    }
                } else if (temp2->type == "MAINTK") {
                    analyse_MainFuncDef();
                    break;
                } else {
                    std::cout << "<CompUnit>Error: " << sym.value << "   " << sym.type << std::endl;
                    break;
                }
            } else {
                std::cout << "<CompUnit>Error: " << sym.value << "   " << sym.type << std::endl;
                break;
            }
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("CompUnit");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("CompUnit");
        throw TraceException(e2);
    }
}

void Parser::analyse_Decl() {
    try {
        create_grammar_node(index, "<Decl>");
        Word *temp = fade_sym();
        if (temp->type == "CONSTTK") {
            analyse_ConstDecl();
        } else if (temp->type == "INTTK") {
            analyse_VarDecl();
        } else {
            std::cout << "<Decl>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("Decl");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Decl");
        throw TraceException(e2);
    }
}

void Parser::analyse_ConstDecl() {
    try {
        create_grammar_node(index, "<ConstDecl>");
        nextsym();
        current_symbol = Symbol();
        current_symbol.con = 1;
        analyse_BType();
        analyse_ConstDef();
        nextsym();
        while (sym.type == "COMMA") {
            analyse_ConstDef();
            nextsym();
        }
        if (sym.type != "SEMICN") {
            std::cout << "<ConstDecl>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
            error_list.push_back(error);
            lastsym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("ConstDecl");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("ConstDecl");
        throw TraceException(e2);
    }
}

void Parser::analyse_BType() {
    try {
        create_grammar_node(index, "<BType>");
        nextsym();
        if (sym.type != "INTTK") {
            std::cout << "<BType>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("BType");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("BType");
        throw TraceException(e2);
    }
}

void Parser::analyse_ConstDef() {
    try {
        create_grammar_node(index,"<ConstDef>");
        nextsym();
        if (sym.type != "IDENFR") {
            std::cout << "<ConstDef>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        current_symbol.word = fade_sym(-1);
        nextsym();
        int list_num = 0;
        while (sym.type == "LBRACK") {
            list_num++;
            if (list_num >= 3) {
                std::cout << "<ConstDef>Error: " << sym.value << "   " << std::endl;
            }
            analyse_ConstExp();
            nextsym();
            if (sym.type != "RBRACK") {
                std::cout << "<ConstDef>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "k", "缺少右中括号", "缺少右中括号");
                error_list.push_back(error);
                lastsym();
            }
            nextsym();
        }
        current_symbol.type = list_num;
        push_symbol();
        if (sym.type != "ASSIGN") {
            std::cout << "<ConstDef>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        analyse_ConstInitVal();
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("ConstDef");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("ConstDef");
        throw TraceException(e2);
    }
}

void Parser::analyse_ConstInitVal() {
    try {
        create_grammar_node(index, "<ConstInitVal>");
        Word *temp = fade_sym();
        if (temp->type != "LBRACE") {
            analyse_ConstExp();
        } else {
            nextsym();
            temp = fade_sym();
            if (temp->type == "RBRACE") {
            } else {
                analyse_ConstInitVal();
                temp = fade_sym();
                while (temp->type == "COMMA") {
                    nextsym();
                    analyse_ConstInitVal();
                    temp = fade_sym();
                }
                nextsym();
                if (sym.type != "RBRACE") {
                    std::cout << "<ConstInitVal>Error: " << sym.value << "   " << sym.type << std::endl;
                }
            }
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("ConstInitVal");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("ConstInitVal");
        throw TraceException(e2);
    }
}

void Parser::analyse_VarDecl() {
    try {
        create_grammar_node(index,"<VarDecl>");
        current_symbol = Symbol();
        current_symbol.con = 0;
        analyse_BType();
        analyse_VarDef();
        nextsym();
        while (sym.type == "COMMA") {
            analyse_VarDef();
            nextsym();
        }
        if (sym.type != "SEMICN") {
            std::cout << "<VarDecl>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
            error_list.push_back(error);
            lastsym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("VarDecl");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("VarDecl");
        throw TraceException(e2);
    }
}

void Parser::analyse_VarDef() {
    try {
        Grammar grammar(index, "<VarDef>");
        create_grammar_node(index,"<VarDef>");
        nextsym();
        if (sym.type != "IDENFR") {
            std::cout << "<VarDef>Error: " << sym.value << "   " << sym.type << std::endl;
            grammar.start_id = -1;
        }
        current_symbol.word = fade_sym(-1);
        Word *temp = fade_sym();
        int list_num = 0;
        while (temp->type == "LBRACK") {
            nextsym();
            list_num++;
            if (list_num >= 3) {
                std::cout << "<VarDef>Error: " << sym.value << "   " << std::endl;
                grammar.start_id = -1;
            }
            analyse_ConstExp();
            nextsym();
            if (sym.type != "RBRACK") {
                std::cout << "<VarDef>Error: " << sym.value << "   " << sym.type << std::endl;
                grammar.start_id = -1;
                Error error(fade_sym(-2)->line, "k", "缺少右中括号", "缺少右中括号");
                error_list.push_back(error);
                lastsym();
            }
            temp = fade_sym();
        }
        current_symbol.type = list_num;
        push_symbol();
        if (temp->type == "ASSIGN") {
            nextsym();
            analyse_InitVal();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("VarDef");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("VarDef");
        throw TraceException(e2);
    }
}

void Parser::analyse_InitVal() {
    try {
        create_grammar_node(index, "<InitVal>");
        Word *temp = fade_sym();
        if (temp->type == "PLUS" || temp->type == "MINU" || temp->type == "NOT" ||
            temp->type == "IDENFR" || temp->type == "INTCON" || temp->type == "LPARENT") {
            analyse_Exp();
        } else if (temp->type == "LBRACE") {
            nextsym();
            temp = fade_sym();
            while (temp->type != "RBRACE") {
                analyse_InitVal();
                temp = fade_sym();
                if (temp->type == "COMMA") {
                    nextsym();
                    temp = fade_sym();
                } else if (temp->type == "RBRACE") {
                    nextsym();
                    break;
                } else {
                    std::cout << "<InitVal>Error: " << sym.value << "   " << sym.type << std::endl;
                }
            }
        } else {
            std::cout << "<InitVal>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("InitVal");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("InitVal");
        throw TraceException(e2);
    }
}

void Parser::analyse_FuncDef() {
    try {
        create_grammar_node(index, "<FuncDef>");
        current_symbol = Symbol();
        current_symbol.con = -1;
        current_symbol.parent_table = current_table;
        current_symbol.self_table = new SymbolTable(current_table);
        current_table = current_symbol.self_table;
        analyse_FuncType();
        nextsym();
        if (sym.type != "IDENFR") {
            std::cout << "<FuncDef>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        current_symbol.word = fade_sym(-1);
        current_symbol.self_table->name = current_symbol.word->value;
        nextsym();
        if (sym.type != "LPARENT") {
            std::cout << "<FuncDef>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        Word *temp = fade_sym();
        if (temp->type == "INTTK") {
            analyse_FuncFParams();
        }
        nextsym();
        if (sym.type != "RPARENT") {
            std::cout << "<FuncDef>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
            error_list.push_back(error);
            lastsym();
        }
        push_symbol(current_symbol.parent_table);
        analyse_Block();
        current_table = current_table->parent_table;
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("FuncDef");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("FuncDef");
        throw TraceException(e2);
    }
}

void Parser::analyse_MainFuncDef() {
    try {
        create_grammar_node(index, "<MainFuncDef>");
        current_symbol = Symbol();
        nextsym();
        if (sym.type != "INTTK") {
            std::cout << "<MainFuncDef>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        current_symbol.type = 4;
        nextsym();
        if (sym.type != "MAINTK") {
            std::cout << "<MainFuncDef>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        current_symbol.word = fade_sym(-1);
        nextsym();
        if (sym.type != "LPARENT") {
            std::cout << "<MainFuncDef>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        nextsym();
        if (sym.type != "RPARENT") {
            std::cout << "<MainFuncDef>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
            error_list.push_back(error);
            lastsym();
        }
        current_symbol.self_table = new SymbolTable(current_table, current_symbol.word->value);
        push_symbol();
        current_table = current_symbol.self_table;
        analyse_Block();
        current_table = current_table->parent_table;
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("MainFuncDef");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("MainFuncDef");
        throw TraceException(e2);
    }
}

void Parser::analyse_FuncType() {
    try {
        create_grammar_node(index, "<FuncType>");
        nextsym();
        if (sym.type != "VOIDTK" && sym.type != "INTTK") {
            std::cout << "<FuncType>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        if (sym.type == "VOIDTK") {
            current_symbol.type = 3;
        } else {
            current_symbol.type = 4;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("FuncType");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("FuncType");
        throw TraceException(e2);
    }
}

void Parser::analyse_FuncFParams() {
    try {
        create_grammar_node(index, "<FuncFParams>");
        analyse_FuncFParam();
        Word *temp = fade_sym();
        while (temp->type == "COMMA") {
            nextsym();
            analyse_FuncFParam();
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("FuncFParams");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("FuncFParams");
        throw TraceException(e2);
    }
}

void Parser::analyse_FuncFParam() {
    try {
        create_grammar_node(index, "<FuncFParam>");
        Symbol symbol0 = Symbol();
        symbol0.con = 2;
        analyse_BType();
        nextsym();
        if (sym.type != "IDENFR") {
            std::cout << "<FuncFParam>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        symbol0.word = fade_sym(-1);
        Word *temp = fade_sym();
        int list_num = 0;
        if (temp->type == "LBRACK") {
            nextsym();
            nextsym();
            if (sym.type != "RBRACK") {
                std::cout << "<FuncFParam>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "k", "缺少右中括号", "缺少右中括号");
                error_list.push_back(error);
                lastsym();
            }
            list_num++;
            temp = fade_sym();
            if (temp->type == "LBRACK") {
                nextsym();
                analyse_ConstExp();
                nextsym();
                if (sym.type != "RBRACK") {
                    std::cout << "<FuncFParam>Error: " << sym.value << "   " << sym.type << std::endl;
                    Error error(fade_sym(-2)->line, "k", "缺少右中括号", "缺少右中括号");
                    error_list.push_back(error);
                    lastsym();
                }
                list_num++;
            }
        }
        current_symbol.func_param_type.push_back(list_num);
        symbol0.type = list_num;
        push_symbol(symbol0);
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("FuncFParam");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("FuncFParam");
        throw TraceException(e2);
    }
}

void Parser::analyse_Block() {
    try {
        create_grammar_node(index, "<Block>");
        nextsym();
        if (sym.type != "LBRACE") {
            std::cout << "<Block>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        Word *temp = fade_sym();
        while (temp->type != "RBRACE") {
            analyse_BlockItem();
            temp = fade_sym();
        }
        nextsym();
        if (!current_table->name.empty() && current_table->name != "for" &&
            current_table->parent_table->symbol_map[current_table->name].type == 4 &&
            current_table->return_flag == -1) {
            Error error(sym.line, "g", "缺少return语句", "缺少return语句");
            error_list.push_back(error);
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("Block");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Block");
        throw TraceException(e2);
    }
}

void Parser::analyse_BlockItem() {
    try {
        create_grammar_node(index, "<BlockItem>");
        Word *temp = fade_sym();
        if (temp->type == "INTTK" || temp->type == "CONSTTK") {
            analyse_Decl();
        } else {
            analyse_Stmt(false);
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("BlockItem");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("BlockItem");
        throw TraceException(e2);
    }
}

void Parser::analyse_Stmt(bool for_flag) {
    try {
        create_grammar_node(index, "<Stmt>");
        Word *temp = fade_sym();
        if (temp->type == "PLUS" || temp->type == "MINU" || temp->type == "NOT" ||
            temp->type == "INTCON" || temp->type == "LPARENT") {
            analyse_Exp();
            nextsym();
            if (sym.type != "SEMICN") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
                error_list.push_back(error);
                lastsym();
            }
        }
        else if (temp->type == "SEMICN") {
            nextsym();
        }
        else if (temp->type == "LBRACE") {
            auto *temp_table = new SymbolTable(current_table);
            current_table = temp_table;
            if (for_flag) {
                current_table->name = "for";
            }
//            else if (if_flag) {
//                current_table->name = "if";
//            } else {
//                current_table->name = "basic";
//            }
            analyse_Block();
            current_table = current_table->parent_table;
        }
        else if (temp->type == "IFTK") {
            nextsym();
            nextsym();
            if (sym.type != "LPARENT") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
            }
            analyse_Cond();
            nextsym();
            if (sym.type != "RPARENT") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
                error_list.push_back(error);
                lastsym();
            }
            analyse_Stmt(false);
            temp = fade_sym();
            if (temp->type == "ELSETK") {
                nextsym();
                analyse_Stmt(false);
            }
        }
        else if (temp->type == "FORTK") {
            analyse_Stmt_for(temp);
        }
        else if (temp->type == "BREAKTK" || temp->type == "CONTINUETK") {
            nextsym();
            if (!current_table->isInTable("for") && !for_flag) {
                Error error(sym.line, "m", "break或continue语句只能出现在循环语句中",
                            "break或continue语句只能出现在循环语句中");
                error_list.push_back(error);
            }
            nextsym();
            if (sym.type != "SEMICN") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
                error_list.push_back(error);
                lastsym();
            }
        }
        else if (temp->type == "RETURNTK") {
            nextsym();
            temp = fade_sym();
            int func_type = get_return_type();
            if (temp->type != "SEMICN") {
                current_table->return_flag = 4;
                if (func_type == 3) {
                    Error error(sym.line, "f", "无返回值的函数存在不匹配的return语句",
                                "无返回值的函数存在不匹配的return语句");
                    error_list.push_back(error);
                }
                analyse_Exp();
            } else {
                current_table->return_flag = 3;
//                if (func_type == 4) {
//                    Error error(sym.line, "f", "有返回值的函数缺少return语句",
//                                "有返回值的函数缺少return语句");
//                    error_list.push_back(error);
//                }
            }
            nextsym();
            if (sym.type != "SEMICN") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
                error_list.push_back(error);
                lastsym();
            }
        } else if (temp->type == "IDENFR") {
            analyse_Stmt_idenfr(temp);
        }
        else if (temp->type == "PRINTFTK") {
            nextsym();
            nextsym();
            if (sym.type != "LPARENT") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
            }
            nextsym();
            if (sym.type != "STRCON") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
            }
            int ident_num = 0;
            size_t pos = 0;
            while ((pos = sym.value.find("%d", pos)) != std::string::npos) {
                ++ident_num;
                pos += 2;
                if (pos >= sym.value.length()) {
                    break;
                }
            }
            temp = fade_sym();
            int actual_num = -1;
            while (temp->type == "COMMA") {
                actual_num++;
                nextsym();
                analyse_Exp();
                temp = fade_sym();
            }
            if (ident_num - 1 != actual_num) {
                Error error(sym.line, "l", "格式控制串的参数个数与变量个数不匹配",
                            "格式控制串的参数个数与变量个数不匹配");
                error_list.push_back(error);
            }
            nextsym();
            if (sym.type != "RPARENT") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
                error_list.push_back(error);
                lastsym();
            }
            nextsym();
            if (sym.type != "SEMICN") {
                std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
                error_list.push_back(error);
                lastsym();
            }
        } else {
            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("Stmt");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Stmt");
        throw TraceException(e2);
    }
}

void Parser::analyse_Stmt_idenfr(Word *temp) {
    try {
        temp = fade_sym(1);
        if (temp->type == "LPARENT") {
            analyse_Exp();
            nextsym();
        } else {
            bool flag = false;
            int i = 2;
            while (temp->type != "SEMICN") {
                if (temp->type == "ASSIGN") {
                    flag = true;
                    break;
                }
                temp = fade_sym(i++);
            }
            if (!flag) {
                analyse_Exp();
                nextsym();
            } else {
                analyse_LVal();
                nextsym();
                if (sym.type == "ASSIGN") {
                    check_const_symbol(temp_current_symbol.word->value, temp_current_symbol.word->line);
                    temp = fade_sym();
                    if (temp->type == "GETINTTK") {
                        nextsym();
                        nextsym();
                        if (sym.type != "LPARENT") {
                            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                        }
                        nextsym();
                        if (sym.type != "RPARENT") {
                            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
                            Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
                            error_list.push_back(error);
                            lastsym();
                        }
                        nextsym();
                    } else {
                        analyse_Exp();
                        nextsym();
                    }
                }
            }
        }
        if (sym.type != "SEMICN") {
            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
            error_list.push_back(error);
            lastsym();
        }
    }catch (TraceException &e) {
        e.add_trace_back("Stmt_idenfr");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Stmt_idenfr");
        throw TraceException(e2);
    }
}

void Parser::analyse_Stmt_for(Word *temp) {
    try {
        nextsym();
        nextsym();
        if (sym.type != "LPARENT") {
            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        temp = fade_sym();
        if (temp->type != "SEMICN") {
            analyse_ForStmt();
        }
        nextsym();
        if (sym.type != "SEMICN") {
            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
            error_list.push_back(error);
            lastsym();
        }
        temp = fade_sym();
        if (temp->type != "SEMICN") {
            analyse_Cond();
        }
        nextsym();
        if (sym.type != "SEMICN") {
            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "i", "缺少分号", "缺少分号");
            error_list.push_back(error);
            lastsym();
        }
        temp = fade_sym();
        if (temp->type != "RPARENT") {
            analyse_ForStmt();
        }
        nextsym();
        if (sym.type != "RPARENT") {
            std::cout << "<Stmt>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
            error_list.push_back(error);
            lastsym();
        }
        analyse_Stmt(true);
    }catch (TraceException &e) {
        e.add_trace_back("Stmt_for");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Stmt_for");
        throw TraceException(e2);
    }
}

void Parser::analyse_ForStmt() {
    try {
        create_grammar_node(index, "<ForStmt>");
        analyse_LVal();
        check_const_symbol(temp_current_symbol.word->value, temp_current_symbol.word->line);
        nextsym();
        if (sym.type != "ASSIGN") {
            std::cout << "<ForStmt>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        analyse_Exp();
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("ForStmt");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("ForStmt");
        throw TraceException(e2);
    }
}

void Parser::analyse_Exp() {
    try {
        create_grammar_node(index, "<Exp>");
        analyse_AddExp();
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("Exp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Exp");
        throw TraceException(e2);
    }
}

void Parser::analyse_Cond() {
    try {
        create_grammar_node(index, "<Cond>");
        analyse_LOrExp();
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("Cond");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Cond");
        throw TraceException(e2);
    }
}

void Parser::analyse_LVal() {
    try {
        create_grammar_node(index, "<LVal>");
        nextsym();
        if (sym.type != "IDENFR") {
            std::cout << "<LVal>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        check_symbol(sym.value);
        temp_current_symbol.word->value = sym.value;
        temp_current_symbol.word->type = sym.type;
        temp_current_symbol.word->line = sym.line;
        Word *temp = fade_sym();
        while (temp->type == "LBRACK") {
            nextsym();
            int temp0 = temp_current_symbol.type;
            std::string temp1 = temp_current_symbol.word->value;
            std::string temp2 = temp_current_symbol.word->type;
            int temp3 = temp_current_symbol.word->line;
            analyse_Exp();
            temp_current_symbol.type = temp0;
            temp_current_symbol.word->value = temp1;
            temp_current_symbol.word->type = temp2;
            temp_current_symbol.word->line = temp3;
            nextsym();
            temp_current_symbol.type--;
            if (sym.type != "RBRACK") {
                std::cout << "<LVal>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "k", "缺少右中括号", "缺少右中括号");
                error_list.push_back(error);
                lastsym();
            }
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("LVal");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("LVal");
        throw TraceException(e2);
    }
}

void Parser::analyse_PrimaryExp() {
    try {
        create_grammar_node(index, "<PrimaryExp>");
        Word *temp = fade_sym();
        if (temp->type == "LPARENT") {
            nextsym();
            analyse_Exp();
            nextsym();
            if (sym.type != "RPARENT") {
                std::cout << "<PrimaryExp>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
                error_list.push_back(error);
                lastsym();
            }
        } else if (temp->type == "IDENFR") {
            analyse_LVal();
        } else if (temp->type == "INTCON") {
            analyse_Number();
        } else {
            std::cout << "<PrimaryExp>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("PrimaryExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("PrimaryExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_Number() {
    try {
        create_grammar_node(index, "<Number>");
        nextsym();
        if (sym.type != "INTCON") {
            std::cout << "<Number>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("Number");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("Number");
        throw TraceException(e2);
    }
}

void Parser::analyse_UnaryExp() {
    try {
        create_grammar_node(index, "<UnaryExp>");
        Word *temp = fade_sym();
        if (temp->type == "LPARENT" || temp->type == "INTCON") {
            analyse_PrimaryExp();
        } else if (temp->type == "PLUS" || temp->type == "MINU" || temp->type == "NOT") {
            analyse_UnaryOp();
            analyse_UnaryExp();
        } else if (temp->type == "IDENFR") {
            temp = fade_sym(1);
            if (temp->type == "LPARENT") {
                nextsym();
                check_symbol(sym.value);
                int temp_type = temp_current_symbol.type;
                nextsym();
                temp = fade_sym();
                int i;
                if (temp->type != "RPARENT") {
                    if (temp->type == "PLUS" || temp->type == "MINU" || temp->type == "NOT" ||
                        temp->type == "MULT" || temp->type == "DIV" || temp->type == "LSS" ||
                        temp->type == "LEQ" || temp->type == "GRE" || temp->type == "GEQ" ||
                        temp->type == "EQL" || temp->type == "NEQ" || temp->type == "AND" ||
                        temp->type == "OR" || temp->type == "INTCON" || temp->type == "IDENFR") {
                        analyse_FuncRParams();
                    }
                    temp = fade_sym();
                    if (temp->type != "RPARENT") {
                        Error error(fade_sym(-2)->line, "j", "缺少右小括号", "缺少右小括号");
                        error_list.push_back(error);
                    } else {
                        nextsym();
                    }
                } else {
                    nextsym();
                    if (!current_symbol.func_param_type.empty() ){
                        Error error(fade_sym(-2)->line, "d", "函数参数个数不匹配", "函数参数个数不匹配");
                        error_list.push_back(error);
                    }
                }
                temp_current_symbol.type = temp_type;
            } else {
                analyse_PrimaryExp();
            }
        } else {
            std::cout << "<UnaryExp>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("UnaryExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("UnaryExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_UnaryOp() {
    try {
        create_grammar_node(index, "<UnaryOp>");
        nextsym();
        if (sym.type != "PLUS" && sym.type != "MINU" && sym.type != "NOT") {
            std::cout << "<UnaryOp>Error: " << sym.value << "   " << sym.type << std::endl;
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("UnaryOp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("UnaryOp");
        throw TraceException(e2);
    }
}

void Parser::analyse_FuncRParams() {
    try {
        create_grammar_node(index, "<FuncRParams>");
        int param_num = 0;
        temp_current_symbol.type = -1;
        Symbol temp_current_symbol2 = current_symbol;
        analyse_Exp();
        current_symbol = temp_current_symbol2;
        if(temp_current_symbol.type ==3 ){
            temp_current_symbol.type = -2;
        }else if (temp_current_symbol.type == 4){
            temp_current_symbol.type = 0;
        }
        int param_type;
        if(param_num + 1  > current_symbol.func_param_type.size()){
            std::cout << "<FuncRParams1111>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(temp_current_line, "d", "函数参数个数不匹配", "函数参数个数不匹配");
            error_list.push_back(error);
        }
        else {
            param_type = current_symbol.func_param_type.at(param_num++);
            if (param_type == temp_current_symbol.type || (param_type == 0 && temp_current_symbol.type == -1));
            else {
                std::cout << "<FuncRParams>Error1: " << sym.value << "   " << sym.type << std::endl;
                Error error(temp_current_line, "e", "函数参数类型不匹配", "函数参数类型不匹配");
                error_list.push_back(error);
            }
        }
        Word *temp = fade_sym();
        while (temp->type == "COMMA") {
            nextsym();
            temp_current_symbol.type = -1;
            temp_current_symbol2 = current_symbol;
            analyse_Exp();
            current_symbol = temp_current_symbol2;
            if(param_num + 1 > current_symbol.func_param_type.size()){
                std::cout << "<FuncRParams222222>Error: " << sym.value << "   " << sym.type << std::endl;
                Error error(temp_current_line, "d", "函数参数个数不匹配", "函数参数个数不匹配");
                error_list.push_back(error);
            }else {
                if(temp_current_symbol.type ==3 ){
                    temp_current_symbol.type = -2;
                }else if (temp_current_symbol.type == 4){
                    temp_current_symbol.type = 0;
                }
                param_type = current_symbol.func_param_type.at(param_num++);
                if (param_type == temp_current_symbol.type || (param_type == 0 && temp_current_symbol.type == -1));
                else {
                    std::cout << "<FuncRParams>Error2: " << sym.value << "   " << sym.type << std::endl;
                    Error error(temp_current_line, "e", "函数参数类型不匹配", "函数参数类型不匹配");
                    error_list.push_back(error);
                }
            }
            temp = fade_sym();
        }
        if (param_num < current_symbol.func_param_type.size()) {
            std::cout << "<FuncRParams333333>Error: " << sym.value << "   " << sym.type << std::endl;
            Error error(temp_current_line, "d", "函数参数个数不匹配", "函数参数个数不匹配");
            error_list.push_back(error);
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("FuncRParams");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("FuncRParams");
        throw TraceException(e2);
    }
}

void Parser::analyse_MulExp() {
    try {
        int start_id = index;
        create_grammar_node(index, "<MulExp>");
        analyse_UnaryExp();
        Word *temp = fade_sym();
        this->grammar_tree->end_id = this->index - 1;
        while (temp->type == "MULT" || temp->type == "DIV" || temp->type == "MOD") {
            create_grammar_parent_node(start_id, "<MulExp>");
            nextsym();
            analyse_UnaryExp();
            this->grammar_tree->end_id = this->index - 1;
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("MulExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("MulExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_AddExp() {
    try {
        int start_id = index;
        create_grammar_node(index, "<AddExp>");
        analyse_MulExp();
        Word *temp = fade_sym();
        this->grammar_tree->end_id = this->index - 1;
        while (temp->type == "PLUS" || temp->type == "MINU") {
            create_grammar_parent_node(start_id, "<AddExp>");
            nextsym();
            analyse_MulExp();
            this->grammar_tree->end_id = this->index - 1;
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("AddExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("AddExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_RelExp() {
    try {
        int start_id = index;
        create_grammar_node(index, "<RelExp>");
        analyse_AddExp();
        Word *temp = fade_sym();
        this->grammar_tree->end_id = this->index - 1;
        while (temp->type == "LSS" || temp->type == "LEQ" || temp->type == "GRE" || temp->type == "GEQ") {
            create_grammar_parent_node(start_id, "<RelExp>");
            nextsym();
            analyse_AddExp();
            this->grammar_tree->end_id = this->index - 1;
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("RelExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("RelExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_EqExp(){
    try {
        int start_id = index;
        create_grammar_node(index, "<EqExp>");
        analyse_RelExp();
        Word *temp = fade_sym();
        this->grammar_tree->end_id = this->index - 1;
        while (temp->type == "EQL" || temp->type == "NEQ") {
            create_grammar_parent_node(start_id, "<EqExp>");
            nextsym();
            analyse_RelExp();
            this->grammar_tree->end_id = this->index - 1;
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("EqExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("EqExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_LAndExp() {
    try {
        int start_id = index;
        create_grammar_node(index, "<LAndExp>");
        analyse_EqExp();
        Word *temp = fade_sym();
        this->grammar_tree->end_id = this->index - 1;
        while (temp->type == "AND") {
            create_grammar_parent_node(start_id, "<LAndExp>");
            nextsym();
            analyse_EqExp();
            this->grammar_tree->end_id = this->index - 1;
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("LAndExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("LAndExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_LOrExp() {
    try {
        int start_id = index;
        create_grammar_node(index, "<LOrExp>");
        analyse_LAndExp();
        Word *temp = fade_sym();
        this->grammar_tree->end_id = this->index - 1;
        while (temp->type == "OR") {
            create_grammar_parent_node(start_id, "<LOrExp>");
            nextsym();
            analyse_LAndExp();
            this->grammar_tree->end_id = this->index - 1;
            temp = fade_sym();
        }
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("LOrExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("LOrExp");
        throw TraceException(e2);
    }
}

void Parser::analyse_ConstExp() {
    try {
        create_grammar_node(index, "<ConstExp>");
        analyse_AddExp();
        update_grammar_node(this->index - 1);
    }catch (TraceException &e){
        e.add_trace_back("ConstExp");
        throw;
    }catch (std::exception &e){
        std::cout<<e.what()<<std::endl;
        TraceException e2(e.what());
        e2.add_trace_back("ConstExp");
        throw TraceException(e2);
    }
}

void Parser::push_symbol() {
    if (current_table->symbol_map.count(current_symbol.word->value) != 0) {
    Error error(current_symbol.word->line, "b", "变量重定义", "变量重定义");
    error_list.push_back(error);
    return;
    }
    current_symbol.parent_table = current_table;
    current_table->symbol_map[current_symbol.word->value] = current_symbol;
}

void Parser::push_symbol(Symbol symbol) {
    if (current_table->symbol_map.count(symbol.word->value) != 0) {
        Error error(symbol.word->line, "b", "变量重定义", "变量重定义");
        error_list.push_back(error);
    }else{
        symbol.parent_table = current_table;
        current_table->symbol_map[symbol.word->value] = symbol;
    }
}

void Parser::push_symbol(SymbolTable *table) {
    if (table->symbol_map.count(current_symbol.word->value) != 0) {
        Error error(current_symbol.word->line, "b", "变量重定义", "变量重定义");
        error_list.push_back(error);
    }else{
        current_symbol.parent_table = table;
        table->symbol_map[current_symbol.word->value] = current_symbol;
    }
}

void Parser::check_symbol(const std::string& name) {
    SymbolTable* temptable = current_table;
    while (temptable->symbol_map.count(name) == 0){
        if (temptable->parent_table == nullptr){
            Error error(sym.line, "c", "变量未定义", "变量未定义");
            error_list.push_back(error);
            temp_current_symbol.type = -2;
            return;
        }else {
            temptable = temptable->parent_table;
        }
    }
    if (temptable->symbol_map[name].type == 3 || temptable->symbol_map[name].type == 4){
        current_symbol = temptable->symbol_map[name];
        temp_current_line = sym.line;
    }
    temp_current_symbol.type = temptable->symbol_map[name].type;
}

void Parser::check_const_symbol(const std::string &name, int line) {
    SymbolTable* temptable = current_table;
    while (temptable->symbol_map.count(name) == 0){
        if (temptable->parent_table == nullptr){
//            Error error(sym.line, "c", "变量未定义", "变量未定义");
//            error_list.push_back(error);
//            temp_current_symbol.type = -2;
            return;
        }else {
            temptable = temptable->parent_table;
        }
    }
    if (temptable->symbol_map[name].con == 1){
        Error error(line, "h", "常量不能被赋值", "常量不能被赋值");
        error_list.push_back(error);
    }
    if (temptable->symbol_map[name].type == 3 || temptable->symbol_map[name].type == 4){
        current_symbol = temptable->symbol_map[name];
        temp_current_line = sym.line;
    }
    temp_current_symbol.type = temptable->symbol_map[name].type;
}

int Parser::get_return_type() {
    SymbolTable *temptable = current_table;
    while(temptable->name.empty() || temptable->name == "for"){
        temptable = temptable->parent_table;
    }
    return temptable->parent_table->symbol_map[temptable->name].type;
}








