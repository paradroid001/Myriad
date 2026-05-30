#ifndef _MYRIAD_ISERIALISABLE_H_
#define _MYRIAD_ISERIALISABLE_H_

#include "core/config.h"

namespace Myriad
{
    class ISerialisable
    {
      public:
        virtual MyrError_t Serialise() = 0;
        virtual MyrError_t DeSerialise() = 0;
    };

    /*
    template <typename T>
    void to_json(json &j, const T &t)
    {
      static_cast<const ISerialisable<T> &>(t).to_json(j, t);
    }
    template <typename T>
    void from_json(const json &j, T &t)
    {
      (static_cast<ISerialisable<T> &>(t))->from_json(j, t);
    }
    */
} // namespace Myriad

#endif
