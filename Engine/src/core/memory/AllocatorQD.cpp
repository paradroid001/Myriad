#include "core/memory/AllocatorQD.h"
#include "core/MyrHandle.h"

bool Myriad::AllocatorQD::Init()
{
    assert(_allocs_vector.size() == 0);
    return true;
}
bool Myriad::AllocatorQD::Shutdown()
{
    // delete all the handles.
    for (const auto &handle : allocs_vector_)
    {
        // MYR_CORE_TRACE("Allocator {0:x} is deleting [{1:x}]={2:x}]",
        //                (size_t)this, (size_t)key, (size_t)value);

        // Ok so this allocator is a bit of a mess.

        // We can only delete because the handles have the BaseHandle
        // type. At the moment, handles still have a ref count of 1
        // because of the fact that they are assigned here in this map.
        // We manually call their destructor, and they get decremented
        // by one.
        delete handle;
    }

    allocs_vector_.clear();

    // So handles have one more ref than they should because this
    // allocator holds them all, but even if they didn't, when they self
    // destruct the allocator will never know, unless the handles all
    // know about the allocator.
    MYR_CORE_WARN("Fix the allocator, it will never know when handles "
                  "are deleted.");
    return true;
}

template <class T, typename... Args>
uint16_t Myriad::AllocatorQD::Alloc(Args... args)
{
    MYR_CORE_TRACE("Allocator {0:x} is creating a new handle.", (size_t)this);
    MyrHandle<T> *p =
        new MyrHandle<T>(allocs_vector_.size() + 1, new T(args...));
    // T *p = new T(args...)); //TODO, smart pointer

    // TODO: thread safe?
    // TODO this allocator actuall has a huge problem.
    //    something screwy is going on with the ref counts.
    //    but also, we aren't retaining the MyrHandle, just the
    //    raw pointer. That's...probably not what we want,
    //    unless the pointer cleans it self up, which it currently
    //    does do in its destructor, in which case there's no
    //    way for the allocator to know that it was deleted.
    //    but ALSO, we then delete the raw pointer, but the MyrHandle
    //    itself is never actually deleted because of the ref-count
    //    problem.
    MYR_CORE_TRACE("Inserting into allocs vector");
    _allocs_vector.push_back(p);
    MYR_CORE_TRACE("Returning handle index"); // we actually return index +1
    return _allocs_vector.size(); // so a handle with index 0 is invalid.
}

template <class T> T *Myriad::AllocatorQD::Get(uint16_t index)
{
    // 0 is an invalid index
    if (index > 0 && index <= allocs_vector_.size())
    {
        // return (static_cast<T *>(_allocs_vector[index + 1]));
        return allocs_vector_[index + 1].Get();
    }

    return NULL;
}
