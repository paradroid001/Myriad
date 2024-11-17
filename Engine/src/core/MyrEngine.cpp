#include "core/MyrEngine.h"
#include "core/core.h"
#include "core/event/MyrEvent.h"
#include "core/event/MyrEventService.h"
#include "core/memory/Allocator.h"
#include "core/thread/ThreadPool.h"
#include "rendering/Renderer.h"
#include "rendering/Window.h"

namespace Myriad
{
    MyrEngine::MyrEngine(MyrAppPreferences &prefs)
    {
        p_allocator_general_ = new Allocator(); // todo: params?
        p_allocator_general_->Init();
        p_asset_manager_ = new AssetManager();
        p_object_manager_ = new MyrObjectManager(); // todo: params?
        p_object_manager_->StartService();
        if (prefs.threaded)
        {
            // We only support one thread.
            assert(prefs.num_threads == 1);
            h_pool_ =
                p_allocator_general_->Alloc<ThreadPool>(prefs.num_threads);
            h_window_ =
                p_allocator_general_->Alloc<Window>(p_allocator_general_);
            h_renderer_ = p_allocator_general_->Alloc<Renderer>();
        }
        // Start an event service
        // p_event_service_ = &(MyrEventService::GetInstance());
        // Create (but not start) an event service.
        p_event_service_ = new MyrEventService();

        // Win32 api messes with my StartService.
#undef StartService
        p_event_service_->StartService();
        // So myrevents know where to send data...
        MyrEvent::SetEventService(p_event_service_);

        is_shutdown_ = false;
    }

    void MyrEngine::ShutdownEngine()
    {
        if (!is_shutdown_)
        {
            // stop the event service.
            p_event_service_->StopService();
            // then, stop all the threads.
            // will only have been inited if threaded.
            if (h_pool_.Get() != nullptr)
            {
                // h_pool_->Drain();
                //  now kill the pool.
                p_allocator_general_->Dealloc(h_pool_);
            }
            // pool is now dealloced?
            if (p_event_service_ != nullptr)
            {
                delete p_event_service_;
            }
            // This should release the objects.
            if (p_object_manager_ != nullptr)
            {
                p_object_manager_->StopService();
                delete p_object_manager_;
                p_object_manager_ = nullptr;
            }

            // This should release all assets
            if (p_asset_manager_ != nullptr)
            {
                delete p_asset_manager_;
                p_asset_manager_ = nullptr;
            }

            // shut down the renderer
            h_renderer_->Shutdown();
            p_allocator_general_->Dealloc(h_renderer_);
            // shut down the window
            h_window_->Shutdown();
            p_allocator_general_->Dealloc(h_window_);

            // This needs to be deleted last.
            if (p_allocator_general_ != nullptr)
            {
                p_allocator_general_->Shutdown();
                delete p_allocator_general_;
                p_allocator_general_ = nullptr;
            }
            is_shutdown_ = true;
        }
    }
    MyrEngine::~MyrEngine() { ShutdownEngine(); }
} // namespace Myriad
