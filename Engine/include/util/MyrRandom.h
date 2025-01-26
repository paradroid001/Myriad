#ifndef MYRIAD_UTIL_MYRRANDOM_H
#define MYRIAD_UTIL_MYRRANDOM_H

#include "core/core.h"
#include <cctype>
#include <random>
// For random numbers
using u32 = uint_least32_t;
using rand_engine = std::mt19937;

namespace Myriad
{
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
      float_dist = std::uniform_real_distribution<float>(0.0f, static_cast<float>(REAL_DIST_MAX));
      s_rand_instance_ = this;
    }
    ~MyrRandom()
    {
      delete p_generator_;
    }

    static float Float(float fmin, float fmax)
    {
      float fsample = s_rand_instance_->float_dist(*s_rand_instance_->p_generator_);
      // scale to fmin:fmax
      // TODO this sequence may lose precision.
      return (fsample / s_rand_instance_->REAL_DIST_MAX) * (fmax - fmin) + fmin;
    }
  };
}

#endif
