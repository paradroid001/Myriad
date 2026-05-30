#ifndef _MYRIAD_CORE_ISERIALISER_H_
#define _MYRIAD_CORE_ISERIALISER_H_

#include "core/config.h"

// The invalid serialiser id.
#define MYR_SERIALISER_ID_INVALID 0

/*
A base class for serialisers
*/
namespace Myriad
{
    // fwd declare
    class ISerialisable;

    // A type for IDs of serialised objects
    typedef uint32_t MyrSerialiserID_t;

    class ISerialiser
    {
      public:
        virtual MyrSerialiserID_t SerialiseObject(ISerialisable *object) = 0;
        virtual MyrError_t DeserialiseObject(ISerialisable *object) = 0;
    };
} // namespace Myriad
#endif
