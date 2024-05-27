//
// Created by 周霄 on 2023/10/10.
//

#ifndef COMPILER_LAB4_SYMBOLTABLE_H
#define COMPILER_LAB4_SYMBOLTABLE_H

#include <string>
#include <map>
#include <vector>
#include "Lexer.h"

class SymbolTable;

class Symbol{
public:
    Word* word;
    SymbolTable* parent_table;
    SymbolTable* self_table;
    int type; // 0: a , 1: a[] , 2: a[][] , 3: void function , 4: int function
    int con; // 1: const , 0: var , 2: param
    int value;
    int reg_num;
    int dimension_1 = -1;
    int dimension_2 = -1;
    int current_dimension_1 = -1;
    int current_dimension_2 = -1;
    bool alive = false;
    std::vector<int> func_param_type; // 参数类型
    std::vector<int> func_param_dimension; // 参数维度
    std::vector<int> value_list; // 常数组的值

    Symbol();

    void wake();
    bool isConst() const;
    bool isGlobal() const;
    void update_value(int value0);
    void update_value(int value0, int regNum);
    void update_reg_num(int regNum);
    std::string get_reg() const;
    int get_const_value() const;
    int get_const_value(int index) const;
    int get_const_value(int index1, int index2) const;

};

class SymbolTable {
public:
    int level;   // 0为全局
    int return_flag;
    std::string name;
    SymbolTable* parent_table;
    std::map<std::string, Symbol> symbol_map;
    std::vector<SymbolTable*> child_table_list;

    SymbolTable();
    explicit SymbolTable(SymbolTable* parent_table);
    explicit SymbolTable(SymbolTable* parent_table, const std::string& name);
    void print();

    bool isInTable(const std::string& symbol_name);

    Symbol* get_func_symbol(const std::string& symbol_name);
    Symbol* get_symbol(const std::string& symbol_name);
    Symbol* get_waked_symbol(const std::string& symbol_name);
    std::string get_symbol_reg(const std::string& symbol_name);
    int get_symbol_con(const std::string& symbol_name);
    int get_symbol_type(const std::string& symbol_name);
};


#endif //COMPILER_LAB4_SYMBOLTABLE_H
