#ifndef _MYRTAD_TYPES_H_
#define _MYRTAD_TYPES_H_

#include "myriad_config.h"
#include "s8.h" //serialisation

#include <chrono>  // for timers
#include <cstdint> // int32_t
#include <random>  // for random
#include <string>  // std::string

/* Types for the myriad engine */

namespace Myriad
{
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

        static std::string TypeName() { return "Vector2"; }

        static bool Serialise(const Vector2 &object, s8::Serialiser &s,
                              s8::Serialiser::NodeID_t id)
        {
            return s.Value(s.ScalarField(id, "x"), object.x) &&
                   s.Value(s.ScalarField(id, "y"), object.y);
        }

        static bool Deserialise(Vector2 &object, s8::Serialiser &s,
                                s8::Serialiser::NodeID_t id)
        {
            return s.ReadScalar(s.GetFieldID(id, "x"), object.x) &&
                   s.ReadScalar(s.GetFieldID(id, "y"), object.y);
        }
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

    struct MYR_API WindowConfig
    {
        Vector2 resolution;
        bool fullscreen;
        bool resizable;
        bool vsync;
        bool borderless;
        std::string title;

        static std::string TypeName() { return "WindowConfig"; }
        static bool Serialise(const WindowConfig &object, s8::Serialiser &s,
                              s8::Serialiser::NodeID_t id)
        {
            return Vector2::Serialise(object.resolution, s,
                                      s.ObjectField(id, "resolution")) &&
                   s.Value(s.ScalarField(id, "fullscreen"),
                           object.fullscreen) &&
                   s.Value(s.ScalarField(id, "resizable"), object.resizable) &&
                   s.Value(s.ScalarField(id, "vsync"), object.vsync) &&
                   s.Value(s.ScalarField(id, "borderless"),
                           object.borderless) &&
                   s.Value(s.ScalarField(id, "title"), object.title);
        }

        static bool Deserialise(WindowConfig &object, s8::Serialiser &s,
                                s8::Serialiser::NodeID_t id)
        {
            return s.ReadObject(s.GetFieldID(id, "resolution"),
                                object.resolution) &&
                   s.ReadScalar(s.GetFieldID(id, "fullscreen"),
                                object.fullscreen) &&
                   s.ReadScalar(s.GetFieldID(id, "resizable"),
                                object.resizable) &&
                   s.ReadScalar(s.GetFieldID(id, "vsync"), object.vsync) &&
                   s.ReadScalar(s.GetFieldID(id, "borderless"),
                                object.borderless) &&
                   s.ReadScalar(s.GetFieldID(id, "title"), object.title);
        }
    };

    struct MYR_API EngineConfig
    {
        WindowConfig window_config;
        std::string resource_base_path;
        int target_framerate;

        static std::string TypeName() { return "EngineConfig"; }
        static bool Serialise(const EngineConfig &object, s8::Serialiser &s,
                              s8::Serialiser::NodeID_t id)
        {
            return WindowConfig::Serialise(
                       object.window_config, s,
                       s.ObjectField(id, "window_config")) &&
                   s.Value(s.ScalarField(id, "resource_base_path"),
                           object.resource_base_path) &&
                   s.Value(s.ScalarField(id, "target_framerate"),
                           object.target_framerate);
        }

        static bool Deserialise(EngineConfig &object, s8::Serialiser &s,
                                s8::Serialiser::NodeID_t id)
        {
            return WindowConfig::Deserialise(
                       object.window_config, s,
                       s.GetFieldID(id, "window_config")) &&
                   s.ReadScalar(s.GetFieldID(id, "resource_base_path"),
                                object.resource_base_path) &&
                   s.ReadScalar(s.GetFieldID(id, "target_framerate"),
                                object.target_framerate);
        }
    };

    namespace Util
    {
        /* The timer class used to deal in nanoseconds.
           This tended to overflow the datatypes when timing for more than
           about 4 seconds. Switched to milliseconds instead.
        */
        class MYR_API Timer
        {
          private:
            std::chrono::high_resolution_clock::time_point start_time_;
            std::chrono::high_resolution_clock::time_point end_time_;
            uint64_t elapsed_;
            bool started_;
            // const float ns_in_ms = 1000000.0f;

          public:
            Timer() {}
            virtual ~Timer() {}
            void Start()
            {
                Reset();
                start_time_ = std::chrono::high_resolution_clock::now();
                started_ = true;
            }
            void Stop()
            {
                started_ = false;
                end_time_ = std::chrono::high_resolution_clock::now();
                elapsed_ =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        end_time_ - start_time_)
                        .count();
            };

            // Return how much time this timer has recorded in milliseconds
            // If the timer is currently running, return the elapsed time up to
            // now. Otherwise, return the last recorded elapsed time
            // (Start->Stop)
            float Time()
            {
                if (started_)
                {
                    uint32_t ms =
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::high_resolution_clock::now() -
                            start_time_)
                            .count();

                    return ms;
                    // return ns / ns_in_ms;
                }
                else
                    return elapsed_ / 1.0f; // ns_in_ms;
            }

            void Reset()
            {
                started_ = false;
                elapsed_ = 0;
            }
        };

        // For random numbers
        using u32 = uint_least32_t;
        using rand_engine = std::mt19937;
        class MYR_API Random
        {
          protected:
            static Random *s_rand_instance_;
            const float REAL_DIST_MAX = 10.0f;
            std::random_device os_seed_;
            u32 seed_;
            rand_engine *p_generator_;
            std::uniform_real_distribution<float> float_dist;

          public:
            Random()
            {
                seed_ = os_seed_();
                p_generator_ = new rand_engine(seed_);
                float_dist = std::uniform_real_distribution<float>(
                    0.0f, static_cast<float>(REAL_DIST_MAX));
                s_rand_instance_ = this;
            }
            ~Random() { delete p_generator_; }

            // static MyrRandom *Get_Rand() { return s_rand_instance_; }

            static float Float(float fmin, float fmax)
            {
                float fsample = s_rand_instance_->float_dist(
                    *s_rand_instance_->p_generator_);
                // scale to fmin:fmax
                // TODO this sequence may lose precision.
                return (fsample / s_rand_instance_->REAL_DIST_MAX) *
                           (fmax - fmin) +
                       fmin;
            }
        };
    } // namespace Util

} // namespace Myriad

#endif
