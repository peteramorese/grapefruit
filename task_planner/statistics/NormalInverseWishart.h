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

template <std::size_t N>
class FixedNormalInverseWishart {
    public:
        Eigen::Matrix<float, N, 1> mu = Eigen::Matrix<float, N, 1>::Zero();
        Eigen::Matrix<float, N, N> Lambda = Eigen::Matrix<float, N, N>::Identity();
        float kappa = 1.0f;
        float nu = static_cast<float>(N + 3);

    public:
        FixedNormalInverseWishart() = default;
        FixedNormalInverseWishart(const Eigen::Matrix<float, N, 1>& mu_, const Eigen::Matrix<float, N, N>& Lambda_, float kappa_, float nu_)
            : mu(mu_)
            , Lambda(Lambda_)
            , kappa(kappa_)
            , nu(nu_)
        {}

        static constexpr std::size_t uniqueElements() {return N * (N + 3) / 2;} // Mean elements + unique elements of covariance

        FixedMultivariateT<N> meanMarginal() const {
            float dof = nu - static_cast<float>(N) + 1.0f;
            return FixedMultivariateT<N>(dof, mu, 1.0f / (kappa * dof) * Lambda);
        }

        MinimalFixedInverseWishart<N> covarianceMarginal() const {
            return MinimalFixedInverseWishart<N>(nu, Lambda);
        }

        FixedNormalInverseWishart<N> posterior(const SampleSet<Eigen::Matrix<float, N, 1>>& sample_set) const {
            float n = static_cast<float>(sample_set.size());
            const Eigen::Matrix<float, N, 1>& x_bar = sample_set.avg();

            Eigen::Matrix<float, N, N> susq_err = Eigen::Matrix<float, N, N>::Zero();
            for (const auto& x : sample_set.getSamples()) {
                susq_err += (x - x_bar) * (x - x_bar).transpose();
            }

            return FixedNormalInverseWishart<N>(
                (kappa * mu + n * x_bar) / (kappa + n),
                Lambda + susq_err + (kappa * n) / (kappa + n) * (x_bar - mu) * (x_bar - mu).transpose(),
                kappa + n,
                nu + n
            );
        }

};

}
}
}