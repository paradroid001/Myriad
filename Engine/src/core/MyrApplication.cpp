#include "myriad.h"

#include <memory>

#include "io/BasicLogger.h"

namespace Myriad
{
    // Definition of the global loggers.
    std::shared_ptr<ILogger> global_logger_core;
    std::shared_ptr<ILogger> global_logger_client;

    MyrApplication::~MyrApplication()
    {
        MYR_CORE_TRACE("MyrApplication destructor");
    }

    MyrApplication::MyrApplication()
    {
        // TODO: Engine config could select logger
        global_logger_core = std::make_shared<BasicLogger>("Core");
        global_logger_client = std::make_shared<BasicLogger>("Client");

        MYR_CORE_TRACE("MyrApplication constructor");

        MYR_CORE_TRACE("Trace test.");
        MYR_CORE_INFO("Info test.");
        MYR_CORE_WARN("Warning test.");
        MYR_CORE_ERROR("Error test.");
        MYR_CORE_CRITICAL("Critical test.");

        MYR_TRACE("Trace test.");
        MYR_INFO("Info test.");
        MYR_WARN("Warning test.");
        MYR_ERROR("Error test.");
        MYR_CRITICAL("Critical test.");
    }
    void MyrApplication::Run()
    {
        // Default implementation does nothing
    }
} // namespace Myriad
