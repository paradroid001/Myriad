#ifndef MYRIAD_CORE_MYRAPPLICATION_H
#define MYRIAD_CORE_MYRAPPLICATION_H

#include <cstddef>

#include "core.h"

namespace Myriad
{
    class MYR_API MyrApplication
    {
      public:
        MyrApplication();
        virtual ~MyrApplication();
        virtual void Run();
    };

    // Clients of the shared library use this to create their application.
    MyrApplication *CreateApplication();

    // This is what we might request of the app,
    // from a config file or similar
    class MYR_API MyrAppPreferences
    {
      public:
        Vector2 screen_dimensions;
        size_t target_fps;
    };

    // This is the live app state
    class MYR_API MyrAppData
    {
      private:
        // inlining the static var meant I didn't have to
        // declare it in a cpp file.
        // https://stackoverflow.com/questions/18860895/how-to-initialize-static-members-in-the-header
        inline static MyrAppData *_instance;
        Vector2 _screen_dimensions;
        size_t _target_fps;

        // Private Constructor
        MyrAppData(){};

      public:
        static MyrAppData *&Instance()
        {
            if (_instance == 0)
                _instance = new MyrAppData();
            return _instance;
        }

        void UpdateScreenDimensions(Vector2 &dimensions)
        {
            _screen_dimensions.x = dimensions.x;
            _screen_dimensions.y = dimensions.y;
        }

        const Vector2 GetScreenDimensions() const { return _screen_dimensions; }
    };

} // namespace Myriad

#endif
