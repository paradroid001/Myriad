#ifndef __SINGLETON_H_
#define __SINGLETON_H_

#include "core/core.h"

namespace Myriad
{
    template <class T> class MYR_API Singleton
    {
        public:
            //No cloning
            Singleton(Singleton const& other) = delete;
            //No assigning
            //Singleton& operator=(const Singleton &) = delete;
            void operator=(Singleton const&) = delete;
            //Give the instance as a reference
            static T& GetInstance()
            {
                //guaranteed to be destroyed,
                //initialised on first use
                //no dynamic memory
                //and apparently thread safe...
                //https://stackoverflow.com/questions/1008019/how-do-you-implement-the-singleton-design-pattern/1008289#1008289
                //
                //Using the allow (seems to) help with allowing THIS class
                //and non-friend template descendents call the constructor.
                //But public peeps cannot.
                //https://stackoverflow.com/questions/51974670/implementing-singleton-with-crtp/51976202#51976202
                static T instance{allow()};
                return instance; 
            }
        private:
            struct allow {};
        protected:
            Singleton(allow){}           
    };
}

#endif