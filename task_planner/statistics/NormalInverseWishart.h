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
        FixedNormalInverseWishart(const Eigen::Matrix<float, N, 1>& mu, const Eigen::Matrix<float, N, N>& Lambda, float kappa, float nu)
            : m_mu(mu)
            , m_Lambda(Lambda)
            , m_kappa(kappa)
            , m_nu(nu)
        {}

        FixedMultivariateT<N> meanMarginal() const {
            float dof = m_nu - static_cast<float>(N) + 1.0f;
            return FixedMultivariateT<N>(dof, 1.0f / (m_kappa * dof) * m_Lambda.inverse());
        }

        FixedInverseWishart<N> covarianceMarginal() const {
            return FixedInverseWishart<N>(m_nu, m_Lambda.inverse());
        }

        FixedNormalInverseWishart<N> posterior(const SampleSet<Eigen::Matrix<float, N, 1>>& sample_set) const {
            float n = static_cast<float>(sample_set.size());
            const Eigen::Matrix<float, N, 1>& x_bar = sample_set.avg();

            Eigen::Matrix<float, N, N> sum_sq_err = Eigen::Matrix<float, N, N>::Zero();
            for (const auto& x : sample_set.getSamples()) {
                sum_sq_err += (x - x_bar) * (x - x_bar).transpose();
            }

            return FixedNormalInverseWishart<N>(
                (m_kappa / (m_kappa + n)) * m_mu + (n / (m_kappa + n)) * x_bar,
                m_Lambda + sum_sq_err + (m_kappa * n) / (m_kappa + n) * (x_bar - m_mu) * (x_bar - m_mu).transpose(),
                m_kappa + n,
                m_nu + n
            );
        }

    private:
        Eigen::Matrix<float, N, 1> m_mu = Eigen::Matrix<float, N, 1>::Zero();
        Eigen::Matrix<float, N, N> m_Lambda = Eigen::Matrix<float, N, N>::Identity();
        float m_kappa = 1.0f;
        float m_nu = 3.0f;

};

}
}
}