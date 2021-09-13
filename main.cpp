#include "json.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>

char moveNext(std::string &in_string) {
    char pop = in_string[0];
    in_string.assign(in_string.begin() + 1, in_string.end());
    return pop;
}

void removeSpace(std::string &in_string) {
    while (in_string[0] == ' '||in_string[0] == '\r') {
        moveNext(in_string);
    }
}

void removeComment(std::string &in_string) {
    removeSpace(in_string);
    if (in_string[0] == '/') {
        moveNext(in_string);
        if (in_string[0] == '*') {
            while (in_string[0] != '/')
                moveNext(in_string);
            moveNext(in_string);
        }
    }
    removeSpace(in_string);
}

//
void removeSignal(std::string &in_string) {
    removeSpace(in_string);
    if (in_string[0] != ',' && in_string[0] != ':')
        return;
    else {
        moveNext(in_string);
    }
    removeSpace(in_string);
}


////忽略前面的空格
std::string parseString(std::string &in_string) try {
    removeSpace(in_string);

    if (in_string[0] != '\"')
        throw std::runtime_error("Not a string value");
    else {
        std::string parsed_string;

        moveNext(in_string);
        while (in_string[0] != '\"')
            parsed_string += moveNext(in_string);

        moveNext(in_string);        ////上面判断字符串结束，但是'\"'符号还没有被解析

        return parsed_string;
    }
} catch (std::runtime_error &e) {
    std::cout << e.what()  <<std::endl
    << "remained string : " <<in_string;

    return {};
}

JsonValue parseNumber(std::string &in_string) {
    removeSpace(in_string);

    std::string tmp;
    while (in_string[0] >= '0' && in_string[0] <= '9') {
        tmp += moveNext(in_string);
    }
    if (in_string[0] == '.') {
        while (in_string[0] >= '0' && in_string[0] <= '9') {
            tmp += moveNext(in_string);
        }
        return JsonValue(std::atof(tmp.c_str()));
    }
    return JsonValue(std::atoi(tmp.c_str()));
}

std::map<std::string, JsonValue> parseObject(std::string &in_string);

std::vector<JsonValue> parseArray(std::string &in_string) {
    ////inside an array may be object,number,string or another array.

    std::vector<JsonValue> value;

    moveNext(in_string);
    removeSpace(in_string);

    if (in_string[0] == '{') {
        while (in_string[0] != ']') {
            value.emplace_back(parseObject(in_string));
            removeSignal(in_string);
        }
        moveNext(in_string);

    } else if (in_string[0] == '\"') {
        while (in_string[0] != ']') {

            value.emplace_back(parseString(in_string));
            removeSignal(in_string);

        }
        moveNext(in_string);
    } else if (in_string[0] >= '0' && in_string[0] <= '9') {
        while (in_string[0] != ']') {
            value.emplace_back(parseNumber(in_string));
            removeSignal(in_string);
        }
        moveNext(in_string);

    } else if (in_string[0] == '[') {
        while (in_string[0] != ']') {

            value.emplace_back(parseArray(in_string));

            removeSignal(in_string);
        }
        moveNext(in_string);

    }
    return value;
}


std::map<std::string, JsonValue> parseObject(std::string &in_string) {
    removeSpace(in_string);
    removeComment(in_string);

    if (in_string[0] == '{') {
        std::map<std::string, JsonValue> tmp;

        moveNext(in_string);

        while (in_string[0] != '}') {

            std::string key_string = parseString(in_string);

//            std::cout<<key_string<<std::endl;

            removeSignal(in_string);


            if (in_string[0] == '\"') {
                std::string value = parseString(in_string);

//                std::cout<<value<<std::endl;

                tmp.emplace(key_string, JsonValue(value));

            } else if (in_string[0] >= '0' && in_string[0] <= '9') {   //// parse number

                tmp.emplace(key_string, parseNumber(in_string));

            } else if (in_string[0] == '{') {

                tmp.emplace(key_string, parseObject(in_string));

            } else if (in_string[0] == '[') {
                ////inside an array may be object,number,string.

                tmp.emplace(key_string, parseArray(in_string));

            }


            removeSignal(in_string);

        }

        moveNext(in_string);
        removeSpace(in_string);
        return tmp;
    }


}

int main(int argc, char *argv[]) try {

    //std::string file_name = R"(C:\Users\trc\Desktop\11.txt)";
    std::string file_name = argv[1];

    std::ifstream in_stream(file_name);

    std::string file_string, tmp;

    if (in_stream.is_open()) {
        while (std::getline(in_stream, tmp))
            file_string += tmp;
    } else
        throw std::runtime_error("Cannot open file!");

    //std::cout<<parseString(file_string);

    auto dic = parseObject(file_string);

//    for(auto i: dic)
//        std::cout<<i.first<<": " << i.second.getValue<int>()<<std::endl;

    std::cout << file_string << std::endl;
    if (file_string.empty())
        std::cout << "finished";

    return 0;
} catch (std::runtime_error &e) {
    std::cout << e.what() << std::endl;
    return 1;
}