#ifndef MYRIAD_CORE_THREAD_THREADSAFEQUEUE_H
#define MYRIAD_CORE_THREAD_THREADSAFEQUEUE_H

#include <queue>

namespace Myriad
{
    // Based on the threadsafe queue from Anthony Williams'
    // C++ Concurrency in Action, chapter 6
    //
    template <typename T> class ThreadsafeQueue
    {
      private:
        mutable std::mutex mtx;
        std::queue<T> queue_;
        std::condition_variable cvar_;

      public:
        ThreadsafeQueue() {}
        void push(T new_value)
        {
            // Aquire lock
            std::lock_guard<std::mutex> lock(mtx);
            queue_.push(std::move(new_value));
            cvar_.notify_one();
        }

        void wait_and_pop(T &value)
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvar_.wait(lock, [this] { return !queue_.empty(); });
            value = std::move(queue_.front());
            queue_.pop();
        }
        std::shared_ptr<T> wait_and_pop()
        {
            std::unique_lock<std::mutex> lock(mtx);
            cvar_.wait(lock, [this] { return !queue_.empty(); });
            std::shared_ptr<T> result(
                std::make_shared<T>(std_move(queue_.front())));
            queue_.pop;
            return result;
        }
        // returns True/False if queue wasn't empty, value in:out
        bool try_pop(T &value)
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (queue_.empty())
                return false;
            value = std::move(queue_.front());
            queue_.pop();
            return true;
        }
        // Returns ptr to the item popped, or null.
        std::shared_ptr<T> try_pop()
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (queue_.empty())
                return std::shared_ptr<T>(); // null
            std::shared_ptr<T> result(
                std::make_shared<T>(std::move(queue_.front())));
            queue_.pop();
            return result;
        }
        bool empty() const
        {
            std::lock_guard<std::mutex> lock(mtx);
            return queue_.empty();
        }
    };
} // namespace Myriad
#endif
