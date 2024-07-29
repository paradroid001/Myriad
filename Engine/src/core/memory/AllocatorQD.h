#ifndef MYRIAD_CORE_MEMORY_ALLOCATORQD_H
#define MYRIAD_CORE_MEMORY_ALLOCATORQD_H

#include "core/memory/AllocatorProvider.h"
#include "io/Log.h"
#include <cassert>
#include <map>

/*
  A quick and dirty allocator.
  I'm not worried about performance,
  just pointer safety.
*/

namespace Myriad
{
    class AllocatorQD : public AllocatorProvider
    {
      private:
        std::vector<HandleBase *> allocs_vector_;

      public:
        virtual bool Init() override;
        virtual bool Shutdown() override;
        template <class T, typename... Args> uint16_t Alloc(Args... args);
        template <class T> T *Get(uint16_t index);
    };
} // namespace Myriad

#endif
