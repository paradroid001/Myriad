#include "io/Json.h"
#include <fstream>

namespace Myriad
{
    Json::Json() {}
    Json::~Json() {}
    json Json::ReadJson(const char *filename)
    {
        std::ifstream f(filename);
        json data = json::parse(f);
        return data;
    }
} // namespace Myriad