#include "myriad.h"
#include <memory> //shared_ptr

#include "io/BasicLogger.h"

namespace Myriad
{
    // Definition of the global loggers.
    std::shared_ptr<ILogger> global_logger_core;
    std::shared_ptr<ILogger> global_logger_client;

    Application::~Application() { MYR_CORE_TRACE("Application destructor"); }
    Application::Application()
    {
        // TODO: Engine config could select logger
        global_logger_core = std::make_shared<BasicLogger>("Core");
        global_logger_client = std::make_shared<BasicLogger>("Client");

        MYR_CORE_TRACE("Application constructor");

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
    void Application::Run()
    {
        // Default implementation does nothing
    }
} // namespace Myriad
