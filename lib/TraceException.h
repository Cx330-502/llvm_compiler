//
// Created by 周霄 on 2023/10/13.
//

#ifndef COMPILER_LAB4_TRACEEXCEPTION_H
#define COMPILER_LAB4_TRACEEXCEPTION_H

#include "exception"
#include <string>

class TraceException : public std::exception{
public:
    explicit TraceException(const std::string& msg);
    [[nodiscard]] const char* what() const noexcept override;
    [[nodiscard]] std::string get_trace_back() const;
    std::string get_msg() const;
    void add_trace_back(const std::string& name);
private:
    std::string msg;
    std::string trace_back;
};


#endif //COMPILER_LAB4_TRACEEXCEPTION_H
