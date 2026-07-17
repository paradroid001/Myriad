#ifndef MYRIAD_CORE_CONTAINERS_H
#define MYRIAD_CORE_CONTAINERS_H

#include <algorithm>
#include <cstdint>
#include <unordered_map>

#include "myriad.h" //MYR_ID_t, MYRIAD_INVALID_ID

namespace Myriad
{
    /**
     * @brief: An base class which generates monotonically
     * increasing integer ids.
     * @details: Does not reuse ids, they always increase
     * @todo: there's no check on 'max id'
     */
    class MyrIDSource
    {
      private:
        MYR_ID_t current_id;

      public:
        MyrIDSource() : current_id(0) {}
        virtual ~MyrIDSource() {}
        virtual inline MYR_ID_t GetNextID() { return ++current_id; }
    };

    /*
     * @brief: Hold objects of type T in a map, keyed on ID
     * @note: Requires the object has a copy constructor.
     *        Also really should be redone with hashable
     *        objects for comparisons,
     */
    template <typename T> class MyrIDMap : public MyrIDSource
    {
      private:
        std::unordered_map<MYR_ID_t, T> map;

      public:
        typename std::unordered_map<MYR_ID_t, T>::iterator begin()
        {
            return map.begin();
        }
        typename std::unordered_map<MYR_ID_t, T>::iterator end()
        {
            return map.end();
        }

        bool Exists(MYR_ID_t id) { return map.count(id) == 1; }

        T *Get(MYR_ID_t id)
        {
            T *retval = nullptr;
            if (Exists(id))
            {
                retval = &map[id];
            }
            return retval;
        }

        MYR_ID_t GetIDFor(T object)
        {
            for (auto it = map.begin(); it != map.end(); it++)
            {
                // Note: here you really need a Hashable
                // object or something.
                if (it->second == object)
                {
                    return it->first;
                }
            }

            return MYRIAD_INVALID_ID;
        }

        /**
         * @brief Add an object, return the generated id. If already added,
         * return that id.
         */
        MYR_ID_t Add(T object)
        {
            MYR_ID_t retval = MYRIAD_INVALID_ID;
            MYR_ID_t found_id = GetIDFor(object);

            if (found_id == MYRIAD_INVALID_ID)
            {
                // not found
                retval = GetNextID();
                map[retval] = object;
            }
            else
            {
                retval = found_id;
            }
            return retval;
        }

        T *Remove(MYR_ID_t id)
        {
            T *retval = nullptr;
            if (Exists(id))
            {
                // it does exist
                retval = &map[id]; // TODO this will only work for heap
                                   // allocated
                for (auto it = map.begin(); it != map.end(); /* no increment */)
                {
                    if (it->first == id)
                    {
                        retval = &it->second;
                        it = map.erase(it);
                    }
                    else
                        ++it;
                }
            }
            return retval;
        }
    };

} // namespace Myriad
#endif
