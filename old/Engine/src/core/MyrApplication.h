#ifndef MYRIAD_CORE_MYRAPPLICATION_H
#define MYRIAD_CORE_MYRAPPLICATION_H

// #include <cstddef>
#include "core/MyrEngine.h"
#include "core/core.h"

namespace Myriad
{
    class MYR_API MyrApplication
    {
      protected:
        //inline prevents having to declare outside header.
        inline static MyrEngine *p_engine_;
        inline static MyrAppPreferences prefs_;
        inline static MyrAppData *p_appdata_;

      public:
        MyrApplication();
        virtual ~MyrApplication();
        virtual void Run();
        virtual void InitEngine(MyrAppPreferences &prefs);
        virtual void ShutdownEngine();
        inline static MyrEngine *GetEngine() { return p_engine_; }
        inline static MyrAppPreferences &GetPrefs() { return prefs_; }
        inline static MyrAppData *GetAppData() { return p_appdata_; }
    };

    // Clients of the shared library use this to create their application.
    MyrApplication *CreateApplication();

} // namespace Myriad

#endif
