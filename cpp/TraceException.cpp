//
// Created by 周霄 on 2023/10/13.
//

#include "../lib/TraceException.h"

TraceException::TraceException(const std::string &msg) {
    this->msg = msg;
    this->trace_back = "";
}

const char *TraceException::what() const noexcept {
    return exception::what();
}

std::string TraceException::get_trace_back() const {
    return this->trace_back;
}

void TraceException::add_trace_back(const std::string& name) {
    if (this->trace_back.empty()){
        this->trace_back += name;
    }else{
        this->trace_back += '\t';
        this->trace_back += name;
    }
}

std::string TraceException::get_msg() const {
    return this->msg;
}
