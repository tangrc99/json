//
// Created by trc on 2021/9/11.
//

#include "json.h"
#include <fstream>

namespace json {

/////////////////////////   string,stream -> Json //////////////////////
////////////////////////////////////////////////////////////////////////

//// 处理一个字符串，并将当前位置的 char 弹出 ////
    inline char JsonParser::moveNext(std::string &in_string) {
        char pop = in_string[0];
        in_string.assign(in_string.begin() + 1, in_string.end());
        return pop;
    }

//// 清除字符串中的空格和回车符 ////
    inline void JsonParser::removeSpace(std::string &in_string) {
        //// 清除 空格、换行、回车、制表符 ////
        while (in_string[0] == ' ' || in_string[0] == '\r' || in_string[0] == '\t' || in_string[0] == '\n') {
            moveNext(in_string);
        }
    }

//// 清除注释（只支持 /* */格式）////
    inline void JsonParser::removeComment(std::string &in_string) {
        removeSpace(in_string);
        if (in_string[0] == '/') {
            moveNext(in_string);    //跳出注释的第一个'/'
            if (in_string[0] == '*') {
                while (in_string[0] != '/')
                    moveNext(in_string);
                moveNext(in_string);    //跳出注释的第二个'/'
            }
        }
        removeSpace(in_string);
    }

//// 清除字符串中的 ','和':' 符号 ////
    inline void JsonParser::removeSignal(std::string &in_string) {
        removeSpace(in_string);
        if (in_string[0] != ',' && in_string[0] != ':')
            return;
        else {
            moveNext(in_string);
        }
        removeSpace(in_string);
    }

//// 默认字符串开始是没有空格的 ////
//// 其他函数保证将空格全部去除 ////
    std::string JsonParser::parseString(std::string &in_string) noexcept try {
        removeSpace(in_string);

        if (in_string[0] != '\"')
            throw std::runtime_error("ERROR TYPE");
        else {
            std::string parsed_string;

            moveNext(in_string);    //跳出字符串第一个"符号

            //// 这里的异常处理还没有想好，现在的异常处理花费代价稍高 ////
            //// 并且这里会要求字符串当中不能有 },]等 ////
            while (in_string[0] != '\"') {
                if (in_string[0] == ',' || in_string[0] == '}' || in_string[0] == ']' || in_string[0] == ' ' ||
                    in_string[0] == ':')
                    throw std::out_of_range(parsed_string);
                parsed_string += moveNext(in_string);
            }

            moveNext(in_string);        //跳出字符串第二个"符号

            return parsed_string;
        }
    } catch (std::runtime_error &e) {

        return tryProcessTypeError(in_string, e.what());

    } catch (std::out_of_range &e) {
        std::cerr << "字符串超出范围" << std::endl
                  << "Error string : " << e.what() << std::endl
                  << "Remained string :" << e.what() << in_string << std::endl << std::endl;
        return e.what();
    }


//// 这里如果数字中间出现了字符没有考虑，也没有考虑科学计数法
    JsonValue JsonParser::parseNumber(std::string &in_string) {
        removeSpace(in_string);

        std::string tmp;

        if (in_string[0] == '0') {   //保证数字不以 0 开头
            if (in_string[1] >= '0' && in_string[1] <= '9')
                moveNext(in_string);
        }

        while (in_string[0] >= '0' && in_string[0] <= '9') {
            tmp += moveNext(in_string);
        }

        if (in_string[0] == '.') {

            tmp += moveNext(in_string);

            while (in_string[0] >= '0' && in_string[0] <= '9') {
                tmp += moveNext(in_string);
            }
            return JsonValue(std::atof(tmp.c_str()));
        }

        return JsonValue(std::atoi(tmp.c_str()));
    }

//// 字符串开头和结尾都是“”，因此只用分辨 TRUE 和 FALSE ////

//// 但是这样可能会出现异常，会多出来一个空的键值对
    JsonValue JsonParser::parseBool(std::string &in_string) try {
        bool is_true = false;

        if (in_string[0] == 't') {
            is_true = true;
        }

        in_string.assign(in_string.begin() + 4 + !is_true, in_string.end());

        if (std::isalpha(in_string[0]))
            throw std::out_of_range(is_true ? "true" : "false");

        return is_true ? JsonValue(true) : JsonValue(false);

    } catch (std::out_of_range &e) {
        std::cerr << "ERROR BOOL VALUE :" << e.what() << std::endl
                  << "Remained string :" << in_string << std::endl << std::endl;
        while (std::isalpha(in_string[0]))
            moveNext(in_string);
        return (e.what()[0] == 't') ? JsonValue(true) : JsonValue(false);
    }


    std::vector<JsonValue> JsonParser::parseArray(std::string &in_string) {
        ////inside an array may be object,number,string or another array.

        std::vector<JsonValue> value; //为返回值创建临时变量

        moveNext(in_string);    //去除数组前的 '['
        removeSpace(in_string);

        ////在数组还没有结束前，解析同一种类的变量，变量之间是用,隔开的
        while (in_string[0] != ']') {
            if (in_string[0] == '{') {

                value.emplace_back(parseObject(in_string)); //将一个 value 放入对象

            } else if (in_string[0] == '\"') {

                value.emplace_back(parseString(in_string));

            } else if (in_string[0] >= '0' && in_string[0] <= '9') {

                value.emplace_back(parseNumber(in_string));

            } else if (in_string[0] == 't' || in_string[0] == 'f') {

                value.emplace_back(parseBool(in_string));

            } else if (in_string[0] == '[') {

                value.emplace_back(parseArray(in_string));

            } else {   //// 异常处理，同 parseObject 部分
                std::string tmp_value_string = parseString(in_string);

                tmp_value_string.empty() ? value.emplace_back(JsonValue()) : value.emplace_back(parseBool(in_string));

            }
            removeSignal(in_string);    //去除[,,]中间隔的,
        }

        moveNext(in_string);    //去除数组后的 ']'
        return value;
    }

    std::map<std::string, JsonValue> JsonParser::parseObject(std::string &in_string) {
        removeSpace(in_string);
        removeComment(in_string);

        //// json 对象是由键值对组成的，分别对 key 和 value 解析
        if (in_string[0] == '{') {
            std::map<std::string, JsonValue> tmp_object;   //为返回值创建临时变量

            moveNext(in_string);    //去除对象的第一个{

            while (in_string[0] != '}') {

                //// 保存 json 对象中的每一个 key ////
                std::string key_string = parseString(in_string);

                removeSignal(in_string);    //清除 key:value 中的 :

                //// 根据不同格式解析 value ////
                if (in_string[0] == '\"') {

                    tmp_object.emplace(key_string, parseString(in_string));

                } else if (in_string[0] >= '0' && in_string[0] <= '9') {   //// parse number

                    tmp_object.emplace(key_string, parseNumber(in_string));

                } else if (in_string[0] == 't' || in_string[0] == 'f') {

                    tmp_object.emplace(key_string, parseBool(in_string));

                } else if (in_string[0] == '{') {

                    tmp_object.emplace(key_string, parseObject(in_string));

                } else if (in_string[0] == '[') {

                    tmp_object.emplace(key_string, parseArray(in_string));
                } else {    //// 异常处理，可能为空值，拼写错误
                    std::string tmp_value_string = parseString(in_string);

                    tmp_value_string.empty() ? tmp_object.emplace(key_string, JsonValue()) : tmp_object.emplace(
                            key_string,
                            tmp_value_string);
                }
                removeSignal(in_string);    //每个 key :value 有, 隔开
            }

            moveNext(in_string);    //去除对象的第二个{
            removeSpace(in_string);     //文件末尾可能会有空格和换行
            return tmp_object;
        }
    }

//// 用户接口，将 parseObject 函数进行封装 ////
    JsonObject JsonParser::operator()(std::ifstream in_stream) try {
        std::string file_string, tmp;

        if (in_stream.is_open()) {
            while (std::getline(in_stream, tmp))
                file_string += tmp;
        } else
            throw std::runtime_error("Cannot open file!");

        return parseObject(file_string);
    } catch (std::runtime_error &e) {
        std::cout << e.what();
        std::abort();
    }

//// 用户接口，将 parseObject 函数进行封装 ////
    JsonObject JsonParser::operator()(std::string &str, MODE mode) {
        if (mode == MODE::FILE) {

            return this->operator()(std::ifstream(str));     ////////这里不确定是不是对的

        } else if (mode == MODE::STRING) {
            return parseObject(str);
        }
        return {};
    }


/////////////////////////  Json -> string,stream  //////////////////////
////////////////////////////////////////////////////////////////////////

    using ValueType = BasicValue::ValueType;

//// 将 Array 对象序列化为 std::string ////
//// 由于对用户隐藏接口，因此使用传引用 ////
    void JsonParser::arrayToString(Array array, std::string &result) {

        result += "[ ";

        if (array[0].type() == ValueType::OBJECT) {

            for (JsonValue i: array) {
                result += toString(i.getObject());
                result += ", ";
            }
        } else if (array[0].type() == ValueType::ARRAY) {
            for (JsonValue i: array) {
                arrayToString(i.getArray(), result);
                result += ", ";
            }
        } else {
            for (const JsonValue &i: array) {
                valueToString(i, result);
                result += ", ";
            }
        }
        result.erase(result.end() - 2, result.end());
        result += " ]";
    }

//// 将非递归形式的值序列化为 std::string 的格式 ////
    void JsonParser::valueToString(JsonValue value, std::string &result) {

        switch (value.type()) {
            case ValueType::STRING:
                result += '\"' + value.getValue() + '\"';
                return;

            case ValueType::INT:
                result += std::to_string(value.getValue<int>());
                return;

            case ValueType::DOUBLE:
                result += std::to_string(value.getValue<double>());
                return;

            case ValueType::BOOL:
                result += value.getValue<bool>() ? "true" : "false";
                return;

            case ValueType::OBJECT:
                result += toString(value.getObject());
                return;

            case ValueType::ARRAY:
                arrayToString(value.getArray(), result);
                return;

            case ValueType::NULL_VALUE:
                result += ' ';
                return;
        }
    }

//// 通过递归，将 JsonObject 对象序列化为 std::string ////
//// 这是用户接口，因而选择 return value 的格式，方便调用 ////
    std::string JsonParser::toString(const JsonObject &object) {
        std::string result("{ ");

        if (is_outer == 1)
            result += "\n";

        --is_outer; //每次可能进入递归循环时，标志减一

        for (const auto &i: object) {

            result += '\"' + i.first + '\"' + ": ";         // key-value 中的 key

            valueToString(i.second, result);    //key-value 中的 value

            result += ", \n";   //每个 key-value 之间分隔并换行

        }

        ++is_outer; //每次离开递归循环时，标志加一

        result.erase(result.end() - 3, result.end());
        if (is_outer == 1)
            result += "\n";
        result += "}";
        return result;
    }

    std::string JsonParser::tryProcessTypeError(std::string &in_string, const std::string &error_inf) {
        std::string o_string{};

        if (in_string[0] == ',' || in_string[0] == '}' || in_string[0] == ']' || in_string[0] == ' ')
            return {};
        else {
            std::cerr << error_inf << std::endl
                      << "Error string : " << in_string.substr(0, in_string.find_first_of('\"') + 1)
                      << std::endl
                      << "remained string : " << in_string << std::endl << std::endl;
            while (isalpha(in_string[0])) {
                o_string += moveNext(in_string);
            }
            if (in_string[0] == '\"')      ////顺序不能改变，否则会破坏下一个键值对
                moveNext(in_string);
            return o_string;
        }
    }

    template<typename T>
    T getValue(const JsonObject &obj, const std::string &key) {
        switch (obj.find(key)->second.type()) {
            case BasicValue::ValueType::INT :
                return obj.find(key)->second.getValue<int>();
            case BasicValue::ValueType::DOUBLE :
                return obj.find(key)->second.getValue<double>();
            case BasicValue::ValueType::BOOL:
                return obj.find(key)->second.getValue<bool>();
        }
    }

    std::string getStringValue(const JsonObject &obj, const std::string &key) {
        return obj.find(key)->second.getValue();
    }

    JsonObject getObject(const JsonObject &obj, const std::string &key) {
        return obj.find(key)->second.getObject();
    }

    Array getArray(const JsonObject &obj, const std::string &key) {
        return obj.find(key)->second.getArray();
    }


}//namespace json






using namespace json;

int main(int argc, char *argv[]) {
    std::string file_name = argv[1];

    //从文件中读取流，解析为数据结构
    JsonParser parser;
    JsonObject p = parser(file_name, JsonParser::FILE);

    //利用建造者模式在原有数据结构上增删
    JsonBuilder builder(&p);

    //增加两个键值对
    std::vector<std::string> k{"111", "222"};
    std::vector<JsonValue> v;
    v.emplace_back();
    v.emplace_back(true);
    builder.addValue(k, v);

    //增加一个键值对，其 value 为一个 object
    builder.addValue("23r", JsonBuilder::makeObject(k, v));

    builder.reviseJsonNode("111", "revise");
    builder.deleteJsonNode("111");

    //输出到文件
    std::ofstream os(argv[2]);
    os << parser.toString(p);

    //// 考虑单独输出一个 object 中的 value
    std::cout << getValue<double>(p, "version") << std::endl;

    std::cout << parser.toString(getObject(p, "23r")) << std::endl;

    std::cout << parser.toString(p) << std::endl;

    return 0;
}

