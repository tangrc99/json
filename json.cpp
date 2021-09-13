//
// Created by trc on 2021/9/11.
//

#include "json.h"
#include <fstream>

char moveNext(std::string &in_string) {
    char pop = in_string[0];
    in_string.assign(in_string.begin() + 1, in_string.end());
    return pop;
}

void removeSpace(std::string &in_string) {
    while (in_string[0] == ' ') {
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

JsonNode parseNumber(std::string &in_string) {
    removeSpace(in_string);

    std::string tmp;
    while (in_string[0] >= '0' && in_string[0] <= '9') {
        tmp += moveNext(in_string);
    }
    if (in_string[0] == '.') {
        while (in_string[0] >= '0' && in_string[0] <= '9') {
            tmp += moveNext(in_string);
        }
        return JsonNode(std::atof(tmp.c_str()));
    }
    return JsonNode(std::atoi(tmp.c_str()));
}

std::map<std::string, JsonNode> parseObject(std::string &in_string);

std::vector<JsonNode> parseArray(std::string &in_string) {
    ////inside an array may be object,number,string or another array.

    std::vector<JsonNode> value;

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


std::map<std::string, JsonNode> parseObject(std::string &in_string) {
    removeSpace(in_string);
    removeComment(in_string);

    if (in_string[0] == '{') {
        std::map<std::string, JsonNode> tmp;

        moveNext(in_string);

        while (in_string[0] != '}') {

            std::string key_string = parseString(in_string);

//            std::cout<<key_string<<std::endl;

            removeSignal(in_string);


            if (in_string[0] == '\"') {
                std::string value = parseString(in_string);

//                std::cout<<value<<std::endl;

                tmp.emplace(key_string, JsonNode(value));

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













JsonObject JsonParser::operator()(std::ifstream in_stream) try{
    std::string file_string, tmp;

    if (in_stream.is_open()) {
        while (std::getline(in_stream, tmp))
            file_string += tmp;
    } else
        throw std::runtime_error("Cannot open file!");

    return parseObject(file_string);
}catch(std::runtime_error &e){
    std::cout<<e.what();
    std::abort();
}

JsonObject JsonParser::operator()(std::string& str, MODE mode) {
    if(mode == MODE::FILE){
        std::ifstream in_stream(str);
        return this->operator()(std::move(in_stream));     ////////这里不确定是不是对的

    }else if(mode == MODE::STRING){
        return parseObject(str);
    }

}

using ValueType = BasicValue::ValueType;



std::string toString(const JsonObject &object);
void valueToString(JsonNode value,std::string &result);


void arrayToString(Array array , std::string &result){

    result+="[ ";

    if(array[0].type()==ValueType::OBJECT){

        for(JsonNode i:array){
            result+=toString(i.getObject());
            result+=", ";
        }
    }else if(array[0].type() == ValueType::ARRAY){
        for(JsonNode i:array){
            arrayToString(i.getArray(),result);
            result+=", ";
        }
    }else{
        for(const JsonNode& i:array){
            valueToString(i,result);
            result+=", ";
        }
    }
    result.erase(result.end() - 2, result.end());
    result += " ]";
}

void valueToString(JsonNode value,std::string &result){

    if(value.type() == ValueType::STRING){
        result+= '\"' + value.getValue() + '\"';

    }else if(value.type() == ValueType::INT){
        result+= std::to_string(value.getValue<int>());
    }else if(value.type() == ValueType::DOUBLE){
        result+= std::to_string(value.getValue<double>());
    }else if(value.type() == ValueType::BOOL){
        result+= std::to_string(value.getValue<bool>());
    }else if(value.type() == ValueType::OBJECT){
        result+=toString(value.getObject());
    }else if(value.type() == ValueType::ARRAY){
        arrayToString(value.getArray(),result);
    }
}

static int loop_flag = 1;

std::string toString(const JsonObject &object){
    std::string result("{ ");

    if(loop_flag==1)
        result+="\n";

    --loop_flag;

    for(const auto& i:object){

        result+='\"';
        result+=i.first;
        result+='\"';
        result+=": ";


        valueToString(i.second,result);
        result += ", \n";

    }
    ++loop_flag;

    result.erase(result.end()-3,result.end());
    if(loop_flag==1)
        result+="\n";
    result += "}";
    return result;
}





int main(){
    std::string file_name = R"(C:\Users\trc\Desktop\11.txt)";
    JsonParser  parser;

    //std::ifstream in_stream(file_name);

    //JsonObject p = parser(std::move(in_stream));
    JsonObject p = parser(file_name, JsonParser::FILE);
    JsonBuilder builder(&p);
    std::string k("111");
    JsonNode v(1);
    builder.addValue(k,v);
    //std::cout<<p.find("id")->second.getValue<int>();

    //p["ww"].getValue();

    std::cout<<toString(p);
    return 0;
}