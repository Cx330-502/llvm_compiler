//
// Created by 周霄 on 2023/9/24.
//
#include "lib/Compiler.h"
int main(){
    int i;
    try {
        Compiler compiler("testfile.txt", "output.txt",
                          "error.txt", "llvm_ir.txt");
        compiler.compile();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}