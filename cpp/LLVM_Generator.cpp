//
// Created by 周霄 on 2023/11/4.
//

#include "../lib/LLVM_Generator.h"

Quadruple::Quadruple(int value1, int value3) {
    this->value1 = value1;
    this->value2 = -1;
    this->value3 = value3;
    this->value4 = -1;
}

Quadruple::Quadruple(int value1, int value2, int value3) {
    this->value1 = value1;
    this->value2 = value2;
    this->value3 = value3;
    this->value4 = -1;
}

Quadruple::Quadruple(int value1, int value2, int value3, int value4) {
    this->value1 = value1;
    this->value2 = value2;
    this->value3 = value3;
    this->value4 = value4;
}

std::string Quadruple::get_value1() const {
    return "if" + std::to_string(this->value1);
}

std::string Quadruple::get_value2() const {
    if (this->value2 == -1) return "";
    else return "else" + std::to_string(this->value2);
}

std::string Quadruple::get_value3() const {
    if (this->value3 == -1) return "";
    else return "norm" + std::to_string(this->value3);
}

std::string Quadruple::get_value4() const {
    if (this->value4 == -1) return "";
    else return "forstmt" + std::to_string(this->value4);
}

std::string Quadruple::get_false_jump() const {
    if (this->value2 == -1) return "norm"+ std::to_string(this->value3);
    else return "else" + std::to_string(this->value2);
}

LLVM_Generator::LLVM_Generator(Compiler *compiler) {
    this->compiler = compiler;
    this->curTable = compiler->symbol_table;
    childTableNum.push(0);
    curNode = nullptr;
    curReg = 0;
}

bool LLVM_Generator::isGlobal() {
    if (this->curTable->level == 0) return true;
    else return false;
}

void LLVM_Generator::step_into_child_table() {
    this->curTable = this->curTable->child_table_list[childTableNum.top()];
    childTableNum.top() ++ ;
    childTableNum.push(0);
}

void LLVM_Generator::step_out_child_table() {
    this->curTable = this->curTable->parent_table;
    childTableNum.pop();
}

void LLVM_Generator::generate() {
    this->outfile.open(this->compiler->llvmfile_path);
    this->curNode = this->compiler->parser->grammar_tree_root;
    generate_compUnit();
    this->outfile.close();
}

void LLVM_Generator::generate_compUnit() {
    generate_lib();
    for (auto & child : this->curNode->child_node_list) {
        std::string temp_type = dynamic_cast<Grammar*>(child)->type;
        if (temp_type == "<Decl>") {
            this->curNode = child;
            curReg = 0;
            generate_decl();
        } else if (temp_type == "<MainFuncDef>") {
            this->curNode = child;
            curReg = 0;
            generate_mainFunc();
        } else if (temp_type == "<FuncDef>") {
            this->curNode = child;
            curReg = 0;
            generate_funcDef();
        }
    }
}

void LLVM_Generator::generate_lib() {
    outfile<< "declare i32 @getint()" << std::endl;
    if(local_output) std::cout<< "declare i32 @getint()" << std::endl;
    outfile<< "declare void @putint(i32)" << std::endl;
    if(local_output) std::cout<< "declare void @putint(i32)" << std::endl;
    outfile<< "declare void @putch(i32)" << std::endl;
    if(local_output) std::cout<< "declare void @putch(i32)" << std::endl;
    outfile<< "declare void @putstr(i8*)" << std::endl;
    if(local_output) std::cout<< "declare void @putstr(i8*)" << std::endl;
    outfile << std::endl;
    if(local_output) std::cout << std::endl;
}

void LLVM_Generator::generate_decl() {
    std::string temp_type = dynamic_cast<Grammar*>(this->curNode->child_node_list[0])->type;
    if (temp_type == "<ConstDecl>") {
        this->curNode = this->curNode->child_node_list[0];
        generate_constDecl();
    } else if (temp_type == "<VarDecl>") {
        this->curNode = this->curNode->child_node_list[0];
        generate_varDecl();
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_constDecl() {
    for (auto & child : this->curNode->child_node_list) {
        if (child->node_type == 1 && dynamic_cast<Grammar*>(child)->type == "<ConstDef>") {
            this->curNode = child;
            generate_constDef();
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_constDef() {
    int temp_curReg = this->curReg;
    std::string ident_name = dynamic_cast<Word*>(this->curNode->child_node_list[0])->value;
    Symbol* symbol = this->curTable->get_symbol(ident_name);
    this->decling_name = ident_name;
    std::string str1;//是否是全局变量
    std::string str2;//是否是常量
    std::string str3;//是否是数组
    if (symbol->type != 0) {
        file_output = false;
        curReg = 0;
        this->curNode = this->curNode->child_node_list[2];
        generate_constExp();
        symbol->dimension_1 = this->curNode->child_node_list[2]->get_int_value();
        if (symbol->type == 2) {
            curReg = 0;
            this->curNode = this->curNode->child_node_list[5];
            generate_constExp();
            symbol->dimension_2 = this->curNode->child_node_list[5]->get_int_value();
        }
        curReg = temp_curReg;
        file_output = true;
    }
    if (isGlobal()) {
        str1 = "@" + ident_name;
        str2 = "constant";
        if (symbol->type == 0) str3 = "i32";
        else if (symbol->type == 1) str3 = "[" + std::to_string(symbol->dimension_1) + " x i32]";
        else if (symbol->type == 2) str3 = "[" + std::to_string(symbol->dimension_1) +
                " x [" + std::to_string(symbol->dimension_2) + " x i32]]";
        if (file_output) outfile << str1 << " = dso_local " << str2
                                 << " " << str3 << " ";
        if (local_output) std::cout << str1 << " = dso_local " << str2
                                    << " " << str3 << " ";
    } else{
        if (symbol->type == 0){
            if (file_output) outfile << "  %t" << curReg << " = alloca i32" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = alloca i32" << std::endl;
            symbol->update_reg_num(curReg++);
        }
        else if (symbol->type == 1) {
            if (file_output) outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_1 << " x i32]" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_1 << " x i32]" << std::endl;
            symbol->update_reg_num(curReg++);
        }
        else if (symbol->type == 2) {
            if (file_output) outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_1
            << " x [" << symbol->dimension_2 << " x i32]]" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_1
            << " x [" << symbol->dimension_2 << " x i32]]" << std::endl;
            symbol->update_reg_num(curReg++);
        }
    }
    this->curNode = this->curNode->child_node_list[this->curNode->child_node_list.size()-1];
    generate_constInitVal(symbol);
    symbol->wake();
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
    this->decling_name = "";
}

void LLVM_Generator::generate_constInitVal(Symbol* symbol) {
    int temp_curReg = this->curReg;
    std::string ident_name = symbol->word->value;
    if (isGlobal()) {
        curReg = 0;
        file_output = false;
        if (symbol->type == 0) {
            this->curNode = this->curNode->child_node_list[0];
            generate_constExp();
            this->curNode->update_value(this->curNode->child_node_list[0]);
            symbol->value = this->curNode->get_int_value();
            int value = this->curNode->get_int_value();
            outfile<< value << std::endl;
            if (local_output) std::cout<< value << std::endl;
        }
        else if (symbol->type == 1) {
            if (symbol->current_dimension_1 == -1){
                int sum_value = 0;
                for (auto & child : this->curNode->child_node_list) {
                    if (child->node_type == 1){
                        symbol->current_dimension_1 ++;
                        this->curNode = child;
                        generate_constInitVal(symbol);
                        if (child->get_int_value() != 0) sum_value ++;
                    }
                }
                if (sum_value == 0){
                    outfile << "zeroinitializer" << std::endl;
                    if (local_output) std::cout << "zeroinitializer" << std::endl;
                    for(int i = 0; i < symbol->dimension_1; i++) symbol->value_list.push_back(0);
                }
                else {
                    outfile << "[";
                    if (local_output) std::cout << "[";
                    for (auto & child : this->curNode->child_node_list) {
                        if (child->node_type == 1) {
                            outfile<< "i32 "<< child->get_int_value();
                            if (local_output) std::cout<< "i32 "<< child->get_int_value();
                            symbol->value_list.push_back(child->get_int_value());
                        }else if (child->node_type == 0 && dynamic_cast<Word*>(child)->type == "COMMA"){
                            outfile << ", " ;
                            if (local_output) std::cout << ", " ;
                        }
                    }
                    outfile << "]" << std::endl;
                    if (local_output) std::cout << "]" << std::endl;
                }
            }
            else {
                this->curNode = this->curNode->child_node_list[0];
                generate_constExp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
        else if (symbol->type == 2) {
            if (symbol->current_dimension_1 == -1){
                int sum_value = 0;
                for (auto & child : this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_1 ++;
                        symbol->current_dimension_2 = -1;
                        this->curNode = child;
                        generate_constInitVal(symbol);
                        if (child->get_int_value() != 0) sum_value ++;
                    }
                }
                if (sum_value == 0) {
                    outfile << "zeroinitializer" << std::endl;
                    if (local_output) std::cout << "zeroinitializer" << std::endl;
                    for(int i = 0; i < symbol->dimension_1 * symbol->dimension_2; i++) symbol->value_list.push_back(0);
                }
                else {
                    std::string str = "[" + std::to_string(symbol->dimension_2) + " x i32]";
                    outfile << "[";
                    if (local_output) std::cout << "[";
                    for (auto & child : this->curNode->child_node_list) {
                        if (child->node_type == 1) {
                            outfile << str;
                            if (local_output) std::cout << str;
                            if (child->get_int_value() == 0){
                                outfile << " zeroinitializer";
                                if (local_output) std::cout << " zeroinitializer";
                                for (int i = 0; i < symbol->dimension_2; i++) symbol->value_list.push_back(0);
                                continue;
                            }
                            outfile << " [";
                            if (local_output) std::cout << " [";
                            for (auto &child2: child->child_node_list) {
                                if (child2->node_type == 1) {
                                    outfile << "i32 " << child2->get_int_value();
                                    if (local_output) std::cout << "i32 " << child2->get_int_value();
                                    symbol->value_list.push_back(child2->get_int_value());
                                } else if (child2->node_type == 0 && dynamic_cast<Word*>(child2)->type == "COMMA") {
                                    outfile << ", ";
                                    if (local_output) std::cout << ", ";
                                }
                            }
                            outfile << "]";
                            if (local_output) std::cout << "]";
                        } else if (child->node_type == 0 && dynamic_cast<Word*>(child)->type == "COMMA") {
                            outfile << ", ";
                            if (local_output) std::cout << ", ";
                        }
                    }
                    outfile << "]" << std::endl;
                    if (local_output) std::cout << "]" << std::endl;
                }
            }
            else if (symbol->current_dimension_1 != -1 && symbol->current_dimension_2 == -1){
                int sum_value = 0;
                for (auto & child : this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_2 ++;
                        this->curNode = child;
                        generate_constInitVal(symbol);
                        sum_value += child->get_int_value();
                    }
                }
                this->curNode->update_value(sum_value, curReg++);
            }
            else {
                this->curNode = this->curNode->child_node_list[0];
                generate_constExp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
        curReg = temp_curReg;
        file_output = true;
    }else {
        if (symbol->type == 0) {
            this->curNode = this->curNode->child_node_list[0];
            generate_constExp();
            this->curNode->update_value(this->curNode->child_node_list[0]);
            std::string reg = this->curNode->get_str_value();
            if (file_output) outfile << "  store i32 " << reg << ", i32* %t" << symbol->reg_num << std::endl;
            if (local_output) std::cout << "  store i32 " << reg << ", i32* %t" << symbol->reg_num << std::endl;
            symbol->update_value(this->curNode->get_int_value());
        }
        else if (symbol->type == 1) {
            if (symbol->current_dimension_1 == -1) {
                for (auto &child: this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_1++;
                        this->curNode = child;
                        generate_constInitVal(symbol);
                        this->curNode->update_value(child);
                        std::string reg = this->curNode->get_str_value();
                        if (file_output)
                            outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                    << " x i32], [" << symbol->dimension_1 << " x i32]* %t"
                                    << symbol->reg_num << ", i32 0, i32 " << symbol->current_dimension_1
                                    << std::endl;
                        if (local_output)
                            std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                      << " x i32], [" << symbol->dimension_1 << " x i32]* %t"
                                      << symbol->reg_num << ", i32 0, i32 " << symbol->current_dimension_1
                                      << std::endl;
                        if (file_output) outfile << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        if (local_output) std::cout << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        symbol->value_list.push_back(this->curNode->get_int_value());
                        curReg++;
                    }
                }
                outfile << "  %t" << curReg << " = alloca i32*" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_1 << " x i32]" << std::endl;
                int temp = curReg++;
                outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1 << " x i32], ["
                        << symbol->dimension_1 << " x i32]* %t" << symbol->reg_num << ", i32 0, i32 0"
                        << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1 << " x i32], ["
                                            << symbol->dimension_1 << " x i32]* %t" << symbol->reg_num << ", i32 0, i32 0"
                                            << std::endl;
                outfile << "  store i32* %t" << curReg << ", i32** %t" << temp << std::endl;
                if (local_output) std::cout << "  store i32* %t" << curReg << ", i32** %t" << temp << std::endl;
                curReg++;
                symbol->reg_num = temp;
            } else {
                this->curNode = this->curNode->child_node_list[0];
                generate_constExp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
        else if (symbol->type == 2) {
            if (symbol->current_dimension_1 == -1) {
                for (auto &child: this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_1++;
                        this->curNode = child;
                        symbol->current_dimension_2 = -1;
                        generate_constInitVal(symbol);
                        this->curNode->update_value(child);
                    }
                }
                outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_2 << " x i32]*" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_2 << " x i32]*" << std::endl;
                int temp = curReg++;
                outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                        << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                        << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                        << ", i32 0, i32 0" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                            << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                                            << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                                            << ", i32 0, i32 0" << std::endl;
                outfile << "  store [" << symbol->dimension_2 << " x i32]* %t" << curReg << ", [" << symbol->dimension_2
                        << " x i32]** %t" << temp << std::endl;
                if (local_output) std::cout << "  store [" << symbol->dimension_2 << " x i32]* %t" << curReg << ", [" << symbol->dimension_2
                                            << " x i32]** %t" << temp << std::endl;
                curReg ++ ;
                symbol->reg_num = temp;
            }
            else if (symbol->current_dimension_1 != -1 && symbol->current_dimension_2 == -1) {
                for (auto &child: this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_2++;
                        this->curNode = child;
                        generate_constInitVal(symbol);
                        this->curNode->update_value(child);
                        std::string reg = this->curNode->get_str_value();
                        if (file_output)
                            outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                    << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                                    << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                                    << ", i32 0, i32 " << symbol->current_dimension_1 << ", i32 "
                                    << symbol->current_dimension_2 << std::endl;
                        if (local_output)
                            std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                      << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                                      << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                                      << ", i32 0, i32 " << symbol->current_dimension_1 << ", i32 "
                                      << symbol->current_dimension_2 << std::endl;
                        if (file_output) outfile << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        if (local_output) std::cout << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        symbol->value_list.push_back(this->curNode->get_int_value());
                        curReg++;
                    }
                }
            }
            else {
                this->curNode = this->curNode->child_node_list[0];
                generate_constExp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_constExp() {
    this->curNode = this->curNode->child_node_list[0];
    generate_addExp();
    this->curNode->update_value(this->curNode->child_node_list[0]);
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_varDecl() {
    for (auto & child : this->curNode->child_node_list) {
        if (child->node_type == 1 && dynamic_cast<Grammar*>(child)->type == "<VarDef>") {
            this->curNode = child;
            generate_varDef();
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_varDef() {
    int temp_curReg = this->curReg;
    std::string ident_name = dynamic_cast<Word*>(this->curNode->child_node_list[0])->value;;
    Symbol* symbol = this->curTable->get_symbol(ident_name);
    this->decling_name = ident_name;
    std::string str1;//是否是全局变量
    std::string str2;//是否是常量
    std::string str3;//是否是数组
    if (symbol->type != 0) {
        file_output = false;
        curReg = 0;
        this->curNode = this->curNode->child_node_list[2];
        generate_constExp();
        symbol->dimension_1 = this->curNode->child_node_list[2]->get_int_value();
        if (symbol->type == 2) {
            curReg = 0;
            this->curNode = this->curNode->child_node_list[5];
            generate_constExp();
            symbol->dimension_2 = this->curNode->child_node_list[5]->get_int_value();
        }
        curReg = temp_curReg;
        file_output = true;
    }
    if (isGlobal()) {
        str1 = "@" + ident_name;
        str2 = "global";
        if (symbol->type == 0) str3 = "i32";
        else if (symbol->type == 1) str3 = "[" + std::to_string(symbol->dimension_1) + " x i32]";
        else if (symbol->type == 2) str3 = "[" + std::to_string(symbol->dimension_1) +
                " x [" + std::to_string(symbol->dimension_2) + " x i32]]";
        if (file_output) outfile << str1 << " = dso_local " << str2
                                 << " " << str3 << " ";
        if (local_output) std::cout << str1 << " = dso_local " << str2
                                    << " " << str3 << " ";
    } else{
        if (symbol->type == 0){
            if (file_output) outfile << "  %t" << curReg << " = alloca i32" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = alloca i32" << std::endl;
            symbol->update_reg_num(curReg++);
        }
        else if (symbol->type == 1) {
            if (file_output) outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_1 << " x i32]" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_1 << " x i32]" << std::endl;
            symbol->update_reg_num(curReg++);
        }
        else if (symbol->type == 2) {
            if (file_output) outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_1
            << " x [" << symbol->dimension_2 << " x i32]]" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_1
            << " x [" << symbol->dimension_2 << " x i32]]" << std::endl;
            symbol->update_reg_num(curReg++);
        }
    }
    if (this->curNode->child_node_list[this->curNode->child_node_list.size()-1]->node_type==1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[this->curNode->child_node_list.size()-1])->type == "<InitVal>") {
        this->curNode = this->curNode->child_node_list[this->curNode->child_node_list.size()-1];
        generate_initVal(symbol);
    }
    else {
        if (isGlobal()){
            outfile << "zeroinitializer" << std::endl;
            if (local_output) std::cout << "zeroinitializer" << std::endl;
        } else {
            if (symbol->type == 1){
                outfile << "  %t" << curReg << " = alloca i32*" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = alloca i32*" << std::endl;
                int temp = curReg++;
                outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1 << " x i32], ["
                        << symbol->dimension_1 << " x i32]* %t" << symbol->reg_num << ", i32 0, i32 0"
                        << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1 << " x i32], ["
                                            << symbol->dimension_1 << " x i32]* %t" << symbol->reg_num << ", i32 0, i32 0"
                                            << std::endl;
                outfile << "  store i32* %t" << curReg << ", i32** %t" << temp << std::endl;
                if (local_output) std::cout << "  store i32* %t" << curReg << ", i32** %t" << temp << std::endl;
                curReg++;
                symbol->reg_num = temp;
            }
            else if (symbol->type == 2){
                outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_2 << " x i32]*" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_2 << " x i32]*" << std::endl;
                int temp = curReg++;
                outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                        << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                        << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                        << ", i32 0, i32 0" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                            << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                                            << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                                            << ", i32 0, i32 0" << std::endl;
                outfile << "  store [" << symbol->dimension_2 << " x i32]* %t" << curReg << ", [" << symbol->dimension_2
                        << " x i32]** %t" << temp << std::endl;
                if (local_output) std::cout << "  store [" << symbol->dimension_2 << " x i32]* %t" << curReg << ", [" << symbol->dimension_2
                                            << " x i32]** %t" << temp << std::endl;
                curReg ++ ;
                symbol->reg_num = temp;
            }
        }
    }
    symbol->wake();
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
    this->decling_name = "";
}

void LLVM_Generator::generate_initVal(Symbol* symbol) {
    int temp_curReg = this->curReg;
    std::string ident_name = symbol->word->value;
    if (isGlobal()) {
        curReg = 0;
        file_output = false;
        if (symbol->type == 0) {
            this->curNode = this->curNode->child_node_list[0];
            generate_exp();
            this->curNode->update_value(this->curNode->child_node_list[0]);
            int value = this->curNode->get_int_value();
            outfile<< value << std::endl;
            if (local_output) std::cout<< value << std::endl;
        }
        else if (symbol->type == 1) {
            if (symbol->current_dimension_1 == -1){
                int sum_value = 0;
                for (auto & child : this->curNode->child_node_list) {
                    if (child->node_type == 1){
                        symbol->current_dimension_1 ++;
                        this->curNode = child;
                        generate_initVal(symbol);
                        if(child->get_int_value() != 0) sum_value ++;
                    }
                }
                if (sum_value == 0){
                    outfile << "zeroinitializer" << std::endl;
                    if (local_output) std::cout << "zeroinitializer" << std::endl;
                }
                else {
                    outfile << "[";
                    if (local_output) std::cout << "[";
                    for (auto & child : this->curNode->child_node_list) {
                        if (child->node_type == 1) {
                            outfile<< "i32 "<< child->get_int_value();
                            if (local_output) std::cout<< "i32 "<< child->get_int_value();
                        }else if (child->node_type == 0 && dynamic_cast<Word*>(child)->type == "COMMA"){
                            outfile << ", " ;
                            if (local_output) std::cout << ", " ;
                        }
                    }
                    outfile << "]" << std::endl;
                    if (local_output) std::cout << "]" << std::endl;
                }
            }
            else {
                this->curNode = this->curNode->child_node_list[0];
                generate_exp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
        else if (symbol->type == 2) {
            if (symbol->current_dimension_1 == -1){
                int sum_value = 0;
                for (auto & child : this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_1 ++;
                        symbol->current_dimension_2 = -1;
                        this->curNode = child;
                        generate_initVal(symbol);
                        if (child->get_int_value() != 0) sum_value ++;
                    }
                }
                if (sum_value == 0) {
                    outfile << "zeroinitializer" << std::endl;
                    if (local_output) std::cout << "zeroinitializer" << std::endl;
                }
                else {
                    std::string str = "[" + std::to_string(symbol->dimension_2) + " x i32]";
                    outfile << "[";
                    if (local_output) std::cout << "[";
                    for (auto & child : this->curNode->child_node_list) {
                        if (child->node_type == 1) {
                            outfile << str;
                            if (local_output) std::cout << str;
                            if (child->get_int_value() == 0){
                                outfile << " zeroinitializer";
                                if (local_output) std::cout << " zeroinitializer";
                                continue;
                            }
                            outfile << " [";
                            if (local_output) std::cout << " [";
                            for (auto &child2: child->child_node_list) {
                                if (child2->node_type == 1) {
                                    outfile << "i32 " << child2->get_int_value();
                                    if (local_output) std::cout << "i32 " << child2->get_int_value();
                                } else if (child2->node_type == 0 && dynamic_cast<Word*>(child2)->type == "COMMA") {
                                    outfile << ", ";
                                    if (local_output) std::cout << ", ";
                                }
                            }
                            outfile << "]";
                            if (local_output) std::cout << "]";
                        } else if (child->node_type == 0 && dynamic_cast<Word*>(child)->type == "COMMA") {
                            outfile << ", ";
                            if (local_output) std::cout << ", ";
                        }
                    }
                    outfile << "]" << std::endl;
                    if (local_output) std::cout << "]" << std::endl;
                }
            }
            else if (symbol->current_dimension_1 != -1 && symbol->current_dimension_2 == -1){
                int sum_value = 0;
                for (auto & child : this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_2 ++;
                        this->curNode = child;
                        generate_initVal(symbol);
                        if (child->get_int_value() != 0) sum_value ++;
                    }
                }
                this->curNode->update_value(sum_value, curReg++);
            }
            else {
                this->curNode = this->curNode->child_node_list[0];
                generate_exp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
        curReg = temp_curReg;
        file_output = true;
    }else {
        if (symbol->type == 0) {
            this->curNode = this->curNode->child_node_list[0];
            generate_exp();
            this->curNode->update_value(this->curNode->child_node_list[0]);
            std::string reg = this->curNode->get_str_value();
            if (file_output) outfile << "  store i32 " << reg << ", i32* %t" << symbol->reg_num << std::endl;
            if (local_output) std::cout << "  store i32 " << reg << ", i32* %t" << symbol->reg_num << std::endl;
            symbol->update_value(this->curNode->get_int_value());
        }
        else if (symbol->type == 1) {
            if (symbol->current_dimension_1 == -1) {
                for (auto &child: this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_1++;
                        this->curNode = child;
                        generate_initVal(symbol);
                        this->curNode->update_value(child);
                        std::string reg = this->curNode->get_str_value();
                        if (file_output)
                            outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                    << " x i32], [" << symbol->dimension_1 << " x i32]* %t"
                                    << symbol->reg_num << ", i32 0, i32 " << symbol->current_dimension_1
                                    << std::endl;
                        if (local_output)
                            std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                      << " x i32], [" << symbol->dimension_1 << " x i32]* %t"
                                      << symbol->reg_num << ", i32 0, i32 " << symbol->current_dimension_1
                                      << std::endl;
                        if (file_output) outfile << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        if (local_output) std::cout << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        curReg++;
                    }
                }
                outfile << "  %t" << curReg << " = alloca i32*" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_1 << " x i32]" << std::endl;
                int temp = curReg++;
                outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1 << " x i32], ["
                        << symbol->dimension_1 << " x i32]* %t" << symbol->reg_num << ", i32 0, i32 0"
                        << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1 << " x i32], ["
                                            << symbol->dimension_1 << " x i32]* %t" << symbol->reg_num << ", i32 0, i32 0"
                                            << std::endl;
                outfile << "  store i32* %t" << curReg << ", i32** %t" << temp << std::endl;
                if (local_output) std::cout << "  store i32* %t" << curReg << ", i32** %t" << temp << std::endl;
                curReg++;
                symbol->reg_num = temp;
            } else {
                this->curNode = this->curNode->child_node_list[0];
                generate_exp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
        else if (symbol->type == 2) {
            if (symbol->current_dimension_1 == -1) {
                for (auto &child: this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_1++;
                        this->curNode = child;
                        symbol->current_dimension_2 = -1;
                        generate_initVal(symbol);
                        this->curNode->update_value(child);
                    }
                }
                outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_2 << " x i32]*" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_2 << " x i32]*" << std::endl;
                int temp = curReg++;
                outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                        << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                        << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                        << ", i32 0, i32 0" << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                            << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                                            << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                                            << ", i32 0, i32 0" << std::endl;
                outfile << "  store [" << symbol->dimension_2 << " x i32]* %t" << curReg << ", [" << symbol->dimension_2
                        << " x i32]** %t" << temp << std::endl;
                if (local_output) std::cout << "  store [" << symbol->dimension_2 << " x i32]* %t" << curReg << ", [" << symbol->dimension_2
                                            << " x i32]** %t" << temp << std::endl;
                curReg++;
                symbol->reg_num = temp;
            }
            else if (symbol->current_dimension_1 != -1 && symbol->current_dimension_2 == -1) {
                for (auto &child: this->curNode->child_node_list) {
                    if (child->node_type == 1) {
                        symbol->current_dimension_2++;
                        this->curNode = child;
                        generate_initVal(symbol);
                        this->curNode->update_value(child);
                        std::string reg = this->curNode->get_str_value();
                        if (file_output)
                            outfile << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                    << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                                    << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                                    << ", i32 0, i32 " << symbol->current_dimension_1 << ", i32 "
                                    << symbol->current_dimension_2 << std::endl;
                        if (local_output)
                            std::cout << "  %t" << curReg << " = getelementptr [" << symbol->dimension_1
                                      << " x [" << symbol->dimension_2 << " x i32]], [" << symbol->dimension_1
                                      << " x [" << symbol->dimension_2 << " x i32]]* %t" << symbol->reg_num
                                      << ", i32 0, i32 " << symbol->current_dimension_1 << ", i32 "
                                      << symbol->current_dimension_2 << std::endl;
                        if (file_output) outfile << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        if (local_output) std::cout << "  store i32 " << reg << ", i32* %t" << curReg << std::endl;
                        curReg++;
                    }
                }
            }
            else {
                this->curNode = this->curNode->child_node_list[0];
                generate_exp();
                this->curNode->update_value(this->curNode->child_node_list[0]);
            }
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_funcDef() {
    std::string func_name = dynamic_cast<Word*>(this->curNode->child_node_list[1])->value;
    Symbol* symbol = this->curTable->get_symbol(func_name);
    step_into_child_table();
    if(symbol->type == 4) {
        if (file_output) outfile << "define dso_local i32 @" << func_name << "(";
        if (local_output) std::cout << "define dso_local i32 @" << func_name << "(";
    }else {
        if (file_output) outfile << "define dso_local void @" << func_name << "(";
        if (local_output) std::cout << "define dso_local void @" << func_name << "(";
    }
    symbol->wake();
    if (this->curNode->child_node_list[3]->node_type == 1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[3])->type == "<FuncFParams>") {
        this->curNode = this->curNode->child_node_list[3];
        generate_funcFParams();
    }
    if (file_output) outfile << ") {" << std::endl;
    if (local_output) std::cout << ") {" << std::endl;
    for (auto & child : this->curNode->child_node_list) {
        if (child->node_type==1 && dynamic_cast<Grammar*>(child)->type == "<Block>") {
            this->curNode = child;
            generate_block();
        }
    }
    if (file_output) outfile << "}" << std::endl;
    if (local_output) std::cout << "}" << std::endl;
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_funcFParams() {
    int num = 0;
    Symbol* tableSymbol = this->curTable->get_func_symbol(this->curTable->name);
    for(auto & child : this->curNode->child_node_list){
        if (child->node_type == 1 && dynamic_cast<Grammar*>(child)->type == "<FuncFParam>") {
            Symbol* symbol = this->curTable->get_symbol(dynamic_cast<Word*>(child->child_node_list[1])->value);
            symbol->update_reg_num(curReg);
            if (symbol->type == 0){
                if(num == 0){
                    if (file_output) outfile << "i32 %t" << curReg;
                    if (local_output) std::cout << "i32 %t" << curReg;
                    num++;
                } else{
                    if (file_output) outfile << ", i32 %t" << curReg;
                    if (local_output) std::cout << ", i32 %t" << curReg;
                }
                tableSymbol->func_param_dimension.push_back(0);
                curReg++;
            }else if (symbol->type == 1){
                if(num == 0){
                    if (file_output) outfile << "i32* %t" << curReg;
                    if (local_output) std::cout << "i32* %t" << curReg;
                    num++;
                } else{
                    if (file_output) outfile << ", i32* %t" << curReg;
                    if (local_output) std::cout << ", i32* %t" << curReg;
                }
                tableSymbol->func_param_dimension.push_back(0);
                curReg++;
            } else if (symbol->type == 2){
                curNode = child->child_node_list[5];
                int temp_reg = curReg;
                file_output = false;
                generate_constExp();
                file_output = true;
                curReg = temp_reg;
                curNode = dynamic_cast<Grammar*>(curNode)->parent;
                symbol->dimension_2 = child->child_node_list[5]->get_int_value();
                if(num == 0){
                    if (file_output) outfile << "[" << symbol->dimension_2 << " x i32]* %t" << curReg;
                    if (local_output) std::cout << "[" << symbol->dimension_2 << " x i32]* %t" << curReg;
                    num++;
                } else{
                    if (file_output) outfile << ", [" << symbol->dimension_2 << " x i32]* %t" << curReg;
                    if (local_output) std::cout << ", [" << symbol->dimension_2 << " x i32]* %t" << curReg;
                }
                tableSymbol->func_param_dimension.push_back(symbol->dimension_2);
                curReg++;
            }
            symbol->wake();
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_mainFunc() {
    if (file_output) outfile << "define dso_local i32 @main() {" << std::endl;
    if (local_output) std::cout << "define dso_local i32 @main() {" << std::endl;
    for (auto & child : this->curNode->child_node_list) {
        if (child->node_type == 1 && dynamic_cast<Grammar*>(child)->type == "<Block>") {
            this->curNode = child;
            step_into_child_table();
            generate_block();
        }
    }
    if (file_output) outfile << "}" << std::endl;
    if (local_output) std::cout << "}" << std::endl;
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_block() {
    if(curTable->name != "basic") curReg ++;
    TreeNode* temp = dynamic_cast<Grammar*>(this->curNode)->parent;
    if (dynamic_cast<Grammar*>(temp)->type == "<FuncDef>"){
        if (temp->child_node_list[3]->node_type == 1 && dynamic_cast<Grammar*>(temp->child_node_list[3])->type == "<FuncFParams>"){
            temp = temp->child_node_list[3];
            for (auto & child : temp->child_node_list){
                if (child->node_type == 1 && dynamic_cast<Grammar*>(child)->type == "<FuncFParam>"){
                    Symbol* symbol = this->curTable->get_symbol(dynamic_cast<Word*>(child->child_node_list[1])->value);
                    if (symbol->type == 0){
                        if (file_output) outfile << "  %t" << curReg << " = alloca i32" << std::endl;
                        if (local_output) std::cout << "  %t" << curReg << " = alloca i32" << std::endl;
                        if (file_output) outfile << "  store i32 %t" << symbol->reg_num << ", i32* %t"
                                                 << curReg << std::endl;
                        if (local_output) std::cout << "  store i32 %t" << symbol->reg_num << ", i32* %t"
                                                    << curReg << std::endl;
                        symbol->update_reg_num(curReg++);
                    }
                    else if (symbol->type == 1){
                        if (file_output) outfile << "  %t" << curReg << " = alloca i32*" << std::endl;
                        if (local_output) std::cout << "  %t" << curReg << " = alloca i32*" << std::endl;
                        if (file_output) outfile << "  store i32* %t" << symbol->reg_num << ", i32** %t"
                                                 << curReg << std::endl;
                        if (local_output) std::cout << "  store i32* %t" << symbol->reg_num << ", i32** %t"
                                                    << curReg << std::endl;
                        symbol->update_reg_num(curReg++);
                    }
                    else {
                        if (file_output) outfile << "  %t" << curReg << " = alloca [" << symbol->dimension_2
                                                 << " x i32]*" << std::endl;
                        if (local_output) std::cout << "  %t" << curReg << " = alloca [" << symbol->dimension_2
                                                    << " x i32]*" << std::endl;
                        if (file_output) outfile << "  store [" << symbol->dimension_2 << " x i32]* %t"
                                                 << symbol->reg_num << ", [" << symbol->dimension_2
                                                 << " x i32]** %t" << curReg << std::endl;
                        if (local_output) std::cout << "  store [" << symbol->dimension_2 << " x i32]* %t"
                                                    << symbol->reg_num << ", [" << symbol->dimension_2
                                                    << " x i32]** %t" << curReg << std::endl;
                        symbol->update_reg_num(curReg++);
                    }
                    symbol->wake();
                }
            }
        }
    }
    this->curTable->return_flag = 0;
    for (auto & child : this->curNode->child_node_list){
        if (child->node_type == 1 && dynamic_cast<Grammar*>(child)->type == "<BlockItem>"){
            this->curNode = child;
            generate_blockItem();
        }
    }
    if ( this->curTable->return_flag == 0 && this->curTable->level == 1 ){
        if (file_output) outfile<<"  ret void"<<std::endl;
        if(local_output) outfile<<"  ret void"<<std::endl;
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
    step_out_child_table();
}

void LLVM_Generator::generate_blockItem() {
    for (auto & child : this->curNode->child_node_list) {
        if (dynamic_cast<Grammar*>(child)->type == "<Stmt>") {
            this->curNode = child;
            generate_stmt();
        } else if (dynamic_cast<Grammar*>(child)->type == "<Decl>") {
            this->curNode = child;
            generate_decl();
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_stmt() {
   if(this->curNode->child_node_list[0]->node_type == 0){
       if (dynamic_cast<Word*>(this->curNode->child_node_list[0])->type == "RETURNTK"){
           generate_returnStmt();
       }
       else if (dynamic_cast<Word*>(this->curNode->child_node_list[0])->type == "PRINTFTK"){
           generate_printfStmt();
       }
       else if (dynamic_cast<Word*>(this->curNode->child_node_list[0])->type == "SEMICN"){
       }
       else if (dynamic_cast<Word*>(this->curNode->child_node_list[0])->type == "IFTK"){
           generate_ifStmt();
       }
       else if (dynamic_cast<Word*>(this->curNode->child_node_list[0])->type == "FORTK"){
           generate_forStmt();
       }
       else if (dynamic_cast<Word*>(this->curNode->child_node_list[0])->type == "BREAKTK"){
           if (file_output) outfile << "  br label %"<< forStack.top()->get_value2() << std::endl;
           if (local_output) std::cout << "  br label %"<< forStack.top()->get_value2() << std::endl;
       }
       else if (dynamic_cast<Word*>(this->curNode->child_node_list[0])->type == "CONTINUETK"){
           if (file_output) outfile << "  br label %"<< forStack.top()->get_value4() << std::endl;
           if (local_output) std::cout << "  br label %"<< forStack.top()->get_value4() << std::endl;
       }
   } else {
       if (dynamic_cast<Grammar*>(this->curNode->child_node_list[0])->type == "<Block>") {
           this->curNode = this->curNode->child_node_list[0];
           step_into_child_table();
           generate_block();
       }
       else if (dynamic_cast<Grammar*>(this->curNode->child_node_list[0])->type == "<LVal>"){
           if (this->curNode->child_node_list[2]->node_type == 0){ // getint()
                generate_getintStmt();
           }else{ //赋值语句
                generate_assignStmt();
           }
       } else if (dynamic_cast<Grammar*>(this->curNode->child_node_list[0])->type == "<Exp>"){
           this->curNode = this->curNode->child_node_list[0];
           generate_exp();
       }
   }
   this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_returnStmt() {
    for (auto & child : this->curNode->child_node_list) {
        if (child->node_type==1 && dynamic_cast<Grammar*>(child)->type == "<Exp>") {
            this->curNode = child;
            generate_exp();
            std::string reg = child->get_str_value();
            if (file_output) outfile << "  ret i32 " << reg << std::endl;
            if (local_output) std::cout << "  ret i32 " << reg << std::endl;
        }
    }
    if (this->curNode->child_node_list[1]->node_type == 0 && dynamic_cast<Word*>(this->curNode->child_node_list[1])->type == "SEMICN"){
        if (file_output) outfile << "  ret void" << std::endl;
        if (local_output) std::cout << "  ret void" << std::endl;
    }
    if (this->curTable->level == 1 && this->curTable->return_flag == 0) this->curTable->return_flag = 1;
}

void LLVM_Generator::generate_assignStmt() {
    this->curNode = this->curNode->child_node_list[0];
    generate_lval(true);
    this->curNode = this->curNode->child_node_list[2];
    generate_exp();
    std::string ident_name = dynamic_cast<Word*>(this->curNode->child_node_list[0]->child_node_list[0])->value;
    Symbol* symbol= this->curTable->get_waked_symbol(ident_name);
    std::string reg1;
    std::string reg2 = this->curNode->child_node_list[2]->get_str_value();
    if (symbol->type == 0) reg1 = symbol->get_reg();
    else reg1 = this->curNode->child_node_list[0]->get_str_value();
    if (file_output) outfile << "  store i32 " << reg2 << ", i32* " << reg1 << std::endl;
    if (local_output) std::cout << "  store i32 " << reg2 << ", i32* " << reg1 << std::endl;
}

void LLVM_Generator::generate_getintStmt() {
    this->curNode = this->curNode->child_node_list[0];
    generate_lval(true);
    std::string ident_name = dynamic_cast<Word*>(this->curNode->child_node_list[0]->child_node_list[0])->value;
    std::string reg1;
    Symbol* symbol= this->curTable->get_waked_symbol(ident_name);
    if (symbol->type == 0) reg1 = symbol->get_reg();
    else reg1 = this->curNode->child_node_list[0]->get_str_value();
    if (file_output) outfile << "  %t" << curReg << " = call i32 @getint()" << std::endl;
    if (local_output) std::cout << "  %t" << curReg << " = call i32 @getint()" << std::endl;
    if (file_output) outfile << "  store i32 %t" << curReg << ", i32* " << reg1 << std::endl;
    if (local_output) std::cout << "  store i32 %t" << curReg << ", i32* " << reg1 << std::endl;
    curReg++;
}

void LLVM_Generator::generate_printfStmt() {
    if (this->curNode->child_node_list.size()>5){
        for (int i = 4; i<(this->curNode->child_node_list.size()-2);i=i+2){
            this->curNode = this->curNode->child_node_list[i];
            generate_exp();
        }
    }
    std::string str = dynamic_cast<Word*>(this->curNode->child_node_list[2])->value;
    int temp =4;
    for(int i = 1;i<str.length()-1;i++) {
        if (str[i] == '\\') {
            if (str[i + 1] == 'n') {
                if (file_output) outfile << "  call void @putch(i32 10)" << std::endl;
                if (local_output) std::cout << "  call void @putch(i32 10)" << std::endl;
                i++;
            }
        } else if(str[i] == '%'){
            if (str[i + 1] == 'd') {
                std::string reg = this->curNode->child_node_list[temp]->get_str_value();
                if (file_output) outfile << "  call void @putint(i32 " << reg << ")" << std::endl;
                if (local_output) std::cout << "  call void @putint(i32 " << reg << ")" << std::endl;
                temp = temp + 2;
                i++;
            }
        } else{
            if (file_output) outfile << "  call void @putch(i32 " << (int)str[i] << ")" << std::endl;
            if (local_output) std::cout << "  call void @putch(i32 " << (int)str[i] << ")" << std::endl;
        }
    }
}

void LLVM_Generator::generate_ifStmt() {
    auto* quadruple = new Quadruple(curReg , curReg + 1);
    ifStack.push(quadruple);
    curReg += 2;
    if(this->curNode->child_node_list.size() != 5) {
        ifStack.top()->value2 = curReg++;
        this->condStack.push(ifStack.top()->value2);
    }else this->condStack.push(ifStack.top()->value3);
    if(this->curNode->child_node_list[2]->node_type == 1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[2])->type == "<Cond>"){
       this->curNode = this->curNode->child_node_list[2];
       generate_cond();
    }
    if (file_output) outfile << "  br i1 " << this->curNode->child_node_list[2]->get_str_value() <<
        ", label %" << ifStack.top()->get_value1() << ", label %"
        << ifStack.top()->get_false_jump() << std::endl;
    if (local_output) std::cout << "  br i1 " << this->curNode->child_node_list[2]->get_str_value() <<
        ", label %" << ifStack.top()->get_value1() << ", label %"
        << ifStack.top()->get_false_jump() << std::endl;
    outfile <<std::endl << ifStack.top()->get_value1() << ":" << std::endl;
    if(local_output) std::cout <<std::endl << ifStack.top()->get_value1() << ":" << std::endl;
    this->curNode = this->curNode->child_node_list[4];
    generate_stmt();
    if (this->curNode->child_node_list.size() != 5){
        if (file_output) outfile << "  br label %" << ifStack.top()->get_value3() << std::endl;
        if (local_output) std::cout << "  br label %" << ifStack.top()->get_value3() << std::endl;
        outfile <<std::endl << ifStack.top()->get_value2() << ":" << std::endl;
        if(local_output) std::cout <<std::endl << ifStack.top()->get_value2() << ":" << std::endl;
        this->curNode = this->curNode->child_node_list[6];
        generate_stmt();
    }
    if (file_output) outfile << "  br label %" << ifStack.top()->get_value3() << std::endl;
    if (local_output) std::cout << "  br label %" << ifStack.top()->get_value3() << std::endl;
    outfile <<std::endl << ifStack.top()->get_value3() << ":" << std::endl;
    if(local_output) std::cout <<std::endl << ifStack.top()->get_value3() << ":" << std::endl;
    ifStack.pop();
    delete quadruple;
}

void LLVM_Generator::generate_forStmt() {
    auto* quadruple = new Quadruple(curReg , curReg + 1, curReg + 2 );
    int index = 2;
    if (this->curNode->child_node_list[index]->node_type == 1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[index])->type == "<ForStmt>"){
        this->curNode = this->curNode->child_node_list[index];
        generate_stmt();
        index ++ ;
    }
    index ++;
    ifStack.push(quadruple);
    curReg += 3;
    forStack.push(quadruple);
    if (file_output) outfile << "  br label %" << ifStack.top()->get_value3() << std::endl;
    if (local_output) std::cout << "  br label %" << ifStack.top()->get_value3() << std::endl;
    if (file_output) outfile <<std::endl << ifStack.top()->get_value3() << ":" << std::endl;
    if (local_output) std::cout <<std::endl << ifStack.top()->get_value3() << ":" << std::endl;
    if (this->curNode->child_node_list[index]->node_type == 1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[index])->type == "<Cond>"){
        condStack.push(ifStack.top()->value2);
        this->curNode = this->curNode->child_node_list[index];
        generate_cond();
        if (file_output) outfile << "  br i1 " << this->curNode->child_node_list[index]->get_str_value() <<
            ", label %" << ifStack.top()->get_value1() << ", label %" << ifStack.top()->get_value2() << std::endl;
        if (local_output) std::cout << "  br i1 " << this->curNode->child_node_list[index]->get_str_value() <<
            ", label %" << ifStack.top()->get_value1() << ", label %" << ifStack.top()->get_value2() << std::endl;
        index ++ ;
    }
    {
        if (file_output) outfile << "  br label %" << ifStack.top()->get_value1() << std::endl;
        if (local_output) std::cout << "  br label %" << ifStack.top()->get_value1() << std::endl;
    }
    index++;
    forStack.top()->value4 = curReg++;
    if(this->curNode->child_node_list[index]->node_type == 1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[index])->type == "<ForStmt>"){
        this->curNode = this->curNode->child_node_list[index];
        if (file_output) outfile << std::endl << forStack.top()->get_value4() << ":" <<  std::endl;
        if (local_output) std::cout << std::endl << forStack.top()->get_value4() << ":" <<  std::endl;
        generate_stmt();
        if (file_output) outfile << "  br label %" << ifStack.top()->get_value3() << std::endl;
        if (local_output) std::cout << "  br label %" << ifStack.top()->get_value3() << std::endl;
        index ++ ;
    }else {
        if (file_output) outfile << std::endl << forStack.top()->get_value4() << ":" << std::endl;
        if (local_output) std::cout << std::endl << forStack.top()->get_value4() << ":" << std::endl;
        if (file_output) outfile << "  br label %" << ifStack.top()->get_value3() << std::endl;
        if (local_output) std::cout << "  br label %" << ifStack.top()->get_value3() << std::endl;
    }
    index++;
    if (file_output) outfile << std::endl << ifStack.top()->get_value1() << ":" << std::endl;
    if (local_output) std::cout << std::endl << ifStack.top()->get_value1() << ":" << std::endl;
    this->curNode = this->curNode->child_node_list[index];
    generate_stmt();
    if (file_output) outfile << "  br label %" <<  forStack.top()->get_value4() << std::endl;
    if (local_output) std::cout << "  br label %" <<  forStack.top()->get_value4() << std::endl;
    if (file_output) outfile << std::endl << ifStack.top()->get_value2() << ":" << std::endl;
    if (local_output) std::cout << std::endl << ifStack.top()->get_value2() << ":" << std::endl;
    ifStack.pop();
    forStack.pop();
    delete quadruple;

}

void LLVM_Generator::generate_exp() {
    this->curNode = this->curNode->child_node_list[0];
    generate_addExp();
    this->curNode->update_value(this->curNode->child_node_list[0]);
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_addExp() {
    if (this->curNode->child_node_list.size() == 1){
        this->curNode = this->curNode->child_node_list[0];
        generate_mulExp();
        this->curNode->update_value(this->curNode->child_node_list[0]);
    }else{
        this->curNode = this->curNode->child_node_list[0];
        generate_addExp();
        this->curNode = this->curNode->child_node_list[2];
        generate_mulExp();
        std::string reg1 = this->curNode->child_node_list[0]->get_str_value();
        std::string reg2 = this->curNode->child_node_list[2]->get_str_value();
        std::string op = dynamic_cast<Word*>(this->curNode->child_node_list[1])->type;
        if(op == "PLUS"){
            if (file_output) outfile << "  %t"<<curReg<<" = add i32 " << reg1 <<", "<< reg2 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = add i32 " << reg1 <<", "<< reg2 <<std::endl;
        }else{
            if (file_output) outfile << "  %t"<<curReg<<" = sub i32 " << reg1 <<", "<< reg2 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = sub i32 " << reg1 <<", "<< reg2 <<std::endl;
        }
        this->curNode->update_value(this->curNode->child_node_list[0], this->curNode->child_node_list[2],
                                    op, curReg++);
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_mulExp() {
    if(this->curNode->child_node_list.size() == 1){
        this->curNode = this->curNode->child_node_list[0];
        generate_unaryExp();
        this->curNode->update_value(this->curNode->child_node_list[0]);
    }else{
        this->curNode = this->curNode->child_node_list[0];
        generate_mulExp();
        this->curNode = this->curNode->child_node_list[2];
        generate_unaryExp();
        std::string reg1 = this->curNode->child_node_list[0]->get_str_value();
        std::string reg2 = this->curNode->child_node_list[2]->get_str_value();
        std::string op = dynamic_cast<Word*>(this->curNode->child_node_list[1])->type;
        if( op == "MULT"){
            if (file_output) outfile << "  %t"<<curReg<<" = mul i32 " << reg1 <<", "<< reg2 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = mul i32 " << reg1 <<", "<< reg2 <<std::endl;
        }else if(op == "DIV"){
            if (file_output) outfile << "  %t"<<curReg<<" = sdiv i32 " << reg1 <<", "<< reg2 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = sdiv i32 " << reg1 <<", "<< reg2 <<std::endl;
        }else if(op == "MOD"){
            if (file_output) outfile << "  %t"<<curReg<<" = srem i32 " << reg1 <<", "<< reg2 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = srem i32 " << reg1 <<", "<< reg2 <<std::endl;
        }
        this->curNode->update_value(this->curNode->child_node_list[0], this->curNode->child_node_list[2],
                                    op, curReg++);
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_unaryExp() {
    if(this->curNode->child_node_list.size() == 1){
        this->curNode = this->curNode->child_node_list[0];
        generate_primaryExp();
        this->curNode->update_value(this->curNode->child_node_list[0]);
    }
    else if(this->curNode->child_node_list.size() == 2){
        this->curNode = this->curNode->child_node_list[1];
        generate_unaryExp();
//       UnaryOp
        std::string op = dynamic_cast<Word*>(dynamic_cast<Grammar*>(this->curNode)->child_list[0]->child_node_list[0])->type;
        if (op == "MINU"){
            std::string reg1 = this->curNode->child_node_list[1]->get_str_value();
            if (file_output) outfile << "  %t"<<curReg<<" = sub i32 0, " << reg1 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = sub i32 0, " << reg1 <<std::endl;
            this->curNode->update_value(this->curNode->child_node_list[1], curReg++);
        }
        else if ( op == "NOT"){
            std::string reg1 = this->curNode->child_node_list[1]->get_str_value();
            if (file_output) outfile << "  %t"<<curReg<<" = icmp eq i32 " << reg1 <<", 0" <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = icmp eq i32 " << reg1 <<", 0" <<std::endl;
            curReg++;
            if (file_output) outfile << "  %t"<<curReg<<" = zext i1 %t" << curReg-1 << " to i32" <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = zext i1 %t" << curReg-1 << " to i32" <<std::endl;
            this->curNode->update_value(this->curNode->child_node_list[1], curReg++);
        }
        else{
            this->curNode->update_value(this->curNode->child_node_list[1]);
        }
    }
    else {
        if (this->curNode->child_node_list[2]->node_type == 1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[2])->type == "<FuncRParams>") {
            this->curNode = this->curNode->child_node_list[2];
            generate_funcRParams();
        }
        std::string func_name = dynamic_cast<Word*>(this->curNode->child_node_list[0])->value;
        Symbol* symbol = this->curTable->get_waked_symbol(func_name);
        if (symbol->type == 4){
            if(file_output) outfile << "  %t" << curReg << " = call i32 @" << func_name << "(" ;
            if(local_output) std::cout << "  %t" << curReg << " = call i32 @" << func_name << "(" ;
            this->curNode->update_reg(curReg++);
        }else {
            if(file_output) outfile << "  call void @" << func_name << "(" ;
            if(local_output) std::cout << "  call void @" << func_name << "(" ;
        }
        if (this->curNode->child_node_list[2]->node_type == 1 && dynamic_cast<Grammar*>(this->curNode->child_node_list[2])->type == "<FuncRParams>") {
            int num = -1;
            for (auto & child : this->curNode->child_node_list[2]->child_node_list) {
                if (child->node_type==1 && dynamic_cast<Grammar*>(child)->type  == "<Exp>") {
                    num++;
                    if (num != 0) {
                        if (file_output) outfile << ", ";
                        if (local_output) std::cout << ", ";
                    }
                    std::string reg = child->get_str_value();
                    if (symbol->func_param_type.at(num) == 0){
                        if (file_output) outfile << "i32 " << reg;
                        if (local_output) std::cout << "i32 " << reg;
                    }
                    else if (symbol->func_param_type.at(num) == 1){
                        if (file_output) outfile << "i32* " << reg;
                        if (local_output) std::cout << "i32* " << reg;
                    }
                    else if (symbol->func_param_type.at(num) == 2){
                        if (file_output) outfile << "[" << symbol->func_param_dimension.at(num) << " x i32]* " << reg;
                        if (local_output) std::cout << "[" << symbol->func_param_dimension.at(num) << " x i32]* " << reg;
                    }
                }
            }
        }
        if(file_output) outfile << ")" << std::endl;
        if(local_output) std::cout << ")" << std::endl;
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_primaryExp() {
    if (this->curNode->child_node_list.size() == 1) {
        if (dynamic_cast<Grammar*>(this->curNode->child_node_list[0])->type == "<Number>") {
            this->curNode = this->curNode->child_node_list[0];
            generate_number();
            this->curNode->update_value(this->curNode->child_node_list[0]);
        } else {
            this->curNode = this->curNode->child_node_list[0];
            generate_lval(false);
            this->curNode->update_value(this->curNode->child_node_list[0]);
        }
    }
    else {
        this->curNode = this->curNode->child_node_list[1];
        generate_exp();
        this->curNode->update_value(this->curNode->child_node_list[1]);
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_cond() {
    this->curNode= this->curNode->child_node_list[0];
    generate_lOrExp();
    this->curNode->update_value(this->curNode->child_node_list[0]);
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
    this->condStack.pop();
}

void LLVM_Generator::generate_lOrExp() {

    if(this->curNode->child_node_list.size() == 1) {
        this->curNode = this->curNode->child_node_list[0];
        generate_lAndExp();
        this->curNode->update_value(this->curNode->child_node_list[0]);
    }else {
        this->curNode = this->curNode->child_node_list[0];
        this->condStack.push(curReg++);
        generate_lOrExp();
        std::string reg1 = this->curNode->child_node_list[0]->get_str_value();
        if(file_output) outfile << "  br" << " i1 " << reg1 << ", label %" << ifStack.top()->get_value1() <<
            ", label %or_cond"<< std::to_string(this->condStack.top()) << std::endl;
        if(local_output) std::cout << "  br" << " i1 " << reg1 << ", label %" << ifStack.top()->get_value1() <<
                                   ", label %or_cond"<< std::to_string(this->condStack.top()) << std::endl;
        outfile <<std::endl << "or_cond" << std::to_string(this->condStack.top()) << ":" << std::endl;
        if(local_output) std::cout <<std::endl << "or_cond" << std::to_string(this->condStack.top()) << ":" << std::endl;
        this->curNode = this->curNode->child_node_list[2];
        generate_lAndExp();
        this->curNode->update_value(this->curNode->child_node_list[2]);
        this->condStack.pop();
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_lAndExp() {
    if(this->curNode->child_node_list.size() == 1) {
        this->curNode = this->curNode->child_node_list[0];
        generate_eqExp();
        this->curNode->update_value(this->curNode->child_node_list[0]);
    }else {
        this->curNode = this->curNode->child_node_list[0];
        int temp_reg = curReg++;
        generate_lAndExp();
        std::string reg1 = this->curNode->child_node_list[0]->get_str_value();
        int temp_reg2 = condStack.top();
        condStack.pop();
        if (condStack.size() >=2 ){
//            if (file_output) outfile << "  br" << " i1 " << reg1 << ", label %or_cond"<<
//                                        std::to_string(this->condStack.top()) <<", label %and_cond"<<
//                                        std::to_string(temp_reg) << std::endl;
//            if (local_output) std::cout << "  br" << " i1 " << reg1 << ", label %or_cond"<<
//                                        std::to_string(this->condStack.top()) <<", label %and_cond"<<
//                                        std::to_string(temp_reg) << std::endl;
            if (file_output) outfile << "  br" << " i1 " << reg1 << ", label %and_cond"<<
                                        std::to_string(temp_reg) <<", label %or_cond"<<
                                        std::to_string(this->condStack.top()) << std::endl;
            if (local_output) std::cout << "  br" << " i1 " << reg1 << ", label %and_cond"<<
                                        std::to_string(temp_reg) <<", label %or_cond"<<
                                        std::to_string(this->condStack.top()) << std::endl;
        } else {
            if (file_output) outfile << "  br" << " i1 " << reg1 << ", label %and_cond"<<
                                     std::to_string(temp_reg) <<", label %"<<
                                     ifStack.top()->get_false_jump() << std::endl;
            if (local_output) std::cout << "  br" << " i1 " << reg1 << ", label %and_cond"<<
                                     std::to_string(temp_reg) <<", label %"<<
                                     ifStack.top()->get_false_jump() << std::endl;
        }
        if (file_output) outfile <<std::endl << "and_cond" <<
                                         std::to_string(temp_reg) << ":" << std::endl;
        if (local_output) std::cout <<std::endl << "and_cond" <<
                                         std::to_string(temp_reg) << ":" << std::endl;
        condStack.push(temp_reg2);
        this->curNode = this->curNode->child_node_list[2];
        generate_eqExp();
        this->curNode->update_value(this->curNode->child_node_list[2]);
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_eqExp() {
    if(this->curNode->child_node_list.size() == 1) {
        this->curNode = this->curNode->child_node_list[0];
        generate_relExp();
        if(dynamic_cast<Grammar*>(this->curNode)->parent->type == "<LAndExp>"){
            if (file_output) outfile << "  %t" << curReg << " = icmp ne i32 "
                                     << this->curNode->child_node_list[0]->get_str_value() << ", 0" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = icmp ne i32 "
                                        << this->curNode->child_node_list[0]->get_str_value() << ", 0" << std::endl;
            this->curNode->update_value((this->curNode->child_node_list[0]->get_int_value() != 0),
                                        curReg++);
        }else {
            this->curNode->update_value(this->curNode->child_node_list[0]);
        }
    }else {
        this->curNode = this->curNode->child_node_list[0];
        generate_eqExp();
        this->curNode = this->curNode->child_node_list[2];
        generate_relExp();
        std::string reg1 = this->curNode->child_node_list[0]->get_str_value();
        std::string reg2 = this->curNode->child_node_list[2]->get_str_value();
        std::string op = dynamic_cast<Word*>(this->curNode->child_node_list[1])->type;
        if (op == "EQL"){
            if (file_output) outfile << "  %t"<<curReg<<" = icmp eq i32 " << reg1 <<", "<< reg2 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = icmp eq i32 " << reg1 <<", "<< reg2 <<std::endl;
        }else{
            if (file_output) outfile << "  %t"<<curReg<<" = icmp ne i32 " << reg1 <<", "<< reg2 <<std::endl;
            if (local_output) std::cout << "  %t"<<curReg<<" = icmp ne i32 " << reg1 <<", "<< reg2 <<std::endl;
        }
        if (dynamic_cast<Grammar*>(this->curNode)->parent->type == "<EqExp>" ){
            curReg++;
            if (file_output) outfile << "  %t" << curReg << " = zext i1 %t" << curReg-1 << " to i32" << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = zext i1 %t" << curReg-1 << " to i32" << std::endl;
            this->curNode->update_value(this->curNode->child_node_list[0], this->curNode->child_node_list[2],
                                        op, curReg++);
        }else {
            this->curNode->update_value(this->curNode->child_node_list[0], this->curNode->child_node_list[2],
                                        op, curReg++);
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_relExp() {
    if(this->curNode->child_node_list.size() == 1) {
        this->curNode = this->curNode->child_node_list[0];
        generate_addExp();
        this->curNode->update_value(this->curNode->child_node_list[0]);
    }else {
        this->curNode = this->curNode->child_node_list[0];
        generate_relExp();
        this->curNode = this->curNode->child_node_list[2];
        generate_addExp();
        std::string reg1 = this->curNode->child_node_list[0]->get_str_value();
        std::string reg2 = this->curNode->child_node_list[2]->get_str_value();
        std::string op = dynamic_cast<Word*>(this->curNode->child_node_list[1])->type;
        if (op == "LSS") {
            if (file_output) outfile << "  %t" << curReg << " = icmp slt i32 " << reg1 << ", " << reg2 << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = icmp slt i32 " << reg1 << ", " << reg2 << std::endl;
        } else if (op == "LEQ") {
            if (file_output) outfile << "  %t" << curReg << " = icmp sle i32 " << reg1 << ", " << reg2 << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = icmp sle i32 " << reg1 << ", " << reg2 << std::endl;
        } else if (op == "GRE") {
            if (file_output) outfile << "  %t" << curReg << " = icmp sgt i32 " << reg1 << ", " << reg2 << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = icmp sgt i32 " << reg1 << ", " << reg2 << std::endl;
        } else {
            if (file_output) outfile << "  %t" << curReg << " = icmp sge i32 " << reg1 << ", " << reg2 << std::endl;
            if (local_output) std::cout << "  %t" << curReg << " = icmp sge i32 " << reg1 << ", " << reg2 << std::endl;
        }
        curReg ++;
        if(file_output) outfile << "  %t"<<curReg<<" = zext i1 %t"<<curReg-1<<" to i32" << std::endl;
        if(local_output) std::cout << "  %t"<<curReg<<" = zext i1 %t"<<curReg-1<<" to i32" << std::endl;
        this->curNode->update_value(this->curNode->child_node_list[0], this->curNode->child_node_list[2],
                                    op, curReg++);
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_number() {
    this->curNode->update_value(dynamic_cast<Word*>(this->curNode->child_node_list[0])->value);
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_lval(bool isAddress) {
    std::string ident_name = dynamic_cast<Word*>(this->curNode->child_node_list[0])->value;
    Symbol* symbol = this->curTable->get_waked_symbol(ident_name);
    std::string str1;//是否是全局变量
    std::string str3;//是否是数组
    if (symbol->type == 0){
        if(symbol->isGlobal()) str1 = "@" + ident_name;
        else str1 = "%t" + std::to_string(symbol->reg_num);
        str3 = "i32";
        if (file_output) outfile << "  %t"<<curReg<<" = load " << str3 << ", " << str3 << "* " << str1 << std::endl;
        if (local_output) std::cout << "  %t"<<curReg<<" = load " << str3 << ", " << str3 << "* " << str1 << std::endl;
        this->curNode->update_value(symbol->value, curReg++);
    }
    else if (symbol->type == 1) {
        if (this->curNode->child_node_list.size() == 4) {
            this->curNode = this->curNode->child_node_list[2];
            generate_exp();
            std::string reg = this->curNode->child_node_list[2]->get_str_value();
            if (symbol->isGlobal()) {
                str1 = "@" + ident_name;
                str3 = "[" + std::to_string(symbol->dimension_1) + " x i32]";
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 0, i32 " << reg << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 0, i32 " << reg << std::endl;
                curReg++;

            } else {
                str1 = "%t" + std::to_string(symbol->reg_num);
                str3 = "i32";
                if (file_output) outfile << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                curReg++;
                str1 = "%t" + std::to_string(curReg - 1);
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 " << reg << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 " << reg << std::endl;
                curReg++;
            }
            if (!isAddress) {
                if (file_output) outfile << "  %t" << curReg << " = load i32, i32* %t" << curReg - 1 << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = load i32, i32* %t" << curReg - 1 << std::endl;
                int temp_value ;
                if (symbol->isConst()) temp_value= symbol->get_const_value(this->curNode->child_node_list[2]->get_int_value());
                else temp_value = symbol->value;
                this->curNode->update_value(temp_value, curReg++);
            } else {
                this->curNode->update_reg(curReg - 1);
                this->curNode->setAddress(true);
            }
        } else {
            if (symbol->isGlobal()){
                str1 = "@" + ident_name;
                str3 = "[" + std::to_string(symbol->dimension_1) + " x i32]";
                                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 0, i32 " << 0 << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 0, i32 " << 0 << std::endl;
                curReg++;
            }else {
                str1 = "%t" + std::to_string(symbol->reg_num);
                str3 = "i32";
                if (file_output) outfile << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                curReg++;
                str1 = "%t" + std::to_string(curReg - 1);
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 " << 0 << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 " << 0 << std::endl;
                curReg++;
            }
            this->curNode->update_reg(curReg - 1 );
            this->curNode->setAddress(true);
        }
    }
    else if (symbol->type == 2){
        if (this->curNode->child_node_list.size() == 7) {
            this->curNode = this->curNode->child_node_list[2];
            generate_exp();
            std::string reg1 = this->curNode->child_node_list[2]->get_str_value();
            this->curNode = this->curNode->child_node_list[5];
            generate_exp();
            std::string reg2 = this->curNode->child_node_list[5]->get_str_value();
            if (symbol->isGlobal()) {
                str1 = "@" + ident_name;
                str3 = "[" + std::to_string(symbol->dimension_1) + " x [" + std::to_string(symbol->dimension_2) +
                       " x i32]]";
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 0, i32 " << reg1 << ", i32 " << reg2 << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 0, i32 " << reg1 << ", i32 " << reg2 << std::endl;
                curReg ++ ;
            } else {
                str1 = "%t" + std::to_string(symbol->reg_num);
                str3 = "[" + std::to_string(symbol->dimension_2) + " x i32]";
                outfile << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                if(local_output) std::cout << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                curReg++;
                str1 = "%t" + std::to_string(curReg - 1);
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 " << reg1 << ", i32 " << reg2 << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 " << reg1 << ", i32 " << reg2 << std::endl;
                curReg++;
            }
            if (!isAddress) {
                if (file_output) outfile << "  %t" << curReg << " = load i32, i32* %t" << curReg - 1 << std::endl;
                if (local_output) std::cout << "  %t" << curReg << " = load i32, i32* %t" << curReg - 1 << std::endl;
                int temp_value;
                if (symbol->isConst()) temp_value = symbol->get_const_value(this->curNode->child_node_list[2]->get_int_value(),
                                                          this->curNode->child_node_list[5]->get_int_value());
                else temp_value = symbol->value;
                this->curNode->update_value(temp_value, curReg++);
            } else {
                this->curNode->update_reg(curReg - 1);
                this->curNode->setAddress(true);
            }
        }
        else if (this->curNode->child_node_list.size() == 4) {
            this->curNode = this->curNode->child_node_list[2];
            generate_exp();
            std::string reg = this->curNode->child_node_list[2]->get_str_value();
            if (symbol->isGlobal()) {
                str1 = "@" + ident_name;
                str3 = "[" + std::to_string(symbol->dimension_1) + " x [" + std::to_string(symbol->dimension_2) +
                       " x i32]]";
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 0, i32 " << reg << ", i32 0" << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 0, i32 " << reg << ", i32 0" << std::endl;
                curReg ++ ;
            } else {
                str1 = "%t" + std::to_string(symbol->reg_num);
                str3 = "[" + std::to_string(symbol->dimension_2) + " x i32]";
                outfile << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                if(local_output) std::cout << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                curReg++;
                str1 = "%t" + std::to_string(curReg - 1);
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 " << reg << ", i32 0" << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 " << reg << ", i32 0" << std::endl;
                curReg++;
            }
            this->curNode->update_reg(curReg - 1);
            this->curNode->setAddress(true);
        }
        else {
            if (symbol->isGlobal()){
                str1 = "@" + ident_name;
                str3 = "[" + std::to_string(symbol->dimension_1) + " x [" + std::to_string(symbol->dimension_2) +
                       " x i32]]";
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 0, i32 0" << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 0, i32 0" << std::endl;
                curReg++;
            }else {
                str1 = "%t" + std::to_string(symbol->reg_num);
                str3 = "[" + std::to_string(symbol->dimension_2) + " x i32]";
                outfile << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                if(local_output) std::cout << "  %t" << curReg << " = load " << str3 << "*, " << str3 << "** " << str1 << std::endl;
                curReg++;
                str1 = "%t" + std::to_string(curReg - 1);
                if (file_output)
                    outfile << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                            << ", i32 0" << std::endl;
                if (local_output)
                    std::cout << "  %t" << curReg << " = getelementptr " << str3 << ", " << str3 << "* " << str1
                              << ", i32 0" << std::endl;
                curReg++ ;
            }
            this->curNode->update_reg(curReg - 1);
            this->curNode->setAddress(true);
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}

void LLVM_Generator::generate_funcRParams() {
    for(auto & child : this->curNode->child_node_list){
        if (child->node_type == 1 && dynamic_cast<Grammar*>(child)->type == "<Exp>") {
            this->curNode = child;
            generate_exp();
        }
    }
    this->curNode = dynamic_cast<Grammar*>(this->curNode)->parent;
}




