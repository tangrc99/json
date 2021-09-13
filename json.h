//
// Created by trc on 2021/9/11.
//

#ifndef TREE_JSON_H
#define TREE_JSON_H

#include <string>
#include <memory>
#include <utility>
#include <vector>
#include <iostream>
#include <map>
#include <mutex>
#include <fstream>


class JsonValue;

typedef std::vector<JsonValue> Array;
typedef std::map<std::string, JsonValue> JsonObject;

//// 定义一组允许被用户端来初始化的类型 ////
template<class T>
concept VALID_TYPE = (
        std::is_same_v<T, int> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, std::string> ||
        std::is_same_v<T, const char*>   ||
        std::is_same_v<T, float>    ||
        std::is_same_v<T,bool>  ||
        std::is_same_v<T,Array> ||
        std::is_same_v<T,JsonObject>
);

class BasicValue {
    friend JsonValue;
protected:

    virtual int getIntValue() = 0;

    virtual double getDoubleValue() = 0;

    virtual std::string getStringValue() = 0;

    virtual bool getBoolValue() = 0;

    virtual JsonObject getObject() = 0;

    virtual Array getArray() = 0;

    virtual int getValueType() = 0;

public:
    enum ValueType {
        INT, DOUBLE, BOOL, STRING, ARRAY, OBJECT
    };
};

template<VALID_TYPE T>
class NodeValue : public BasicValue {
protected:
    T value;

    explicit NodeValue(T node_value) : value(std::move(node_value)) {}
};

class IntValue final : public NodeValue<int> {
private:
    double getDoubleValue() override { return {}; }

    std::string getStringValue() override { return {}; }

    bool getBoolValue() override { return {}; }

    JsonObject getObject() override {return {};}

    Array getArray() override {return {};}

public:
    int getValueType() override {
        return INT;
    }

    int getIntValue() override {
        return value;
    }

    explicit IntValue(const int &node_value) : NodeValue<int>(node_value) {}
};

class DoubleValue final : public NodeValue<double> {
private:
    int getIntValue() override { return 0; }

    std::string getStringValue() override { return {}; }

    bool getBoolValue() override { return {};}

    JsonObject getObject() override {return {};}

    Array getArray() override {return {};}

public:
    int getValueType() override {
        return DOUBLE;
    }

    double getDoubleValue() override {
        return value;
    }

    explicit DoubleValue(const double &node_value) : NodeValue<double>(node_value) {}

};

class BoolenValue final : public NodeValue<bool> {
private:
    int getIntValue() override {return {};}

    double getDoubleValue() override {return {};}

    std::string getStringValue() override {return {};}

    JsonObject getObject() override {return {};}

    Array getArray() override {return {};}

public:
    int getValueType() override {
        return BOOL;
    };

    bool getBoolValue() override {
        return value;
    }

    explicit BoolenValue(const bool &node_value) : NodeValue<bool>(node_value) {}
};

class StringValue final : public NodeValue<std::string> {
private:
    double getDoubleValue() override { return {}; }

    int getIntValue() override { return {}; }

    bool getBoolValue() override{return {};}

    JsonObject getObject() override {return {};}

    Array getArray() override {return {};}

public:
    int getValueType() override {
        return STRING;
    }

    std::string getStringValue() override {
        return value;
    }

    explicit StringValue(const std::string &node_value) : NodeValue<std::string>(node_value) {}

};

class ArrayValue final : public NodeValue<Array> {
private:
    JsonObject getObject() override {return {};}

    int getIntValue() override{return {};}

    double getDoubleValue() override{return {};}

    std::string getStringValue() override{return {};}

    bool getBoolValue() override{return {};}

public:
    int getValueType() override {
        return ARRAY;
    };

    Array getArray() override { return value; }

    explicit ArrayValue(const std::vector<JsonValue> &node_value) : NodeValue<std::vector<JsonValue>>(node_value) {}
};

class ObjectValue final : public NodeValue<JsonObject> {
private:
    int getIntValue() override { return {}; }

    double getDoubleValue() override { return {}; }

    std::string getStringValue() override { return {}; }

    bool getBoolValue() override { return {}; }

    Array getArray() override { return {}; }

public:
    int getValueType() override {
        return OBJECT;
    };

    JsonObject getObject() override { return value; }

    explicit ObjectValue(std::map<std::string, JsonValue> node_value) : NodeValue<JsonObject>(std::move(node_value)) {}
};



//// 桥接模式，将六种不同的类型值抽象为一种 ////

class JsonParser;
class JsonValue {
    friend JsonParser;
private:
    std::shared_ptr<BasicValue> _value; //利用多态实现一种结点存储多个类型的值

public:
    void getValue(int &value) {
        value = _value->getIntValue();
    }

    void getValue(double &value) {
        value = _value->getDoubleValue();
    }

    void getValue(std::string &value) {
        value = _value->getStringValue();
    }


    ////通过模板返回类型和特例化，可以实现不同的返回类型
    template<typename T>
    T getValue() {
        switch (_value->getValueType()) {
            case BasicValue::ValueType::INT :
                return _value->getIntValue();
            case BasicValue::ValueType::DOUBLE :
                return _value->getDoubleValue();
            case BasicValue::ValueType::BOOL:
                return _value->getBoolValue();
        }
    }

    auto getValue() {
        return _value->getStringValue();
    }

    JsonObject getObject() {
        return _value->getObject();
    }

    Array getArray() {
        return _value->getArray();
    }

    int type() {
        return _value->getValueType();
    }


    explicit JsonValue(const int &node_value) : _value(std::make_shared<IntValue>(node_value)) {}

    explicit JsonValue(const double &node_value) : _value(std::make_shared<DoubleValue>(node_value)) {}

    explicit JsonValue(const std::string &node_value) : _value(std::make_shared<StringValue>(node_value)) {}

    explicit JsonValue(const char* node_value) : _value(std::make_shared<StringValue>(node_value)) {}

    explicit JsonValue(const Array &node_value) : _value(std::make_shared<ArrayValue>(node_value)) {}

    explicit JsonValue(const JsonObject &node_value) : _value(std::make_shared<ObjectValue>(node_value)) {}

    explicit JsonValue(const bool &node_value) : _value(std::make_shared<BoolenValue>(node_value)) {}

    explicit JsonValue() : _value(nullptr) {}

};


struct JsonBuilder {
private:
    JsonObject *object;
public:
    explicit JsonBuilder(JsonObject *obj) : object(obj) {}



    template<VALID_TYPE T>
    bool addValue(const std::string &key,const T &value){
        return addValue(key,JsonValue(value));
    }

    ////const char* 和 const T&的接口不是通用的
    bool addValue(const std::string &key,const char *value){
        return addValue(key,JsonValue(value));
    }

    bool addValue(const std::string &key, const JsonValue &value) try{
        object->emplace(key,value);

        return true;
    }catch(std::bad_alloc &e){
        std::cout << "Failed to create a json object.";
        return false;
    }

    bool addValue(const std::vector<std::string> &keys, const std::vector<JsonValue> &values) try {

        if (keys.size() != values.size())
            throw std::runtime_error("wrong key-value size");

        ////首先将这些作为键值对进行创建元素，然后再把 std::map 更新做到异常安全
        JsonObject tmp_object;
        for (int i = 0; i < keys.size(); i++)
            tmp_object.emplace(keys[i], values[i]);
        object->insert(tmp_object.begin(),tmp_object.end());

        return true;
    } catch (std::runtime_error &e) {
        std::cerr << e.what();
        return false;
    } catch (std::bad_alloc &e) {
        std::cout << "Failed to create a json object.";
        return false;
    }



////////////修改 json/////////////
    template<VALID_TYPE T>
    bool reviseJsonNode(const std::string &key,const T &value){
        return reviseJsonNode(key,JsonValue(value));
    }

    bool reviseJsonNode(const std::string &key,const char *value){
        return reviseJsonNode(key,JsonValue(value));
    }

    bool reviseJsonNode(const std::string &key,const JsonValue &value)try{

        auto i = object->find(key);
        if(i == object->end())
            throw std::runtime_error(" doesn't exist.");
        i->second = value;
        return true;

    }catch(std::runtime_error &e){
        std::cout<<key<<e.what();
        return false;
    }


///////////删除 json/////////////
    bool deleteJsonNode(const std::string &key)try {
        object->erase(key);
        return true;
    }catch(...){
        std::cout<<"Failed to delete";
        return false;
    }
////功能函数
    static JsonObject makeObject(const std::map<std::string, JsonValue> &key_values) {
        return key_values;
    }

////接口函数
    static JsonObject makeObject(const std::vector<std::string> &keys, const std::vector<JsonValue> &values) try {
        ////首先需要对输入的值进行检查，
        if (keys.size() != values.size())
            throw std::runtime_error("键值对大小不同");

        JsonObject tmp_object;
        for (int i = 0; i < keys.size(); i++)
            tmp_object.emplace(keys[i], values[i]);
        return tmp_object;

    } catch (std::runtime_error &e) {
        std::cerr << e.what();
        return {};
    }

    template<VALID_TYPE T>
    static Array makeArray(const std::vector<T> &array) {
        ////这里需要一个 array 中的元素类型相同
        std::vector<JsonValue> tmp_array;
        ////把 array 中的数据类型转化为 JsonValue 格式
        for(auto i:array)
            tmp_array.template emplace_back(i);
        return tmp_array;
    }

};

class JsonParser {
private:

    int is_outer = 1;   //用于确认 toString 的递归层数，从而实现外部 object 的{}换行

    //// 从流、字符串转化为 Json 对象的内部实现 ////
    static inline char moveNext(std::string &in_string);
    static inline void removeSpace(std::string &in_string);
    static inline void removeComment(std::string &in_string);
    static inline void removeSignal(std::string &in_string);
    static std::string parseString(std::string &in_string);
    static JsonValue parseNumber(std::string &in_string);
    static JsonValue parseBool(std::string &in_string);
    std::vector<JsonValue> parseArray(std::string &in_string);
    std::map<std::string, JsonValue> parseObject(std::string &in_string);

    static std::string tryProcessError(std::string &in_string){
        std::string o_string;
        while(isalpha(in_string[0])){
            o_string += moveNext(in_string);
        }
        return o_string;
    }

    //// 将 Json 对象序列化为 std::string 的内部实现 ////
    void arrayToString(Array array, std::string &result);
    void valueToString(JsonValue value, std::string &result);



public:
    enum MODE {
        FILE, STRING
    };
    //// 用户接口，使用（ ）从流、字符串转化为 Json 对象 ////
    JsonObject operator()(std::string &file_name, MODE mode);

    JsonObject operator()(std::ifstream in_stream);




    std::string toString(const JsonObject &object) ;

    std::ofstream toFile(const JsonObject &object){
        return std::ofstream(toString(object));
    }

    };

#endif //TREE_JSON_H
