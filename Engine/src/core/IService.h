#ifndef MYRIAD_CORE_ISERVICE_H
#define MYRIAD_CORE_ISERVICE_H

// #include "core/memory/MyrHandle.h"
#include "core/MyrSingleton.h"

namespace Myriad
{
    class IService
    {
        /*
        private:
          static IService *_hinstance;
          IService()
          {
              if (_hinstance == 0)
              {
                  // This should be dealloced when the last one goes out of
        scope.
                  // TODO: Test this.
                  _hinstance = this;
              }
          } // private constructor
        */
      protected:
        bool started_ = false;
        virtual bool WhenStartService() = 0;
        virtual bool WhenStopService() = 0;

      public:
        virtual ~IService() {}
        // static IService *Instance() { return _hinstance; }
        bool StartService()
        {
            if (!started_)
            {
                bool ret = WhenStartService();
                started_ = true;
                return ret;
            }
            return true;
        }
        bool StopService()
        {
            if (started_)
            {
                bool ret = WhenStopService();
                started_ = false;
                return ret;
            }
            return true;
        }
    };

    // MyrHandle<IService> *IService::_hinstance;

    /*
    IService::~IService()
    {
        // nothing
    }*/

} // namespace Myriad
#endif
