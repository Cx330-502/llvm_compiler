//
// Created by 周霄 on 2023/9/24.
//

#include "../lib/Lexer.h"
#include <iostream>
Word::Word() {
    node_type = 0;
    line = 0;
}

Lexer::Lexer(const std::string& infile_path, const std::string& outfile_path, Compiler* compiler) {
    line_num = 0;
    this->compiler = compiler;
    this->infile_path = infile_path;
    this->outfile_path = outfile_path;
    infile.open(infile_path);
    outfile.open(outfile_path);
    if(!infile.is_open()){
        throw std::runtime_error("Cannot open file: " + infile_path);
    }
    if(!outfile.is_open()){
        throw std::runtime_error("Cannot open file: " + outfile_path);
    }
    infile.close();
    outfile.close();
}

std::vector<std::string> Lexer::type_list = {
        "IDENFR","INTCON","STRCON","MAINTK","CONSTTK","INTTK","BREAKTK",
        "CONTINUETK","IFTK","ELSETK","NOT","AND","OR","FORTK","GETINTTK",
        "PRINTFTK","RETURNTK","VOIDTK","PLUS","MINU","MULT","DIV","MOD",
        "LSS","LEQ","GRE","GEQ","EQL","NEQ","ASSIGN","SEMICN","COMMA",
        "LPARENT","RPARENT","LBRACK","RBRACK","LBRACE","RBRACE"
};

void Lexer::push_word(Word &word) {
    word.line = line_num;
    this->wordlist.push_back(word);

//        outfile<<word.code<<" "<<word.value<<std::endl;
//    catch (std::exception& e) {
//        std::cout << "Error: " << e.what() << std::endl;
//    }
}

void Lexer::analyse() {
    try{
        this->infile.open(infile_path);
        this->outfile.open(outfile_path);
        if(!infile.is_open()){
            throw std::runtime_error("Cannot open file: " + infile_path);
        }
        if(!outfile.is_open()){
            throw std::runtime_error("Cannot open file: " + outfile_path);
        }
        std::string line;
        line_num = 0;
        bool flag = false;
        while(getline(infile,line)){
            int index = 0;
            int printf_flag = 0;
            line_num++;
            try {
                while(index<line.length()){
                    char token = line[index];
                    //最后一个字符
//                    if (index == line.length()-1)
//                        std::cout<<"Last character is : "<<token<<"   ASCII: "<<static_cast<int>(token)<<std::endl;
                    //如果在注释中
                    if (flag){
                        if (token == '*'){
                            index++;
                            token = line[index];
                            if (token == '/'){
                                index++;
                                token = line[index];
                                flag = false;
                                continue;
                            }else{
                                continue;
                            }
                        }else{
                            index++;
                            continue;
                        }
                    }
                    //字母
                    if ((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_')) {
                        analyse_word(line, index, printf_flag, token);
                        continue;
                    }
                    //数字(无前导0)
                    if (token >= '0' && token <= '9'){
                        analyse_number(line, index, token);
                        continue;
                    }
                    //FormatString
                    if (token == '"') {
                        analyse_formatString(line, index, printf_flag, token);
                        continue;
                    }
                    //字符
                    {
                        int temp = analyse_char(line, index, printf_flag, flag, token);
                        if (temp == 0) continue;
                        else if (temp == 1) break;
                    }
                    //空格
                    if ( token == ' ' || token == '\t' || token == '\n' || token == '\r'){
                        index++;
                        continue;
                    }
                    //其他字符
                    {
                        Word word = *new Word();
                        word.value += token;
                        index++;
                        //这有一个bug接口
                        continue;
                    }
                }
            }catch (std::exception& e){
                std::cout << "Error: " << e.what() << std::endl;
            }
        }
        infile.close();
        outfile.close();
    }catch (std::exception& e){
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int Lexer::analyse_char(const std::string &line, int &index, int &printf_flag, bool &flag, char &token) {
    if (token == '!'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        if (token == '='){
            word.value += token;
            index++;
            word.type = "NEQ";
            push_word(word);
            return 0;
        }else{
            word.type = "NOT";
            push_word(word);
            return 0;
        }
    }
    if (token == '&'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        if (token == '&'){
            word.value += token;
            index++;
            word.type = "AND";
            push_word(word);
            return 0;
        }else{
            //这有一个bug接口
            return 0;
        }
    }
    if (token == '|'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        if (token == '|'){
            word.value += token;
            index++;
            word.type = "OR";
            push_word(word);
            return 0;
        }else{
            //这有一个bug接口
            return 0;
        }
    }
    if (token == '+'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "PLUS";
        push_word(word);
        return 0;
    }
    if (token == '-'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "MINU";
        push_word(word);
        return 0;
    }
    if (token == '*'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "MULT";
        push_word(word);
        return 0;
    }
    //除和注释
    if (token == '/'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        if (token == '/') return 1;
        if (token == '*'){
            index++;
            flag = true ;
            return 0;
        }
        word.type = "DIV";
        push_word(word);
        return 0;
    }
    if (token == '%'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "MOD";
        push_word(word);
        return 0;
    }
    if (token == '<'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        if (token == '='){
            word.value += token;
            index++;
            word.type = "LEQ";
            push_word(word);
            return 0;
        }else{
            word.type = "LSS";
            push_word(word);
            return 0;
        }
    }
    if (token == '>'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        if (token == '='){
            word.value += token;
            index++;
            word.type = "GEQ";
            push_word(word);
            return 0;
        }else{
            word.type = "GRE";
            push_word(word);
            return 0;
        }
    }
    if (token == '='){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        if (token == '='){
            word.value += token;
            index++;
            word.type = "EQL";
            push_word(word);
            return 0;
        }else{
            word.type = "ASSIGN";
            push_word(word);
            return 0;
        }
    }
    if (token == ';'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "SEMICN";
        push_word(word);
        return 0;
    }
    if (token == ','){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "COMMA";
        push_word(word);
        return 0;
    }
    if (token == '('){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "LPARENT";
        push_word(word);
        if (printf_flag == 1) printf_flag++;
        return 0;
    }
    if (token == ')'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "RPARENT";
        push_word(word);
        if (printf_flag == 3) printf_flag = 0;
        return 0;
    }
    if (token == '['){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "LBRACK";
        push_word(word);
        return 0;
    }
    if (token == ']'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "RBRACK";
        push_word(word);
        return 0;
    }
    if (token == '{'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "LBRACE";
        push_word(word);
        return 0;
    }
    if (token == '}'){
        Word word = *new Word();
        word.value += token;
        index++;
        token = line[index];
        word.type = "RBRACE";
        push_word(word);
        return 0;
    }
    return 2;
}

void Lexer::analyse_formatString(const std::string &line, int &index, int &printf_flag, char &token) {
    Word word = *new Word();
    word.value += token;
    index++;
    token = line[index];
    if (printf_flag !=2) {
        //这有一个bug接口
    }
//    index -- ;
    printf_flag++;
    while (token != '"'){
        if (index >= line.length()) break;
        token = line[index];
        //确保字符类型
        if (token != 32 && token != 33 && (token > 126 || token < 40) && token != '"' && token != '%'){
            Error error(line_num, "a", "非法符号", "字符串中出现非法字符");
            error_list.push_back(error);
            word.value +=token;
            index++;
            token = line[index];
            continue;
        }
        //确保\后只有n
        if (token == '\\'){
            word.value += token;
            index++;
            token = line[index];
            if (token != 'n'){
                Error error(line_num, "a", "非法符号", "字符串中出现非法字符");
                error_list.push_back(error);
                if (token == '"') break;
            }
            word.value += token;
            index++;
            token = line[index];
            continue;
        }
        //确保%后面只有d
        if (token == '%'){
            word.value += token;
            index++;
            token = line[index];
            if (token != 'd'){
                Error error(line_num, "a", "非法符号", "字符串中出现非法字符");
                error_list.push_back(error);
                if (token == '"') break;
            }
            word.value += token;
            index++;
            token = line[index];
            continue;
        }
        word.value += token;
        index++;
        token = line[index];
    }
    if (!wordlist.empty() && wordlist[wordlist.size() - 1].type == "ERROR"){
        return;
    }
    word.value += token;
    index ++ ;
    word.type = "STRCON";
    push_word(word);
}

void Lexer::analyse_number(const std::string &line, int &index, char &token) {
    Word word = *new Word();
    while(token >= '0' && token <= '9'){
        word.value += token;
        index++;
        if(index>=line.length()) break;
        token = line[index];
    }
    if (word.value.length() > 1 && word.value[0] == '0'){
        //这有一个bug接口
        return;
    }
    word.type = "INTCON";
    push_word(word);
}

void Lexer::analyse_word(const std::string &line, int &index, int &printf_flag, char &token) {
    Word word = *new Word();
    while((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z')
                || (token >= '0' && token <= '9') || (token == '_')){
        word.value += token;
        token = line[++index];
        if(index == line.length())
            break;
    }
    if (word.value == "main") word.type = "MAINTK";
    else if (word.value == "const") word.type = "CONSTTK";
    else if (word.value == "int") word.type = "INTTK";
    else if (word.value == "break") word.type = "BREAKTK";
    else if (word.value == "continue")  word.type = "CONTINUETK";
    else if (word.value == "if") word.type = "IFTK";
    else if (word.value == "else") word.type = "ELSETK";
    else if (word.value == "for") word.type = "FORTK";
    else if (word.value == "getint") word.type = "GETINTTK";
    else if (word.value == "printf") {word.type = "PRINTFTK";printf_flag = 1;}
    else if (word.value == "return") word.type = "RETURNTK";
    else if (word.value == "void") word.type = "VOIDTK";
    else word.type = "IDENFR";
    push_word(word);
}


