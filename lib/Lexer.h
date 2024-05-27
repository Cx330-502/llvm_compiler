//
// Created by 周霄 on 2023/9/24.
//

#ifndef COMPILER_LEXER_H
#define COMPILER_LEXER_H

#include <vector>
#include <iostream>
#include <fstream>
#include "Compiler.h"
#include "TreeNode.h"

class Word : public TreeNode{
public:
    std::string type;
    std::string value;
    int line;
    Word();
};

class Lexer {
public:
    static std::vector<std::string> type_list;
    std::vector<Word> wordlist;
    std::vector<Error> error_list;
    std::string infile_path;
    std::string outfile_path;
    Lexer(const std::string& infile_path, const std::string& outfile_path, Compiler* compiler);
    void analyse();
private:
    int line_num;
    Compiler* compiler = nullptr;
    std::ifstream infile;
    std::ofstream outfile;
    void push_word(Word &word);
    void analyse_word(const std::string &line, int &index, int &printf_flag, char &token);
    void analyse_number(const std::string &line, int &index, char &token);
    void analyse_formatString(const std::string &line, int &index, int &printf_flag, char &token);
    int analyse_char(const std::string &line, int &index, int &printf_flag, bool &flag, char &token);
};


#endif //COMPILER_LEXER_H
