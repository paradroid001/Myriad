#ifndef _MYRIAD_CORETYPES_H_
#define _MYRIAD_CORETYPES_H_

#include <memory> //shared_ptr
#include <string>

/* MyrIDArray*/
#include <queue>
#include <unordered_map>
/* MyrIDArray*/

#include "EngineConfig.h"

namespace Myriad
{
    /* Basic Types */
    /**
     * A class for representation of colours as
     * four 8 bit integer values, or 32 bit RGBA
     */
    class MyrColour
    {
      public:
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };

    class Vector2
    {
      public:
        float x;
        float y;
        Vector2(float x, float y) : x(x), y(y) {}
        Vector2() : Vector2(0.0f, 0.0f) {};
    };

    class Vector2i
    {
      public:
        int32_t x;
        int32_t y;
        Vector2i(int32_t x, int32_t y) : x(x), y(y) {}
        Vector2i() : Vector2i(0, 0) {};
    };

    template <typename V, typename D> class Rect
    {
      public:
        V pos;
        V size;
        V anchor;
        Rect() : pos(), size(), anchor() {}
        Rect(D sizex, D sizey) : pos(), size(sizex, sizey), anchor() {}
        Rect(V pos, V size) : pos(pos), size(size), anchor() {}
        Rect(V pos, V size, V anchor) : pos(pos), size(size), anchor(anchor) {}
        inline D Top() { return pos.y - anchor.y; }
        inline D Bottom() { return pos.y - anchor.y + size.y; }
        inline D Left() { return pos.x - anchor.x; }
        inline D Right() { return pos.x - anchor.x + size.x; }
    };

    class Rect2D : public Rect<Vector2, float>
    {
      public:
        Rect2D() : Rect() {}
        Rect2D(Vector2 pos, Vector2 size) : Rect(pos, size) {}
        Rect2D(Vector2 pos, Vector2 size, Vector2 anchor)
            : Rect(pos, size, anchor)
        {
        }
    };

    /* A sequence/iterable class which is:
       - Contiguous in memory (for cache friendliness)
       - Has a fixed max size (no dynamic resizing)
       - Fast to iterate, reasonable lookup by id

       - Will maintain an id to index mapping, so that we can have fast lookups
       by id.
       - Will maintain a free list of available slots, and reuse them when
       adding new items.
       - Will maintain a skiplist of occupied slots, so that we can iterate over
       only occupied slots.

       - We will use a freelist to track free slots.
       - We will use skiplists to optimise the iterator.
    */
    template <typename T, std::size_t n> class MyrIDArray
    {
      private:
        static_assert(n > 0, "MyrIDArray requires n > 0");
        static constexpr std::size_t InvalidID = MYRIAD_INVALID_ID;

        T array[n];
        bool occupied[n];     // track which slots are occupied
        std::size_t capacity; // max number of items that can be stored
        std::size_t size;     // current number of occupied slots
        std::size_t
            min_used_index; // track the minimum index that has ever been used
        std::size_t
            max_used_index; // track the maximum index that has ever been used
        std::queue<std::size_t> free_slots; // queue of free slot indices
        MYR_ID_t max_used_id;
        std::unordered_map<MYR_ID_t, std::size_t>
            id_to_index; // map from ID to index in array

        // Pass in the index you just used.
        // All the internal accounting is then done.
        void UpdateOccupancy(size_t index)
        {
            occupied[index] = true;
            if (index > max_used_index)
            {
                max_used_index = index;
            }
            if (index < min_used_index)
            {
                min_used_index = index;
            }
            // std::cout << "Occupying " << index << ": min=" << min_used_index
            //           << ", max=" << max_used_index << std::endl;
        }

        size_t GetNextFreeIndex()
        {
            size_t next_index = InvalidID;
            // No new space for items
            if (size >= capacity)
                return InvalidID;

            if (free_slots.empty())
            {
                if (size == 0)
                {
                    next_index = 0;
                }
                else
                {
                    next_index = max_used_index + 1;
                }
            }
            else
            {
                next_index = free_slots.front();
                free_slots.pop();
            }

            return next_index;
        }

        /*
        void InitializeFreeList()
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                occupied[i] = false;
            }
            max_used_index = 0;
            size = 0;
            capacity = n;
            free_slots.empty();
            id_to_index.clear();
        }

        std::size_t DequeueFreeSlot()
        {
            if (free_head == InvalidID)
                return InvalidID;

            std::size_t id = free_head;
            free_head = free_next[id];
            if (free_head == InvalidID)
                free_tail = InvalidID;
            free_next[id] = InvalidID;
            return id;
        }

        void EnqueueFreeSlot(std::size_t id)
        {
            // Reuse recently freed slots first, so the same ID can be
            reused
                // as soon as it becomes available.
                free_next[id] = free_head;
            free_head = id;
            if (free_tail == InvalidID)
            {
                free_tail = id;
            }
        }

        void LinkOccupiedAtTail(std::size_t id)
        {
            occupied_prev[id] = occupied_tail;
            occupied_next[id] = InvalidID;
            if (occupied_tail != InvalidID)
            {
                occupied_next[occupied_tail] = id;
            }
            else
            {
                occupied_head = id;
            }
            occupied_tail = id;
        }

        void UnlinkOccupied(std::size_t id)
        {
            std::size_t prev = occupied_prev[id];
            std::size_t next = occupied_next[id];

            if (prev != InvalidID)
                occupied_next[prev] = next;
            else
                occupied_head = next;

            if (next != InvalidID)
                occupied_prev[next] = prev;
            else
                occupied_tail = prev;

            occupied_prev[id] = InvalidID;
            occupied_next[id] = InvalidID;
        }
      */
        // These three functions are really just used by
        // the iterator.
        bool Occupied(size_t index) { return occupied[index]; }
        MYR_ID_t MaxID() { return max_used_id; }
        size_t MaxIndex() { return max_used_index; }

      public:
        class Iterator
        {
          public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T *;
            using reference = T &;

            Iterator(MyrIDArray *array, std::size_t index)
                : array(array), index(index)
            {
            }

            reference operator*() const { return array->array[index]; }
            pointer operator->() const { return &array->array[index]; }

            // Prefix increment
            Iterator &operator++()
            {
                // Get next occupied index;
                index++;
                while (!array->Occupied(index) && index <= array->MaxIndex())
                {
                    index++;
                }
                return *this;
            }
            // Postfix increment
            // Return a copy of this object before the increment.
            // But increment this object, (re)using the prefix increment
            // we just defined.
            Iterator operator++(int)
            {
                Iterator copy = *this;
                ++*this;
                return copy;
            }

            bool operator==(const Iterator &other) const
            {
                return array == other.array && index == other.index;
            }

            bool operator!=(const Iterator &other) const
            {
                return !(*this == other);
            }

          private:
            MyrIDArray *array;
            std::size_t index;
        };

        // Deliberately create an empty array with
        // min used index being 1 past max of 0, so iterating over
        // an empty array will do nothing, because begin() == end()
        MyrIDArray()
            : size(0), min_used_index(1), max_used_index(0),
              max_used_id(InvalidID)
        {
            for (std::size_t i = 0; i < n; ++i)
            {
                occupied[i] = false;
            }
            capacity = n;
            // free_slots.??? no way to clear a queue?
            id_to_index.clear();
        }
        virtual ~MyrIDArray() {}

        // To exist, last used ID wouldn't be InvalidID,
        // and the id to index mapping would have to exist,
        // and the slot would have to be occupied.
        bool Exists(MYR_ID_t id) // const
        {
            bool val = (id != InvalidID) && (size != 0) &&
                       (id_to_index.count(id) == 1) &&
                       (occupied[id_to_index[id]]);
            return val;
        }

        // If the id exists and is occupied, then return the address of the
        // array item at the index mapped to the id, else return null.
        T *Get(MYR_ID_t id)
        {
            return Exists(id) ? &array[id_to_index[id]] : nullptr;
        }

        // Add an item, return the id, or InvalidID if it fails (e.g. no free
        // slots)
        MYR_ID_t Add(T object)
        {
            MYR_ID_t new_id = InvalidID;
            // The index we will use:
            std::size_t new_index = GetNextFreeIndex();
            if (new_index != InvalidID)
            {
                // Either start the id at 0, or the last used + 1
                new_id = size == 0 ? 0 : max_used_id + 1;
                max_used_id = new_id; // This is now the higest id used.
                // Store the data
                array[new_index] = std::move(object);
                // Map the new id to the new_index.
                id_to_index[new_id] = new_index;
                ++size;

                // Update occupied and min/max indicies
                UpdateOccupancy(new_index);
            }
            return new_id;
        }

        // Mark the item as removed.
        // Return a pointer to what it was, though.
        T *Remove(MYR_ID_t id)
        {
            if (!Exists(id))
                return nullptr;
            size_t index = id_to_index[id];
            T *retval = &array[index];
            occupied[index] = false;
            size--;
            free_slots.push(index);
            id_to_index.erase(id);

            if (index == min_used_index)
            {
                // find the first occupancy, if none, invalid.
                if (size == 0)
                    min_used_index = InvalidID;
                else
                {
                    // start at what it was and work forwards...
                    for (size_t i = min_used_index; i <= max_used_index; i++)
                    {
                        if (occupied[i])
                        {
                            min_used_index = i;
                            break;
                        }
                    }
                    // If you cant find an occupancy, something
                    // is wrong.
                    // assert(false, "Remove: Cannot find occupancy to "
                    //              "update min_used_index");
                    // std::cerr << "Remove: Cannot find occupancy to update "
                    //             "min_used_index"
                    //          << std::endl;
                }
            }
            if (index == max_used_index)
            {
                // find the last occupancy, if none, invalid.
                if (size == 0)
                    min_used_index = InvalidID;
                else
                {
                    // start at what it was and work backwards...
                    for (size_t i = max_used_index; i >= min_used_index; i--)
                    {
                        if (occupied[i])
                        {
                            max_used_index = i;
                            break;
                        }
                    }
                    // If you cant find an occupancy, something
                    // is wrong.
                    // assert(false, "Remove: Cannot find occupancy to "
                    //              "update max_used_index");
                    // std::cerr << "Remove: Cannot find occupancy to update "
                    //             "max_used_index"
                    //         << std::endl;
                }
            }
            return retval;
        }

        // Returns the number of used slots
        std::size_t Size() const { return size; }
        // Total number of slots
        std::size_t Capacity() const { return capacity; }

        std::size_t GetIndexForID(MYR_ID_t id)
        {
            if (Exists(id))
            {
                return id_to_index[id];
            }
            return InvalidID;
        }

        // Begin returns the first element
        Iterator begin() { return Iterator(this, min_used_index); }
        // End returns the first illegal element
        Iterator end() { return Iterator(this, max_used_index + 1); }
    };

    // Window Properties Flags
    // You can have multiple of these, but some
    // don't make sense together.
    typedef enum
    {
        MINIMISED = 1,
        MAXIMISED = 2,
        FULLSCREEN = 4,
        UNDECORATED = 8,
        HIDDEN = 16,
        UNFOCUSED = 32,
        TOPMOST = 64,
        RESIZED = 128,
        TRANSPARENT = 256,
        VSYNC = 512,
        HIGHDPI = 1024,
        MSAA_4X = 2048,
        ALWAYS_RUN = 4096

    } WindowPropFlags_t;

    // Window States. You can only be one of these.
    typedef enum
    {
        OPENING,
        READY,
        CLOSING,
        CLOSED
    } WindowState_t;

    typedef uint16_t WindowProps_t;

    struct MYR_API WindowConfig
    {
        Vector2 resolution;
        bool resizable;
        bool vsync;
        bool fullscreen;
        bool borderless;
    };

} // namespace Myriad

#endif
