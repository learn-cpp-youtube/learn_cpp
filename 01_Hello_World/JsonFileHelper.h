#pragma once

#include "Utilities/Json.h"
#include <string>

const json::Value& GetValue(const json::Object& obj, const std::string& key);