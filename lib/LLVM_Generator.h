//
// Created by 周霄 on 2023/11/4.
//

#ifndef COMPILER_LAB5_LLVM_GENERATOR_H
#define COMPILER_LAB5_LLVM_GENERATOR_H

#include <string>
#include "iostream"
#include "Parser.h"
#include "stack"

class Quadruple {
public:
    int value1; // if 和 循环体
    int value2; // else 和 顺序语句
    int value3; // norm 和 判断语句
    int value4; // for 循环第三句
    Quadruple(int value1, int value3);
    Quadruple(int value1, int value2, int value3);
    Quadruple(int value1, int value2, int value3, int value4);

    std::string get_value1() const;
    std::string get_value2() const;
    std::string get_value3() const;
    std::string get_value4() const;
    std::string get_false_jump() const;
};

class LLVM_Generator {
public:
    Compiler* compiler;
    explicit LLVM_Generator(Compiler* compiler);
    void generate();

private:
    std::ofstream outfile;
    bool file_output = true;
    bool local_output = false;

    TreeNode *curNode;
    int curReg; //指向下一个寄存器
    std::string decling_name;
    SymbolTable *curTable;
    std::stack<int> childTableNum;
    std::stack<Quadruple*> ifStack;
    std::stack<int> condStack;
    std::stack<Quadruple*> forStack;

    bool isGlobal();
    void step_into_child_table();
    void step_out_child_table();


    void generate_compUnit();
    void generate_lib();
    void generate_decl();
    void generate_constDecl();
    void generate_constDef();
    void generate_constInitVal(Symbol* symbol);
    void generate_constExp();
    void generate_varDecl();
    void generate_varDef();
    void generate_initVal(Symbol* symbol);
    void generate_funcDef();
    void generate_funcFParams();
    void generate_mainFunc();
    void generate_block();
    void generate_blockItem();
    void generate_stmt();
    void generate_returnStmt();
    void generate_assignStmt();
    void generate_getintStmt();
    void generate_printfStmt();
    void generate_ifStmt();
    void generate_forStmt();
    void generate_exp();
    void generate_addExp();
    void generate_mulExp();
    void generate_unaryExp();
    void generate_primaryExp();
    void generate_cond();
    void generate_relExp();
    void generate_eqExp();
    void generate_lAndExp();
    void generate_lOrExp();
    void generate_number();
    void generate_lval(bool isAddress);
    void generate_funcRParams();
};


#endif //COMPILER_LAB5_LLVM_GENERATOR_H
