//
// Created by 周霄 on 2023/9/28.
//

#include <algorithm>
#include "../lib/LLVM_Generator.h"

Compiler::Compiler(const std::string &infile_path, const std::string &outfile_path,
                   const std::string &errorfile_path, const std::string &llvmfile_path) {
    this->lexer = new Lexer(infile_path, outfile_path, this);
    this->symbol_table = new SymbolTable();
    this->parser = new Parser(this);
    this->llvm_generator = new LLVM_Generator(this);
    this->infile_path = infile_path;
    this->outfile_path = outfile_path;
    this->errorfile_path = errorfile_path;
    this->llvmfile_path = llvmfile_path;
}

int Compiler::compile() {
    int i = 0;
    try{
        i=1;
        this->lexer->analyse();
        i=2;
        this->parser->analyse();
//        this->print_symbol_table();
        i=3;
        this->sort_error_list();
        this->print_error();
        i=4;
        if (!this->error_list.empty()) return 1;
        this->llvm_generator->generate();
    }catch (TraceException &e){
        std::cout << e.what() <<" number  : "<< i << std::endl;
        std::cout<< e.get_trace_back() << std::endl;
    }catch (std::exception& e) {
        std::cout << e.what() <<" number  : "<< i << std::endl;
        Error error(0, "0000", "Compiler error", e.what());
    }
    return 0;
}

void Compiler::sort_error_list() {
    error_list.insert(error_list.end(), this->lexer->error_list.begin(), this->lexer->error_list.end());
    error_list.insert(error_list.end(), this->parser->error_list.begin(), this->parser->error_list.end());
    std::cout<<"Lexer error number : "<<this->lexer->error_list.size()<<std::endl;
    for (auto & error : this->lexer->error_list) {
        std::cout<<error.line<<" "<<error.code<<std::endl;
    }
    std::cout<<"Parser error number : "<<this->parser->error_list.size()<<std::endl;
    for (auto & error : this->parser->error_list) {
        std::cout<<error.line<<" "<<error.code<<std::endl;
    }
    std::sort(this->error_list.begin(), this->error_list.end(), [](const Error& a, const Error& b) {
        return a.line < b.line;
    });

    for (int i = 1; i < this->error_list.size(); i++) {
        if (this->error_list[i].line == this->error_list[i - 1].line) {
            this->error_list.erase(this->error_list.begin() + i);
            i--;
        }
    }
}

void Compiler::print_error() {
    std::ofstream errorfile;
    errorfile.open(this->errorfile_path);
    for (auto & error : this->error_list) {
        errorfile << error.line << " " << error.code << std::endl;
//        errorfile << error.line << " " << error.code << error.type<< std::endl;
    }
    errorfile.close();
}

void Compiler::print_symbol_table() const {
    this->symbol_table->print();
}
