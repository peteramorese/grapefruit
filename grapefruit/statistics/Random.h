#pragma once

#include <random>
#include <Eigen/Dense>

#include "tools/Logging.h"
#include "statistics/Normal.h"

namespace GF {

class RNG {
    private:
        inline static std::random_device& s_rd() {static std::random_device rd; return rd;}
        inline static std::mt19937& s_random_gen() {static thread_local std::mt19937 gen(s_rd()()); return gen;}
        inline static std::mt19937_64& s_random_gen_64() {static thread_local std::mt19937_64 gen(s_rd()()); return gen;}
        inline static std::mt19937& s_seeded_gen() {static thread_local std::mt19937 gen(123); return gen;}
        inline static std::uniform_real_distribution<float>& s_real_dist() {static std::uniform_real_distribution<float> real_dist(0.0f, 1.0f); return real_dist;}
        inline static std::uniform_int_distribution<>& s_int_dist() {static std::uniform_int_distribution<> int_dist(INT32_MIN, INT32_MAX); return int_dist;}

    public:
        inline static uint64_t uuid64() {
            return s_int_dist()(s_random_gen_64());
        }

        inline static int randiUnbounded() {
            return s_int_dist()(s_random_gen());
        }

        inline static int srandiUnbounded() {
            return s_int_dist()(s_seeded_gen());
        }

        inline static int randi(int lower, int upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            int diff = (upper - lower);
            return ((randiUnbounded() % diff) + diff) % diff + lower;
        }

        inline static float randf(float lower, float upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            return (upper - lower) * s_real_dist()(s_random_gen()) + lower;
        }

        inline static int srandi(uint32_t lower, uint32_t upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            int diff = (upper - lower);
            return ((srandiUnbounded() % diff) + diff) % diff + lower;
        }

        inline static float srandf(float lower, float upper) {
            ASSERT(lower <= upper, "Upper bound must be geq to lower bound");
            return (upper - lower) * s_real_dist()(s_seeded_gen()) + lower;
        }

        inline static void seed(uint32_t s) {s_seeded_gen().seed(s);}

        inline static float nrand() {
            std::normal_distribution<> dist;
            return dist(s_random_gen());
        }

        inline static float nrand(double mean, double std) {
            std::normal_distribution<> dist(mean, std);
            return dist(s_random_gen());
        }

        inline static float nsrand() {
            std::normal_distribution<> dist;
            return dist(s_seeded_gen());
        }

        inline static float nsrand(double mean, double std) {
            std::normal_distribution<> dist(mean, std);
            return dist(s_seeded_gen());
        }

        inline static float nrand(const Stats::Distributions::Normal& dist) {
            return nrand(dist.mu, dist.std());
        }

        inline static float nsrand(const Stats::Distributions::Normal& dist) {
            return nsrand(dist.mu, dist.std());
        }

        template <std::size_t N>
        static Eigen::Matrix<float, N, 1> mvnrand(const Stats::Distributions::FixedMultivariateNormalSampler<N>& sampler) {
            Eigen::Matrix<float, N, 1> rv = Eigen::Matrix<float, N, 1>::Zero();
            return sampler.mean() + sampler.transform() * rv.unaryExpr([&](auto x) {return nrand();});
        }
};

}