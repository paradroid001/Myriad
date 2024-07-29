#ifndef MYRIAD_CORE_OBJECT_MYROBJECT
#define MYRIAD_CORE_OBJECT_MYROBJECT

#include <string>
#include <vector>

#include "core/MyrHandle.h"
#include "core/core.h"
#include "core/memory/AllocatorProvider.h"
#include "io/Log.h"

namespace Myriad
{
    // A base class for objects.
    class MYR_API MyrObject
    {

      protected:
        std::string name;
        MyrObject *parent;
        std::vector<MyrObject *> children;
        bool destroyed;

      public:
        MyrObject() : name("None"), parent(NULL), destroyed(false) {}
        // This is meant to be private, but the allocator then can't call new.
        MyrObject(const std::string object_name)
            : name(object_name), parent(NULL), destroyed(false)
        { /*Nothing*/
        } // private constructor - you can't create these.

        virtual ~MyrObject() { MYR_CORE_TRACE("Destructing a MyrObject."); }

        const std::string &GetName() const { return name; }
        bool IsDestroyed() const { return destroyed; }
        size_t NumChildren() const { return children.size(); }

        // MyrHandle<MyrObject> &GetChildAtIndex(size_t index);
        // MyrHandle<MyrObject> &RemoveChildAtIndex(size_t index);

        void SetParent(MyrObject *parent)
        {
            // if you are setting my parent, I need to
            // leave my current parent.
        }

        bool AddChild(MyrObject *new_child)
        {
            if (std::find(children.begin(), children.end(), new_child) !=
                children.end())
            {
                MYR_CORE_ERROR("Attempt to add child, child already exists.");
                return false;
            }
            children.push_back(new_child);
            return true;
        }

        bool RemoveChild(MyrObject *child) { return false; }

        void Destroy() { destroyed = true; }
    };

} // namespace Myriad
#endif
