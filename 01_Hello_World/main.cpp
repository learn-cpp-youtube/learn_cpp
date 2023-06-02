#include "Utilities/Json.h"

int main(int argc, char** argv)
{
    std::map<std::string, json::Value> v = {{"hello", json::Value{5}}, {"world", json::Value{"uh oh"}}};
    json::Value json = json::Object
    {
        {"", std::move(v)},
        {"test", "asgjg"},
        {"null", nullptr},
        {"booL", json::Array{false, true, 0, nullptr}},
        {"x", json::Array{3.1, 5, 6, "tet", json::Object{{"",""}}, json::Array{5,-1/3.0,1/3.0}}},
        {"empty", json::Object{}}
    };

    auto j1 = json::Output(json, false, 2);
    std::cout << json << std::endl;
    std::cout << std::endl;

    auto j2 = json::Parse(j1);
    json::Output(std::cout, j2, true, 2);
    std::cout << std::endl;

    return 0;
}
