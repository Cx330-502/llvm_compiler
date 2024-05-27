//
// Created by 周霄 on 2023/9/28.
//

#include "../lib/Error.h"

Error::Error(int line, const std::string &code, const std::string &type) {
    this->line = line;
    this->code = code;
    this->type = type;
    this->explanation = "";
}

Error::Error(int line, const std::string &code, const std::string &type, const std::string &explanation) {
    this->line = line;
    this->code = code;
    this->type = type;
    this->explanation = explanation;
}



