#ifndef MYRIAD_CORE_MYRENGINE_H
#define MYRIAD_CORE_MYRENGINE_H

#include "asset/AssetManager.h"
#include "core/event/MyrEventService.h"
#include "core/memory/Allocator.h"
#include "core/memory/MyrHandle.h"
#include "core/object/MyrObjectManager.h"
#include "core/thread/ThreadPool.h"
#include "rendering/Renderer.h"
#include "rendering/Window.h"

#include <cstdint>

namespace Myriad
{
    // This is what we might request of the app,
    // from a config file or similar
    struct MYR_API MyrAppPreferences
    {
      public:
        Vector2 screen_dimensions;
        size_t target_fps;
        bool threaded;
        uint8_t num_threads; // 0 = auto
    };

    // This is the live app state
    class MYR_API MyrAppData
    {
      private:
        Vector2 _screen_dimensions;
        // TODO: add screen orientation
        size_t _target_fps;
        // TODO: add

      public:
        MyrAppData(MyrAppPreferences &prefs) {};
        void UpdateScreenDimensions(Vector2 &dimensions)
        {
            _screen_dimensions.x = dimensions.x;
            _screen_dimensions.y = dimensions.y;
        }

        void UpdateTargetFPS(size_t fps)
        {
            _target_fps = fps; // todo: this is going to be more complicated.
        }

        const Vector2 GetScreenDimensions() const { return _screen_dimensions; }
    };

    class MYR_API MyrEngine
    {
      private:
        // General allocator
        Allocator *p_allocator_general_;
        // Asset Manager
        AssetManager *p_asset_manager_;
        // GameObject Manager (has own allocator)
        MyrObjectManager *p_object_manager_;
        // Component Manager (has own allocator)
        //???
        MyrHandle<ThreadPool> h_pool_;
        MyrHandle<Renderer> h_renderer_;
        MyrHandle<Window> h_window_;
        MyrEventService *p_event_service_;

        // Because the pointer to this class is static,
        // it may be cleaned up via destructor at an unpredictable time,
        // possibly later than we would like.
        // So we have an explicit shutdown method.
        // The is_shutdown guard helps us only shutdown if needed.
        void ShutdownEngine();
        bool is_shutdown_;

      public:
        MyrEngine(MyrAppPreferences &prefs);
        inline Allocator *GetAllocatorGeneral() { return p_allocator_general_; }
        inline AssetManager *GetAssetManager() { return p_asset_manager_; }
        inline MyrObjectManager *GetObjectManager()
        {
            return p_object_manager_;
        }
        inline MyrHandle<ThreadPool> GetThreadPoolHandle() { return h_pool_; }
        inline MyrHandle<Window> GetWindowHandle() { return h_window_; }
        inline MyrHandle<Renderer> GetRendererHandle() { return h_renderer_; }
        inline MyrEventService *GetEventService() { return p_event_service_; }

        ~MyrEngine();
    };
} // namespace Myriad
#endif
