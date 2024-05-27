//
// Created by 周霄 on 2023/9/28.
//

#ifndef COMPILER_COMPILER_H
#define COMPILER_COMPILER_H


#include "Error.h"
#include <vector>

class Lexer;
class Parser;
class LLVM_Generator;
class Symbol;
class SymbolTable;

class Compiler {
public:

    Compiler(const std::string& infile_path, const std::string& outfile_path,
             const std::string& errorfile_path, const std::string& llvmfile_path);
    int compile();


    Lexer* lexer = nullptr;
    Parser* parser = nullptr;
    SymbolTable* symbol_table = nullptr;
    LLVM_Generator* llvm_generator = nullptr;
    std::string llvmfile_path;
private:
    std::string infile_path;
    std::string outfile_path;
    std::string errorfile_path;

    std::vector<Error> error_list;
    void print_symbol_table() const;
    void sort_error_list();
    void print_error();

};


#endif //COMPILER_COMPILER_H
