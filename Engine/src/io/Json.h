#ifndef __JSON_H_
#define __JSON_H_

#include "core/core.h"
#include "nlohmann/json.hpp"

namespace Myriad
{
    // using ext_json = nlohmann::json;
    typedef nlohmann::json json;
    class MYR_API Json
    {
      public:
        Json();
        virtual ~Json();
        json ReadJson(const char *filename);
    };
} // namespace Myriad

#endif
