//
// Created by 周霄 on 2023/9/28.
//

#ifndef COMPILER_ERROR_H
#define COMPILER_ERROR_H


#include <string>
#include <iostream>
#include "TraceException.h"
class Error {
public:
    int line;
    std::string code;
    std::string type;
    std::string explanation;
    Error(int line, const std::string& code, const std::string& type);
    Error(int line, const std::string& code, const std::string& type, const std::string& explanation);
};


#endif //COMPILER_ERROR_H
