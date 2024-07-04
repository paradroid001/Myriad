#ifndef MYRIAD_BACKENDS_SPDLOGLOGBACKEND_H
#define MYRIAD_BACKENDS_SPDLOGLOGBACKEND_H

#include "io/Log.h" //for the logging interface
#include "spdlog/spdlog.h"
#include <memory>

namespace Myriad
{
    class MYR_API SpdlogLoggingBackend : public Myriad::Log
    {

      public:
      private:
        static std::shared_ptr<spdlog::logger> s_SpdLogCoreLogger;
        static std::shared_ptr<spdlog::logger> s_SpdLogClientLogger;
    };
} // namespace Myriad
#endif
