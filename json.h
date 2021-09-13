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


class JsonNode;

typedef std::vector<JsonNode> Array;
typedef std::map<std::string, JsonNode> JsonObject;


class BasicValue {
    friend JsonNode;
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

template<typename T>
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

    JsonObject getObject() override {}

    Array getArray() override {}

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

    bool getBoolValue() {};

    JsonObject getObject() override {}

    Array getArray() override {}

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
    int getIntValue() override {}

    double getDoubleValue() override {}

    std::string getStringValue() override {}

    JsonObject getObject() override {}

    Array getArray() override {}

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
    double getDoubleValue() override { return 0; }

    int getIntValue() override { return 0; }

    bool getBoolValue() {};

    JsonObject getObject() override {}

    Array getArray() override {}

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
    JsonObject getObject() override {}


public:
    virtual int getIntValue() {}

    virtual double getDoubleValue() {}

    virtual std::string getStringValue() {}

    bool getBoolValue() {};

    int getValueType() override {
        return ARRAY;
    };

    Array getArray() override { return value; }

    explicit ArrayValue(const std::vector<JsonNode> &node_value) : NodeValue<std::vector<JsonNode>>(node_value) {}
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

    explicit ObjectValue(std::map<std::string, JsonNode> node_value) : NodeValue<JsonObject>(std::move(node_value)) {}
};


class JsonNode {


private:
    std::shared_ptr<BasicValue> _value;

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


    explicit JsonNode(const int &node_value) : _value(std::make_shared<IntValue>(node_value)) {}

    explicit JsonNode(const double &node_value) : _value(std::make_shared<DoubleValue>(node_value)) {}

    explicit JsonNode(const std::string &node_value) : _value(std::make_shared<StringValue>(node_value)) {}

    explicit JsonNode(const Array &node_value) : _value(std::make_shared<ArrayValue>(node_value)) {}

    explicit JsonNode(const JsonObject &node_value) : _value(std::make_shared<ObjectValue>(node_value)) {}

    explicit JsonNode(const bool &node_value) : _value(std::make_shared<BoolenValue>(node_value)) {}

    explicit JsonNode() : _value(nullptr) {}

};


struct JsonBuilder {
private:
    JsonObject *object;
public:
    explicit JsonBuilder(JsonObject *obj) : object(obj) {}

    bool addValue(const std::string &key, const JsonNode &value) try{
        object->emplace(key,value);

        return true;
    }catch(std::bad_alloc &e){
        std::cout << "Failed to create a json object.";
        return false;
    }

    bool addValue(const std::vector<std::string> &keys, const std::vector<JsonNode> &values) try {

        if (keys.size() != values.size())
            throw std::runtime_error("wrong key-value size");

        ////首先将这些作为键值对进行创建元素，然后再把 std::map 更新做到异常安全
        JsonObject tmp_object;
        for (int i = 0; i < keys.size(); i++)
            tmp_object.emplace(std::make_pair(keys[i], values[i]));
        object->swap(tmp_object);

        return true;
    } catch (std::runtime_error &e) {
        std::cerr << e.what();
        return false;
    } catch (std::bad_alloc &e) {
        std::cout << "Failed to create a json object.";
        return false;
    }

////功能函数
    JsonObject makeObject(const std::map<std::string, JsonNode> &key_values) {

    }

////接口函数
    JsonObject makeObject(const std::vector<std::string> &keys, const std::vector<JsonNode> &values) try {
        ////首先需要对输入的值进行检查，
        if (keys.size() != values.size())
            throw std::runtime_error("键值对大小不同");

    } catch (std::runtime_error &e) {
        std::cerr << e.what();
        return {};
    }

    template<typename T>
    Array makeArray(std::vector<T> array) {
        ////这里需要一个 array 中的元素类型相同

        ////把 array 中的数据类型转化为 JsonNode 格式

    }

};

class JsonParser {
private:


public:
    enum MODE {
        FILE, STRING
    };

    JsonObject operator()(std::string &file_name, MODE mode);

    JsonObject operator()(std::ifstream in_stream);


};

#endif //TREE_JSON_H
