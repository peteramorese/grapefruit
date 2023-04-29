#pragma once

#include <cmath>
#include <math.h>

#include <Eigen/Dense>

#include "tools/Logging.h"
#include "statistics/StatTools.h"
#include "statistics/AdvancedDistributions.h"

#define _USE_MATH_DEFINES

namespace TP {
namespace Stats {
namespace Distributions {


struct NormalGamma {
    public:
        float mu_0 = 0.0f;
        float kappa_0 = 0.0f;
        float alpha_0 = 1;
        float beta_0 = 0.0f;

    public:
        NormalGamma(float mu_0_, float kappa_0_, float alpha_0_, float beta_0_)
            : mu_0(mu_0_)
            , kappa_0(kappa_0_)
            , alpha_0(alpha_0_)
            , beta_0(beta_0_)
        {}

        inline float normalizationConstant() const {
            ASSERT(alpha_0 >= 1.0f, "'alpha' cannot be zero");
            return std::pow(beta_0, alpha_0) * std::sqrt(kappa_0 / M_2_PI) / tgamma(alpha_0);
        }

        float pdf(float mean, float precision) const {
            return normalizationConstant() * std::pow(precision, static_cast<float>(alpha_0) - 0.5f) 
                * std::exp(-0.5f * precision * (kappa_0 * std::pow(mean - mu_0, 2) + 2.0f * beta_0));
        }

        LocationScaleT meanMarginal() const {
            return LocationScaleT(2.0f * alpha_0, mu_0, beta_0 / (alpha_0 * kappa_0));
        }

        Gamma precisionMarginal() const {
            return Gamma(alpha_0, beta_0);
        }

        NormalGamma posterior(const SampleSet& sample_set) const {
            float n = static_cast<float>(sample_set.size());
            float x_bar = sample_set.avg();

            float sum_sq_err = 0.0f;
            for (float x : sample_set.getSamples()) {
                sum_sq_err += std::pow(x - x_bar, 2);
            }

            return NormalGamma(
                (kappa_0 * mu_0 + n * x_bar) / (kappa_0 + n),
                kappa_0 + n,
                alpha_0 + n / 2.0f,
                beta_0 + 0.5f * sum_sq_err + (kappa_0 * n * std::pow(x_bar - mu_0, 2)) / (2.0f * (kappa_0 + n))
            );
        }

};


} // namespace Distributions
} // namespace Stats
} // namespace TP