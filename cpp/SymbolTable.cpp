//
// Created by 周霄 on 2023/10/10.
//

#include "../lib/SymbolTable.h"

Symbol::Symbol() {
    this->word = nullptr;
    this->parent_table = nullptr;
    this->self_table = nullptr;
    this->type = -1;
    this->con = -1;
    this->reg_num = -1;
    this->value = -1;
}

void Symbol::wake() {
    this->alive = true;
}

bool Symbol::isConst() const {
    if (this->con == 1) return true;
    else return false;
}

bool Symbol::isGlobal() const {
    if (this->parent_table->level == 0) return true;
    else return false;
}

void Symbol::update_value(int value0) {
    this->value = value0;
}

void Symbol::update_value(int value0, int regNum) {
    this->value = value0;
    this->reg_num = regNum;
}

void Symbol::update_reg_num(int regNum) {
    this->reg_num = regNum;
}

std::string Symbol::get_reg() const {
    if (this->isGlobal()){
        return "@" + this->word->value;
    }else {
        return "%t" + std::to_string(this->reg_num);
    }
}

int Symbol::get_const_value() const {
    return this->value;
}

int Symbol::get_const_value(int index) const {
    if (index >= this->value_list.size()) {
        std::cout << "Error: index out of range" << std::endl;
        return -1;
    }
    return this->value_list[index];
}

int Symbol::get_const_value(int index1, int index2) const {
    if (index1 >= this->dimension_1 || index2 >= this->dimension_2) {
        std::cout << "Error: index out of range" << std::endl;
        return -1;
    }
    return this->value_list[index1 * this->dimension_2 + index2];
}



SymbolTable::SymbolTable() {
    this->level = 0;
    this->parent_table = nullptr;
    this->return_flag = -1;
    this->name = "";
}

SymbolTable::SymbolTable(SymbolTable *parent_table) {
    this->level = parent_table->level + 1;
    this->parent_table = parent_table;
    parent_table->child_table_list.push_back(this);
    this->return_flag = -1;
    this->name = "";
}

SymbolTable::SymbolTable(SymbolTable *parent_table, const std::string &name) {
    this->level = parent_table->level + 1;
    this->parent_table = parent_table;
    parent_table->child_table_list.push_back(this);
    this->name = name;
    this->return_flag = -1;
}

void SymbolTable::print() {
    std::string space;
    for (int i = 0; i < this->level; ++i) {
        space += "    ";
    }
    std::cout << space << "Symbol Table level : " << level << std::endl;
    for (auto & symbol : this->symbol_map) {
        std::cout << space << "    " << symbol.first << " : " << symbol.second.word->value ;
        std::cout << " , type : " << symbol.second.type << " , con : " << symbol.second.con ;
std::cout << std::endl;
    }
    for (auto & child_table : this->child_table_list) {
        child_table->print();
    }
}

bool SymbolTable::isInTable(const std::string &symbol_name) {
    SymbolTable* cur_table = this;
    while (cur_table != nullptr) {
        if (cur_table->name == symbol_name)
            return true;
        cur_table = cur_table->parent_table;
    }
    return false;
}

Symbol *SymbolTable::get_symbol(const std::string &symbol_name) {
    SymbolTable* cur_table = this;
    while (cur_table != nullptr) {
        if (cur_table->symbol_map.count(symbol_name) > 0) {
            return &cur_table->symbol_map[symbol_name];
        }
        cur_table = cur_table->parent_table;
    }
    return nullptr;
}

Symbol *SymbolTable::get_waked_symbol(const std::string &symbol_name) {
    SymbolTable* cur_table = this;
    while (cur_table != nullptr) {
        if (cur_table->symbol_map.count(symbol_name) > 0 && cur_table->symbol_map[symbol_name].alive) {
            return &cur_table->symbol_map[symbol_name];
        }
        cur_table = cur_table->parent_table;
    }
    return nullptr;
}

int SymbolTable::get_symbol_con(const std::string &symbol_name) {
    SymbolTable* cur_table = this;
    while (cur_table != nullptr) {
        if (cur_table->symbol_map.count(symbol_name) > 0) {
            return cur_table->symbol_map[symbol_name].con;
        }
        cur_table = cur_table->parent_table;
    }
    return -1;
}

int SymbolTable::get_symbol_type(const std::string &symbol_name) {
    SymbolTable* cur_table = this;
    while (cur_table != nullptr) {
        if (cur_table->symbol_map.count(symbol_name) > 0) {
            return cur_table->symbol_map[symbol_name].type;
        }
        cur_table = cur_table->parent_table;
    }
    return -1;
}

std::string SymbolTable::get_symbol_reg(const std::string &symbol_name) {
    Symbol* symbol = this->get_waked_symbol(symbol_name);
    if (symbol->isGlobal()){
        return "@" + symbol_name;
    }else {
        return "%t" + std::to_string(symbol->reg_num);
    }
}

Symbol *SymbolTable::get_func_symbol(const std::string &symbol_name) {
    SymbolTable* cur_table = this;
    while (cur_table->parent_table != nullptr) {
        cur_table = cur_table->parent_table;
    }
    if (cur_table->symbol_map.count(symbol_name) > 0) {
        return &cur_table->symbol_map[symbol_name];
    }
    return nullptr;
}











