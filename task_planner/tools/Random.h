#pragma once

#include <random>

#include "tools/Logging.h"

namespace TP {

class RNG {
    private:
        static std::random_device& s_rd() {static std::random_device rd; return rd;}
        static std::mt19937& s_random_gen() {static std::mt19937 gen(s_rd()()); return gen;}
        static std::mt19937_64& s_random_gen_64() {static std::mt19937_64 gen(s_rd()()); return gen;}
        static std::mt19937& s_seeded_gen() {static std::mt19937 gen(123); return gen;}
        static std::uniform_real_distribution<float>& s_real_dist() {static std::uniform_real_distribution<float> real_dist(0.0f, 1.0f); return real_dist;}
        static std::uniform_int_distribution<>& s_int_dist() {static std::uniform_int_distribution<> int_dist(INT32_MIN, INT32_MAX); return int_dist;}

    public:
        static uint64_t uuid64() {
            return s_int_dist()(s_random_gen_64());
        }

        static int randi(uint32_t lower, uint32_t upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            int unbounded_value = s_int_dist()(s_random_gen());
            return (unbounded_value % (upper - lower)) + lower;
        }

        static float randf(float lower, float upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            return (upper - lower) * s_real_dist()(s_random_gen()) + lower;
        }

        static int srandi(uint32_t lower, uint32_t upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            int unbounded_value = s_int_dist()(s_seeded_gen());
            return (unbounded_value % (upper - lower)) + lower;
        }

        static float srandf(float lower, float upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            return (upper - lower) * s_real_dist()(s_seeded_gen()) + lower;
        }

        static void seed(uint32_t s) {s_seeded_gen().seed(s);}
};

}