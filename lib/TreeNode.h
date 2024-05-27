//
// Created by 周霄 on 2023/11/4.
//

#ifndef COMPILER_LAB5_TREENODE_H
#define COMPILER_LAB5_TREENODE_H

#include <string>
#include <vector>

class TreeNode {
public:
    int node_type; // 1: Grammar 0: Token
    std::vector<TreeNode*> child_node_list;
    int reg_num = -1;
    int value = -1;
    int value_type = -1; // 0: 寄存器 1: 数字
    bool isAddress = false;
    virtual ~TreeNode() = default;

    void setAddress(bool isAddress0);
    std::string get_str_value() const;
    int get_int_value() const;
    void update_reg(int regNum);
    void update_value(int value0);  // 更新为常数
    void update_value(int value0, int regNum); // 读取变量
    void update_value(const std::string& value0); // 更新为常数
    void update_value(TreeNode *node); // 单赋值
    void update_value(TreeNode *node, int regNum); // 变量、数字前缀减号运算
    void update_value(TreeNode *node1, TreeNode *node2, const std::string& op, int regNum);  // 二元运算

};


#endif //COMPILER_LAB5_TREENODE_H
