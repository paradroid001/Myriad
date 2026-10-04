#include "io/Logging.h" //core logging
#include "myriad_types.h"

namespace Myriad
{
    Renderer::Renderer(std::unique_ptr<IRenderProvider> provider)
        : rendererprovider_(std::move(provider))
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

    void Renderer::DrawText(AssetID_t font_id, const std::string &text,
                            Vector2 pos, int size, Colour colour)
    {
        rendererprovider_->DrawText(font_id, text, pos, size, colour);
    }

} // namespace Myriad
