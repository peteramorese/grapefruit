#pragma once

#include <cmath>
#include <math.h>

#include "statistics/StatTools.h"
#include "tools/Logging.h"

#define _USE_MATH_DEFINES

namespace TP {
namespace Stats {
namespace Distributions {
    
struct Gamma {
    public:
        uint32_t alpha = 1;
        float beta = 1.0f;

    public:
        Gamma() = default;
        Gamma(uint32_t alpha_, float beta_)
            : alpha(alpha_)
            , beta(beta_)
        {}

        float pdf(float x) const {
            ASSERT(x >= 0.0f, "Input 'x' must be geq 0");
            ASSERT(alpha >= 1, "'alpha' cannot be zero");
            return std::pow(x, alpha - 1) * std::exp(-beta * x) * std::pow(beta, alpha) / tgamma(alpha);
        }
};

struct LocationScaleT {
    public:
        uint32_t nu = 0;
        float mu = 0.0f;
        float sigma_2 = 0.0f;

    public:
        LocationScaleT() = default;
        LocationScaleT(uint32_t nu_, float mu_, float sigma_2_) 
            : nu(nu_)
            , mu(mu_)
            , sigma_2(sigma_2_)
        {}

        inline float normalizationConstant() const {
            float nu_f = static_cast<float>(nu);
            return tgamma(0.5f * (nu_f + 1.0f)) / (tgamma(0.5f * nu_f) * std::sqrt(nu_f * M_PI * sigma_2));
        }

        float pdf(float x) const {
            return normalizationConstant() * std::pow(1.0f + std::pow(x - mu, 2) / (static_cast<float>(nu) * sigma_2), -(0.5f * static_cast<float>(nu + 1)));
        }
};

} // namespace Distributions

// Expectation and variance overloads
inline static float E(const Distributions::Gamma& p) {ASSERT(p.beta > 0.0f, "Singular value"); return static_cast<float>(p.alpha) / p.beta;}
inline static float var(const Distributions::Gamma& p) {ASSERT(p.beta > 0.0f, "Singular value"); return static_cast<float>(p.alpha) / (p.beta * p.beta);}
inline static float E(const Distributions::LocationScaleT& p) {ASSERT(p.nu >= 2, "'nu' must be geq 2 to find expected value"); return p.mu;}
inline static float var(const Distributions::LocationScaleT& p) {ASSERT(p.nu >= 3, "'nu' must be geq 3 to find variance"); return p.nu * p.sigma_2 / static_cast<float>(p.nu - 2u);} 

} // namespace Stats
} // namespace TP