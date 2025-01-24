#include "core/MyrApplication.h"
// #include "core/MyrEngine.h"

namespace Myriad
{
  MyrApplication::MyrApplication() {}

  MyrApplication::~MyrApplication() {}

  /*
  // Client applications override this
  void MyrApplication::Run()
  {
    while (true)
      ;
  }


  void MyrApplication::InitEngine(MyrAppPreferences &prefs)
  {
      p_appdata_ = new MyrAppData(prefs);
      p_engine_ = new MyrEngine(prefs);
  }
  void MyrApplication::ShutdownEngine()
  {
      if (p_engine_ != nullptr)
      {
          delete p_engine_;
          p_engine_ = nullptr;
      }
      if (p_appdata_)
      {
          delete p_appdata_;
          p_appdata_ = nullptr;
      }
  }
  */
} // namespace Myriad
