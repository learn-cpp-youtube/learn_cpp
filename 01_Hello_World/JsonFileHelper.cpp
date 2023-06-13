#include "JsonFileHelper.h"
#include <stdexcept>

using error = std::runtime_error;

const json::Value& GetValue(const json::Object& obj, const std::string& key)
{
    auto it = obj.find(key);
    if (it == obj.end())
        throw error("Could not find key \"" + key + "\" in json object.");
    return it->second;
}
