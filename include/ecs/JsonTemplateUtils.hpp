#pragma once

#include "Json/JsonParser.hpp"

template<class... Ts>
struct Overloads : Ts... { using Ts::operator()...; };


void replace_json_object(JsonObject &obj, JsonObject const &params);
void replace_json_array(JsonArray &arr, JsonObject const &params);
