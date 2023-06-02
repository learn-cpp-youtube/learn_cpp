#include "Json.h"
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace json
{

Value::Value(const Value& v)
{
    if (v.IsNull())
        data = v.GetNull();
    else
    if (v.IsBool())
        data = v.GetBool();
    else
    if (v.IsInteger())
        data = v.GetInteger();
    else
    if (v.IsDouble())
        data = v.GetDouble();
    else
    if (v.IsString())
        data = v.GetString();
    else
    if (v.IsArray())
        data = std::make_unique<Array>(v.GetArray());
    else
    if (v.IsObject())
        data = std::make_unique<Object>(v.GetObject());
    else
        throw std::runtime_error("Unexpected json::Value.");
}

Value& Value::operator=(const Value& v)
{
    if (this == &v)
        return *this;

    if (v.IsNull())
        data = v.GetNull();
    else
    if (v.IsBool())
        data = v.GetBool();
    else
    if (v.IsInteger())
        data = v.GetInteger();
    else
    if (v.IsDouble())
        data = v.GetDouble();
    else
    if (v.IsString())
        data = v.GetString();
    else
    if (v.IsArray())
        data = std::make_unique<Array>(v.GetArray());
    else
    if (v.IsObject())
        data = std::make_unique<Object>(v.GetObject());
    else
        throw std::runtime_error("Unexpected json::Value.");

    return *this;
}
    
Value& Value::operator=(Value&& v) noexcept
{
    if (this != &v)
        data = std::move(v.data);

    return *this;
}

class Parser
{
public:
    explicit Parser(std::istream& in) : in{&in}, c{0}, lineNum{1}, charNum{0} { ReadChar(); }
    Value Parse();
    
private:
    std::runtime_error ParsingError(const std::string& error);
    void   ReadChar();
    void   RemoveWhitespace() { while(c==' ' || c=='\t' || c=='\n' || c=='\r') ReadChar(); }
    Value  ReadElement();
    Value  ReadValue();
    void   ReadLiteral(const char* text);
    std::nullptr_t ReadNull() { ReadLiteral("null"); return nullptr; }
    bool   ReadFalse() { ReadLiteral("false"); return false; }
    bool   ReadTrue()  { ReadLiteral("true"); return true; }
    std::string ReadString();
    Array  ReadArray();
    Object ReadObject();
    Value  ReadNumber();
    void   ReadDigits(std::string& numberStr);

    std::istream* in;
    char c;      // The last character read from 'in' (but not yet interpreted).
    int lineNum; // Line number for 'c'.
    int charNum; // Character position for 'c'.
};

Value Parser::Parse()
{
    Value v = ReadElement();
    
    if (c != '\0')
        throw ParsingError("Expected no more characters.");

    return v;
}

std::runtime_error Parser::ParsingError(const std::string& error)
{
    return std::runtime_error{"Json parsing error on line " + std::to_string(lineNum)
        + " character " + std::to_string(charNum) + " : " + error};
}

//This function is used in parsing an input file. It reads one character from the file. If
//the end of the file is reached c is set to '\0'.
void Parser::ReadChar()
{
    in->get(c);
    
    if (in->eof())
    {
        c = '\0';
        return;
    }

    if (c == '\n')
    {
        ++lineNum;
        charNum = 0;
    }
    else
        ++charNum;

    return;
}

Value Parser::ReadElement()
{
    RemoveWhitespace();
    Value v = ReadValue();
    RemoveWhitespace();
    return v;
}

Value Parser::ReadValue()
{
    if (c == 'n')
        return ReadNull();

    if (c == 'f')
        return ReadFalse();

    if (c == 't')
        return ReadTrue();

    if (c == '\"')
        return ReadString();

    if (c == '[')
        return ReadArray();

    if (c == '{')
        return ReadObject();

    if (c == '-' || ('0' <= c && c <= '9'))
        return ReadNumber();

    throw ParsingError("Unexpected character when trying to parse value.");
}

void Parser::ReadLiteral(const char* text)
{
    std::size_t index = 0;
    while (text[index])
    {
        if (text[index] != c)
        {
            throw ParsingError("Expected character '" + std::to_string(text[index]) 
                               + "' when parsing " + std::string(text) + ".");
        }

        ReadChar();
        ++index;
    }

    return;
}

std::string Parser::ReadString()
{
    if(c != '\"')
        throw ParsingError("Expected '\"' when parsing string.");
    ReadChar();

    std::string out;
    while (c != '\"')
    {
        if (c == '\0')
            throw ParsingError("Unexpected character when parsing string.");

        out.push_back(c);

        if (c != '\\')
        {
            ReadChar();
            continue;
        }

        // Start of an escape sequence.
        ReadChar();

        if (c == '\"'
         || c == '\\'
         || c == '/'
         || c == 'b'
         || c == 'f'
         || c == 'n'
         || c == 'r'
         || c == 't')
        {
            out.push_back(c);
            ReadChar();
            continue;
        }

        if (c != 'u')
            throw ParsingError("Unexpected character when parsing escape sequence in string.");
            
        // Escape sequence is \u hex hex hex hex
        out.push_back(c);
        ReadChar();

        for (int i=0; i<4; ++i)
        {
            if (('0' <= c && c <= '9')
             || ('a' <= c && c <= 'f')
             || ('A' <= c && c <= 'F'))
            {
                out.push_back(c);
                ReadChar();
                continue;
            }

            throw ParsingError("Unexpected character when parsing escape sequence in string.");
        }
    }
    
    ReadChar();
    return out;
}

Array Parser::ReadArray()
{
    if (c != '[')
        throw ParsingError("Expected '[' when parsing array.");
    ReadChar();

    RemoveWhitespace();

    Array out;
    if (c != ']')
    {
        while (true)
        {
            out.push_back(ReadElement());

            if (c == ']')
                break;

            if (c != ',')
                throw ParsingError("Expected ',' when parsing array.");

            ReadChar();
        }
    }
    
    ReadChar();
    return out;
}

Object Parser::ReadObject()
{
    if (c != '{')
        throw ParsingError("Expected '{' when parsing object.");
    ReadChar();

    RemoveWhitespace();

    Object out;
    if (c != '}')
    {
        while (true)
        {
            // Read key-value pair.
            RemoveWhitespace();
            std::string key = ReadString();
            RemoveWhitespace();

            if (c != ':')
                throw ParsingError("Expected ':' when parsing object.");
            ReadChar();

            Value value = ReadElement();

            out.insert({key, value});

            if (c == '}')
                break;

            if (c != ',')
                throw ParsingError("Expected ',' when parsing object.");

            ReadChar();
        }
    }
    
    ReadChar();
    return out;
}

Value Parser::ReadNumber()
{
    bool isFloat = false;
    std::string numberStr;

    if (c == '-')
    {
        numberStr.push_back(c);
        ReadChar();
    }

    // Read integer part.
    if (c == '0')
    {
        numberStr.push_back(c);
        ReadChar();
    }
    else
        ReadDigits(numberStr);

    // Read fraction part.
    if (c == '.')
    {
        isFloat = true;

        numberStr.push_back(c);
        ReadChar();
        ReadDigits(numberStr);
    }

    // Read exponent part.
    if (c == 'e' || c == 'E')
    {
        isFloat = true;

        numberStr.push_back(c);
        ReadChar();

        if (c == '-' || c == '+')
        {
            numberStr.push_back(c);
            ReadChar();
        }

        ReadDigits(numberStr);
    }

    // Convert numberStr to a double or int64_t.
    if (isFloat)
        return std::stod(numberStr);

    return std::stoll(numberStr);
}

void Parser::ReadDigits(std::string& numberStr)
{
    if (c < '0' || '9' < c)
        throw ParsingError("Expected a digit when parsing number.");

    while ('0' <= c && c <= '9')
    {
        numberStr.push_back(c);
        ReadChar();
    }

    return;
}

Value Parse(std::istream& in)
{
    Parser parser{in};
    return parser.Parse();
}

Value Parse(std::string in)
{
    std::istringstream ss{std::move(in)};
    return Parse(ss);
}

Value ParseFile(const std::string& filename)
{
    std::ifstream in;

    in.open(filename.c_str());
    if (!in.good())
        throw std::runtime_error("Could not open \"" + filename + "\" for reading.");

    return Parse(in);
}

template<typename T>
void Output(std::ostream& out,
            const T& t,
            bool pretty = true,
            int tabWidth = 4,
            int currentIndent = 0)
{
    if constexpr (std::is_same_v<T, Value>)
    {
        if (t.IsNull())
            Output(out, t.GetNull(), pretty, tabWidth, currentIndent);
        else
        if (t.IsBool())
            Output(out, t.GetBool(), pretty, tabWidth, currentIndent);
        else
        if (t.IsInteger())
            Output(out, t.GetInteger(), pretty, tabWidth, currentIndent);
        else
        if (t.IsDouble())
            Output(out, t.GetDouble(), pretty, tabWidth, currentIndent);
        else
        if (t.IsString())
            Output(out, t.GetString(), pretty, tabWidth, currentIndent);
        else
        if (t.IsArray())
            Output(out, t.GetArray(), pretty, tabWidth, currentIndent);
        else
        if (t.IsObject())
            Output(out, t.GetObject(), pretty, tabWidth, currentIndent);
        else
            throw std::runtime_error("Unexpected json::Value.");

        return;
    }

    if (!pretty)
        tabWidth = 0;
    std::string indent(tabWidth*currentIndent, ' ');

    if constexpr (std::is_same_v<T, std::nullptr_t>)
    {
        out << indent << "null";
        return;
    }
    else
    if constexpr (std::is_same_v<T, bool>)
    {
        out << indent << (t ? "true" : "false");
        return;
    }
    else
    if constexpr (std::is_same_v<T, std::int64_t>)
    {
        std::stringstream ss;
        ss << t;
        out << indent << ss.str();
        return;
    }
    else
    if constexpr (std::is_same_v<T, double>)
    {
        std::stringstream ss;
        ss << std::scientific << std::setprecision(std::numeric_limits<double>::max_digits10);
        ss << t;
        out << indent << ss.str();
        return;
    }
    else
    if constexpr (std::is_same_v<T, std::string>)
    {
        out << indent << "\"" << t << "\"";
        return;
    }
    else
    if constexpr (std::is_same_v<T, Array>)
    {
        out << indent << "[";
        if (pretty)
            out << "\n";
        
        for (std::size_t i=0; i<t.size(); ++i)
        {
            Output(out, t[i], pretty, tabWidth, currentIndent+1);
            
            if (i+1 < t.size())
                out << ",";
            
            if (pretty)
                out << "\n";
        }
        
        out << indent << "]";
        return;
    }
    else
    if constexpr (std::is_same_v<T, Object>)
    {
        out << indent << "{";
        if (pretty)
            out << "\n";

        auto it = t.cbegin();
        while (it != t.cend())
        {
            Output(out, it->first, pretty, tabWidth, currentIndent+1);
            
            if (pretty)
                out << " ";

            out << ":";

            if (it->second.IsArray() || it->second.IsObject())
            {
                if (pretty)
                    out << "\n";

                Output(out, it->second, pretty, tabWidth, currentIndent+2);
            }
            else
            {
                if (pretty)
                    out << " ";

                Output(out, it->second, pretty, tabWidth, 0);
            }

            ++it;

            if (it != t.cend())
                out << ",";
            
            if (pretty)
                out << "\n";
        }
        
        out << indent << "}";
        return;
    }
    else
        throw std::runtime_error("Unexpected type.");
}

std::string Output(const Value& v, bool pretty, int tabWidth)
{
    std::stringstream ss;
    Output(ss, v, pretty, tabWidth, 0);
    return ss.str();
}

void Output(std::ostream& out, const Value& v, bool pretty, int tabWidth)
{
    Output(out, v, pretty, tabWidth, 0);
    return;
}

void SaveToFile(const std::string& filename, const Value& v, bool pretty, int tabWidth)
{
    std::ofstream out;

    out.open(filename.c_str(), std::ios::trunc);
    if (!out.good())
        throw std::runtime_error("Could not open \"" + filename + "\" for writing.");

    Output(out, v, pretty, tabWidth);
    return;
}

std::ostream& operator<<(std::ostream& out, const Value& v)
{
    Output(out, v, false);
    return out;
}

std::vector<std::string> ConvertArrayOfStrings(const Array& a)
{
    std::vector<std::string> v;

    v.reserve(a.size());
    for (const auto& e : a)
        v.push_back(e.GetString());

    return v;
}

std::map<std::string, std::string> ConvertObjectOfStrings(const Object& obj)
{
    std::map<std::string, std::string> m;

    for (const auto& e : obj)
        m.insert({e.first, e.second.GetString()});

    return m;
}

} // End of namespace json.
