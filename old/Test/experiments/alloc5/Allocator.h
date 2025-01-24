#ifndef ALLOC5_ALLOCATOR_H
#define ALLOC5_ALLOCATOR_H

#include <iostream>
#include <cassert>

template <typename T>
struct MemRecord
{
  T *ptr;
  uint32_t count;

  MemRecord() : ptr(nullptr), count(0) { std::cout << "Alloc" << std::endl; }
  // MemRecord(T *p) : ptr(p), count(1) {}
  ~MemRecord()
  {
    if (ptr != nullptr)
      delete ptr;
    std::cout << "Deleted MemRecord " << " (" << ptr << ")" << " [" << this << "]" << std::endl;
    ptr = nullptr;
    count = 0;
  }
  void Reset(T *p)
  {
    assert(count == 0);
    assert(ptr == nullptr);
    ptr = p;
    count = 1;
    std::cout << "Assigned MemRecord" << " (" << ptr << ")" << " [" << this << "]" << std::endl;
  }
  T *Get()
  {
    return ptr;
  }
  T *Hold()
  {
    count += 1;
    return ptr;
  }
  uint32_t Release()
  {
    return --count;
  }
};

typedef uint32_t Handle_T;

template <typename T>
class Allocator
{
protected:
  MemRecord<T> *records_;
  uint32_t max_records_;            // this actually shouldn't be bigger than 2^24
  uint32_t high_watermark_ = 0;     // has never before been allocated
  uint32_t low_watermark_ = 0;      // the lowest recycle index
  uint32_t last_recycle_index_ = 0; // the last (most recent) index that was freed
  uint32_t allocs = 0;
  uint32_t frees = 0;

  Handle_T assign_to_first_free_slot(T *p)
  {
    Handle_T retval = 0;
    // We want the lowest free index. That's the low watermark.
    records_[low_watermark_].Reset(p); // = MemRecord<T>(pobj); // count of 1
    retval = low_watermark_;

    // Do Accounting.
    // First, increase allocs
    allocs += 1;
    // Then, we want to increment the low watermark.
    // if the low is less than the high, then inc to the next free spot.
    while (low_watermark_ <= high_watermark_ && records_[low_watermark_].count != 0)
    {
      low_watermark_ += 1;
    }
    // Now if low started the same as high, it will have ended up 1 higher.
    if (low_watermark_ > high_watermark_)
      high_watermark_ = low_watermark_;
    return retval;
  }

public:
  Allocator(uint32_t max_records) : max_records_(max_records)
  {
    uint32_t max = 1 << 24;
    std::cout << "Max records must be less than " << max << std::endl;
    assert(max_records < max);
    records_ = new MemRecord<T>[max_records];
  }
  ~Allocator()
  {
    delete[] records_;
  }

  void PrintStats()
  {
    std::cout << "Allocs: " << allocs << std::endl;
    std::cout << "Frees: " << frees << std::endl;
    std::cout << "Low Watermark: " << low_watermark_ << std::endl;
    std::cout << "High Watermark: " << high_watermark_ << std::endl;
    std::cout << "Last Recycle: " << last_recycle_index_ << std::endl;
    std::cout << "Utilisation: " << allocs - frees << "/" << max_records_ << " (" << (allocs - frees) * 100.0f / max_records_ << "%)" << std::endl;
  }

  template <typename... Args>
  Handle_T Alloc(Args... args)
  {
    // TODO there is no checking here about max records.
    T *pobj = new T(args...);
    return assign_to_first_free_slot(pobj);
  }

  template <typename U, typename... Args>
  Handle_T Alloc(Args... args)
  {
    U *pobj = new U(args...);
    return assign_to_first_free_slot(static_cast<T *>(pobj));
  }

  T *Get(Handle_T handle)
  {
    // This could return nulls, if the handle has been freed
    if (handle < max_records_)
      return records_[handle].Get();
    return nullptr;
  }

  // Free a handle, making it usable by someone else.
  void Free(Handle_T handle)
  {
    if (handle < max_records_)
    {
      records_[handle].~MemRecord<T>(); // call dtor

      // now do some accounting.
      frees += 1;
      if (handle < low_watermark_)
        low_watermark_ = handle;
      last_recycle_index_ = handle;
    }
  }
};

#endif
