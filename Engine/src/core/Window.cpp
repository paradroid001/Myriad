#include "io/Logging.h"
#include "myriad_types.h"

namespace Myriad
{

    Window::Window(std::unique_ptr<IWindowProvider> provider)
        : windowprovider_(std::move(provider))
    {
    }

    Window::~Window()
    {
        windowprovider_->Shutdown();
        windowprovider_.reset();
        MYR_CORE_TRACE("Window destroyed");
    }

    WindowState_t Window::GetState() { return windowprovider_->GetState(); }

    bool Window::Open(const WindowConfig &config)
    {
        return windowprovider_->Open(config);
    }

    void Window::Close() { windowprovider_->Close(); }

} // namespace Myriad
