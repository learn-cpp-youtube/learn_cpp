#pragma once

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace json
{

struct Value; // Wrapper for variant<nullptr_t,bool,int64,double,string,uptr<Array>,uptr<Object>>
using  Array  = std::vector<Value>;
using  Object = std::map<std::string, Value>;

struct Value
{
    Value() : data{nullptr} {}
    Value(const Value& v);
    Value(Value&& v) noexcept : data{std::move(v.data)} {}

    Value(std::nullptr_t) : data{nullptr} {}
    Value(bool b)         : data{b} {}
    Value(int i)          : data{static_cast<std::int64_t>(i)} {}
    Value(std::int64_t i) : data{i} {}
    Value(double d)       : data{d} {}
    Value(std::string s)  : data{std::move(s)} {}
    Value(const char* s)  : data{std::string{s}} {}
    Value(Array a)        : data{std::make_unique<Array>(std::move(a))} {}
    Value(Object obj)     : data{std::make_unique<Object>(std::move(obj))} {}

    template<typename T>
    Value(const std::vector<T>& vec)
    {
        std::unique_ptr<Array> p = std::make_unique<Array>();
        
        p->reserve(vec.size());
        for (const auto& e : vec)
            p->push_back(e);

        data = std::move(p);
    }

    template<typename T>
    Value(std::vector<T>&& vec)
    {
        std::unique_ptr<Array> p = std::make_unique<Array>();
        
        p->reserve(vec.size());
        for (auto& e : vec)
            p->push_back(std::move(e));

        data = std::move(p);
    }

    template<typename T>
    Value(const std::map<std::string, T>& m)
    {
        std::unique_ptr<Object> p = std::make_unique<Object>();
        
        for (const auto& e : m)
            p->insert({e.first, e.second});

        data = std::move(p);
    }

    template<typename T>
    Value(std::map<std::string, T>&& m)
    {
        std::unique_ptr<Object> p = std::make_unique<Object>();
        
        for (auto& e : m)
            p->insert({e.first, std::move(e.second)});

        data = std::move(p);
    }

    Value& operator=(const Value& v);    
    Value& operator=(Value&& v) noexcept;
    
    Value& operator=(std::nullptr_t) { data = nullptr; return *this; }
    Value& operator=(bool b)         { data = b; return *this; }
    Value& operator=(int i)          { data = static_cast<std::int64_t>(i); return *this; }
    Value& operator=(std::int64_t i) { data = i; return *this; }
    Value& operator=(double d)       { data = d; return *this; }
    Value& operator=(std::string s)  { data = std::move(s); return *this; }
    Value& operator=(const char* s)  { data = std::string{s}; return *this; }
    Value& operator=(Array a)        { data = std::make_unique<Array>(std::move(a)); return *this; }
    Value& operator=(Object obj)  { data = std::make_unique<Object>(std::move(obj)); return *this; }

    template<typename T>
    Value& operator=(const std::vector<T>& vec)
    {
        std::unique_ptr<Array> p = std::make_unique<Array>();
        
        p->reserve(vec.size());
        for (const auto& e : vec)
            p->push_back(e);

        data = std::move(p);
        return *this;
    }

    template<typename T>
    Value& operator=(std::vector<T>&& vec)
    {
        std::unique_ptr<Array> p = std::make_unique<Array>();
        
        p->reserve(vec.size());
        for (auto& e : vec)
            p->push_back(std::move(e));

        data = std::move(p);
        return *this;
    }

    template<typename T>
    Value& operator=(const std::map<std::string, T>& m)
    {
        std::unique_ptr<Object> p = std::make_unique<Object>();
        
        for (const auto& e : m)
            p->insert({e.first, e.second});

        data = std::move(p);
        return *this;
    }

    template<typename T>
    Value& operator=(std::map<std::string, T>&& m)
    {
        std::unique_ptr<Object> p = std::make_unique<Object>();
        
        for (auto& e : m)
            p->insert({e.first, std::move(e.second)});

        data = std::move(p);
        return *this;
    }

    bool IsNull()    const { return std::holds_alternative<std::nullptr_t>(data); }
    bool IsBool()    const { return std::holds_alternative<bool>(data); }
    bool IsInteger() const { return std::holds_alternative<std::int64_t>(data); }
    bool IsDouble()  const { return std::holds_alternative<double>(data); }
    bool IsNumber()  const { return IsInteger() || IsDouble(); }
    bool IsString()  const { return std::holds_alternative<std::string>(data); }
    bool IsArray()   const { return std::holds_alternative<std::unique_ptr<Array>>(data); }
    bool IsObject()  const { return std::holds_alternative<std::unique_ptr<Object>>(data); }

    std::nullptr_t&    GetNull()          { return std::get<std::nullptr_t>(data); }
    std::nullptr_t     GetNull()    const { return std::get<std::nullptr_t>(data); }
    bool&              GetBool()          { return std::get<bool>(data); }
    bool               GetBool()    const { return std::get<bool>(data); }
    std::int64_t&      GetInteger()       { return std::get<std::int64_t>(data); }
    std::int64_t       GetInteger() const { return std::get<std::int64_t>(data); }
    double&            GetDouble()        { return std::get<double>(data); }
    double             GetDouble()  const { return std::get<double>(data); }
    std::string&       GetString()        { return std::get<std::string>(data); }
    const std::string& GetString()  const { return std::get<std::string>(data); }
    Array&             GetArray()         { return *std::get<std::unique_ptr<Array>>(data); }
    const Array&       GetArray()   const { return *std::get<std::unique_ptr<Array>>(data); }
    Object&            GetObject()        { return *std::get<std::unique_ptr<Object>>(data); }
    const Object&      GetObject()  const { return *std::get<std::unique_ptr<Object>>(data); }
    double GetNumber() const { return IsInteger()?static_cast<double>(GetInteger()):GetDouble(); }

    std::variant<std::nullptr_t,
                 bool,
                 std::int64_t,
                 double,
                 std::string,
                 std::unique_ptr<Array>,
                 std::unique_ptr<Object>> data;
};

Value Parse(std::istream& in);
Value Parse(std::string in);
Value ParseFile(const std::string& filename);

std::string Output(const Value& v, bool pretty = true, int tabWidth = 4);
void Output(std::ostream& out, const Value& v, bool pretty = true, int tabWidth = 4);
void SaveToFile(const std::string& filename, const Value& v, bool pretty = true, int tabWidth = 4);

std::ostream& operator<<(std::ostream& out, const Value& v);

template<typename T>
std::vector<T> ConvertArrayOfBools(const Array& a)
{
    std::vector<T> v;

    v.reserve(a.size());
    for (const auto& e : a)
        v.push_back(e.GetBool());

    return v;
}

template<typename T>
std::vector<T> ConvertArrayOfInts(const Array& a)
{
    std::vector<T> v;

    v.reserve(a.size());
    for (const auto& e : a)
        v.push_back(e.GetInteger());

    return v;
}

template<typename T>
std::vector<T> ConvertArrayOfDoubles(const Array& a)
{
    std::vector<T> v;

    v.reserve(a.size());
    for (const auto& e : a)
        v.push_back(e.GetDoubles());

    return v;
}

template<typename T>
std::vector<T> ConvertArrayOfNumbers(const Array& a)
{
    std::vector<T> v;

    v.reserve(a.size());
    for (const auto& e : a)
        v.push_back(e.GetNumber());

    return v;
}

std::vector<std::string> ConvertArrayOfStrings(const Array& a);

template<typename T>
std::map<std::string, T> ConvertObjectOfBools(const Object& obj)
{
    std::map<std::string, T> m;

    for (const auto& e : obj)
        m.insert({e.first, e.second.GetBool()});

    return m;
}

template<typename T>
std::map<std::string, T> ConvertObjectOfInts(const Object& obj)
{
    std::map<std::string, T> m;

    for (const auto& e : obj)
        m.insert({e.first, e.second.GetInt()});

    return m;
}

template<typename T>
std::map<std::string, T> ConvertObjectOfDoubles(const Object& obj)
{
    std::map<std::string, T> m;

    for (const auto& e : obj)
        m.insert({e.first, e.second.GetDouble()});

    return m;
}

template<typename T>
std::map<std::string, T> ConvertObjectOfNumbers(const Object& obj)
{
    std::map<std::string, T> m;

    for (const auto& e : obj)
        m.insert({e.first, e.second.GetNumber()});

    return m;
}

std::map<std::string, std::string> ConvertObjectOfStrings(const Object& obj);

} // End of namespace json.
