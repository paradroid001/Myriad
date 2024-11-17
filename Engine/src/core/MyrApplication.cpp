#include "core/MyrApplication.h"
#include "core/MyrEngine.h"

namespace Myriad
{
    MyrApplication::MyrApplication() {}

    MyrApplication::~MyrApplication() {}

    // Client applications override this
    void MyrApplication::Run()
    {
        while (true)
            ;
    }

    void MyrApplication::InitEngine(MyrAppPreferences &prefs)
    {
        p_engine_ = new MyrEngine(prefs);
    }
    void MyrApplication::ShutdownEngine()
    {
        if (p_engine_ != nullptr)
        {
            delete p_engine_;
            p_engine_ = nullptr;
        }
    }
} // namespace Myriad
