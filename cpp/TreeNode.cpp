//
// Created by 周霄 on 2023/11/4.
//

#include <iostream>
#include "../lib/TreeNode.h"

void TreeNode::setAddress(bool isAddress0) {
    this->isAddress = isAddress0;
}

std::string TreeNode::get_str_value() const {
    if (this->value_type == 0) {
        return "%t" + std::to_string(this->reg_num);
    } else {
        return std::to_string(this->value);
    }
}

int TreeNode::get_int_value() const {
    return this->value;
}

void TreeNode::update_reg(int regNum) {
    this->reg_num = regNum;
    this->value_type = 0;
}

void TreeNode::update_value(int value0) {
    this->value = value0;
    this->value_type = 1;
}

void TreeNode::update_value(int value0, int regNum) {
    this->reg_num = regNum;
    this->value = value0;
    this->value_type = 0;
    this->isAddress = false;
}

void TreeNode::update_value(const std::string& value0) {
//    std::cout << "value0: " << value0 << std::endl;
    this->value = std::stoi(value0);
    this->value_type = 1;
    this->isAddress = false;
}

void TreeNode::update_value(TreeNode *node) {
    this->reg_num = node->reg_num;
    this->value = node->value;
    this->value_type = node->value_type;
    this->isAddress = node->isAddress;
}

void TreeNode::update_value(TreeNode *node, int regNum) {
    this->reg_num = regNum;
    this->value = - node->value;
    this->value_type = 0;
    this->isAddress = false;
}

void TreeNode::update_value(TreeNode *node1, TreeNode *node2, const std::string& op, int regNum) {
    if (op == "PLUS") {
        this->value = node1->value + node2->value;
    } else if (op == "MINU") {
        this->value = node1->value - node2->value;
    } else if (op == "MULT") {
        this->value = node1->value * node2->value;
    } else if (op == "DIV") {
        this->value = node1->value / node2->value;
    } else if (op == "MOD") {
        this->value = node1->value % node2->value;
    } else if (op == "OR") {
        this->value = node1->value || node2->value;
    } else if (op == "AND") {
        this->value = node1->value && node2->value;
    } else if (op == "EQU") {
        this->value = node1->value == node2->value;
    } else if (op == "NEQ") {
        this->value = node1->value != node2->value;
    } else if (op == "LSS") {
        this->value = node1->value < node2->value;
    } else if (op == "LEQ") {
        this->value = node1->value <= node2->value;
    } else if (op == "GRE") {
        this->value = node1->value > node2->value;
    } else if (op == "GEQ") {
        this->value = node1->value >= node2->value;
    }
    this->reg_num = regNum;
    this->value_type = 0;
    this->isAddress = false;
}














