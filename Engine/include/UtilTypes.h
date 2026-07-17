#ifndef _MYRIAD_UTILTYPES_H_
#define _MYRIAD_UTILTYPES_H_

#include <chrono>
#include <memory> //shared_ptr
#include <random>

#include "EngineConfig.h" //MYR_API

namespace Myriad
{
    class MYR_API MyrTimer
    {
      private:
        std::chrono::high_resolution_clock::time_point start_time_;
        std::chrono::high_resolution_clock::time_point end_time_;
        uint32_t elapsed_;
        bool started_;
        const float ns_in_ms = 1000000.0f;

      public:
        MyrTimer() {}
        virtual ~MyrTimer() {}
        void Start()
        {
            started_ = true;
            Reset();
        }
        void Stop()
        {
            started_ = false;
            end_time_ = std::chrono::high_resolution_clock::now();
            elapsed_ = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           end_time_ - start_time_)
                           .count();
        };

        float Time()
        {
            if (started_)
            {
                uint32_t ns =
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::high_resolution_clock::now() - start_time_)
                        .count();
                return ns / ns_in_ms;
            }
            else
                return 0;
        }
        float Elapsed() const { return elapsed_ / ns_in_ms; }
        void Reset()
        {
            start_time_ = std::chrono::high_resolution_clock::now();
        }
    };

    /* Interfaces */

    class MYR_API ILogger
    {
      public:
        typedef enum MyrLogLevel_t
        {
            MYR_LOGLEVEL_TRACE = 0,
            MYR_LOGLEVEL_INFO = 2,
            MYR_LOGLEVEL_WARNING = 4,
            MYR_LOGLEVEL_ERROR = 8,
            MYR_LOGLEVEL_CRITICAL = 16,
        } MyrLogLevel_t;
        virtual ~ILogger() = 0;
        virtual void SetLogLevel(MyrLogLevel_t level) = 0;
        virtual void Log(MyrLogLevel_t loglevel, const char *fmt, ...) = 0;
    };

    // Create a global loggers to be used in Log calls.
    // Since these need to be used in macros inserted into
    // Any code, the client one needs MYR_API.
    extern MYR_API std::shared_ptr<ILogger> global_logger_core;
    extern MYR_API std::shared_ptr<ILogger> global_logger_client;

#define MYR_TRACE(...)                                                         \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_TRACE, __VA_ARGS__)
#define MYR_INFO(...)                                                          \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_INFO, __VA_ARGS__)
#define MYR_WARN(...)                                                          \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_WARNING, __VA_ARGS__)
#define MYR_ERROR(...)                                                         \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_ERROR, __VA_ARGS__)
#define MYR_CRITICAL(...)                                                      \
    Myriad::global_logger_client->Log(                                         \
        Myriad::ILogger::MyrLogLevel_t::MYR_LOGLEVEL_CRITICAL, __VA_ARGS__)

    // For random numbers
    using u32 = uint_least32_t;
    using rand_engine = std::mt19937;
    class MYR_API MyrRandom
    {
      protected:
        static MyrRandom *s_rand_instance_;
        const float REAL_DIST_MAX = 10.0f;
        std::random_device os_seed_;
        u32 seed_;
        rand_engine *p_generator_;
        std::uniform_real_distribution<float> float_dist;

      public:
        MyrRandom()
        {
            seed_ = os_seed_();
            p_generator_ = new rand_engine(seed_);
            float_dist = std::uniform_real_distribution<float>(
                0.0f, static_cast<float>(REAL_DIST_MAX));
            s_rand_instance_ = this;
        }
        ~MyrRandom() { delete p_generator_; }

        // static MyrRandom *Get_Rand() { return s_rand_instance_; }

        static float Float(float fmin, float fmax)
        {
            float fsample =
                s_rand_instance_->float_dist(*s_rand_instance_->p_generator_);
            // scale to fmin:fmax
            // TODO this sequence may lose precision.
            return (fsample / s_rand_instance_->REAL_DIST_MAX) * (fmax - fmin) +
                   fmin;
        }
    };

} // namespace Myriad

#endif
