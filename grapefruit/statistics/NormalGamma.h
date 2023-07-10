#pragma once

#include <cmath>
#include <math.h>

#include <Eigen/Dense>

#include "tools/Logging.h"
#include "statistics/StatTools.h"
#include "statistics/AdvancedDistributions.h"

#define _USE_MATH_DEFINES

namespace GF {
namespace Stats {
namespace Distributions {


struct NormalGamma {
    public:
        float mu = 0.0f;
        float kappa = 0.0f;
        float alpha = 1;
        float beta = 0.0f;

    public:
        NormalGamma(float mu_, float kappa_, float alpha_, float beta_)
            : mu(mu_)
            , kappa(kappa_)
            , alpha(alpha_)
            , beta(beta_)
        {}

        inline float normalizationConstant() const {
            ASSERT(alpha >= 1.0f, "'alpha' cannot be zero");
            return std::pow(beta, alpha) * std::sqrt(kappa / M_2_PI) / tgamma(alpha);
        }

        float pdf(float mean, float precision) const {
            return normalizationConstant() * std::pow(precision, static_cast<float>(alpha) - 0.5f) 
                * std::exp(-0.5f * precision * (kappa * std::pow(mean - mu, 2) + 2.0f * beta));
        }

        LocationScaleT meanMarginal() const {
            return LocationScaleT(2.0f * alpha, mu, beta / (alpha * kappa));
        }

        Gamma precisionMarginal() const {
            return Gamma(alpha, beta);
        }

        NormalGamma posterior(const SampleSet<float>& sample_set) const {
            float n = static_cast<float>(sample_set.size());
            float x_bar = sample_set.avg();

            float sum_sq_err = 0.0f;
            for (float x : sample_set.getSamples()) {
                sum_sq_err += std::pow(x - x_bar, 2);
            }

            return NormalGamma(
                (kappa * mu + n * x_bar) / (kappa + n),
                kappa + n,
                alpha + n / 2.0f,
                beta + 0.5f * sum_sq_err + (kappa * n * std::pow(x_bar - mu, 2)) / (2.0f * (kappa + n))
            );
        }

};


} // namespace Distributions
} // namespace Stats
} // namespace GF