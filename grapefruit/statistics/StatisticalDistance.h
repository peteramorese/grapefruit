#pragma once

#include <Eigen/Dense>

#include "statistics/Normal.h"

namespace GF {
namespace Stats {

template <std::size_t N>
inline static float KLD(const Distributions::FixedMultivariateNormal<N>& p, const Distributions::FixedMultivariateNormal<N>& q) {
    Eigen::Matrix<float, N, N> q_Sigma_inv = q.Sigma.inverse();
    float kld = (q_Sigma_inv * p.Sigma).trace();
    kld += -static_cast<float>(N) + (q.mu - p.mu).transpose() * q_Sigma_inv * (q.mu - p.mu);
    kld += std::log(q.Sigma.determinant() / p.Sigma.determinant());
    return 0.5f * kld;
}

template <std::size_t N>
inline static float wasserstein2(const Distributions::FixedMultivariateNormal<N>& p, const Distributions::FixedMultivariateNormal<N>& q) {
    return (p.mu - q.mu).squaredNorm() + (p.Sigma + q.Sigma - 2.0f * (p.Sigma * q.Sigma).sqrt()).trace();
}

    
}
}