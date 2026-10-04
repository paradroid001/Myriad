#include "io/Logging.h" //core logging
#include "myriad_types.h"

namespace Myriad
{
    Renderer::Renderer(std::shared_ptr<IRenderProvider> provider)
        : rendererprovider_(provider)
    {
        MYR_CORE_TRACE("Renderer initialized with provider.");
    }

    Renderer::~Renderer()
    {
        rendererprovider_->Shutdown();
        rendererprovider_.reset();
        MYR_CORE_TRACE("Renderer destroyed.");
    }

    void Renderer::BeginFrame()
    {
        // Implementation for beginning a frame
        rendererprovider_->BeginFrame();
    }

    void Renderer::EndFrame()
    {
        // Implementation for ending a frame
        rendererprovider_->EndFrame();
    }
} // namespace Myriad
