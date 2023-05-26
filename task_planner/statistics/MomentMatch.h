#pragma once

#include "Normal.h"
#include "NormalInverseWishart.h"

namespace TP {
namespace Stats {

template <std::size_t N>
Distributions::FixedMultivariateNormal<Distributions::FixedNormalInverseWishart<N>::uniqueElements() + N> matchNIWToMVN(const Distributions::FixedNormalInverseWishart<N>& niw) {
    constexpr std::size_t mvn_dim = Distributions::FixedNormalInverseWishart<N>::uniqueElements() + N;

    Distributions::FixedMultivariateNormal<Distributions::FixedNormalInverseWishart<N>::uniqueElements() + N> mvn;
    Distributions::MinimalFixedInverseWishart<N> covariance_marginal = niw.covarianceMarginal();
    Distributions::FixedMultivariateT<N> mean_marginal = niw.meanMarginal();

    Eigen::Matrix<float, N, 1> mean_of_mean = E(mean_marginal);
    Eigen::Matrix<float, Distributions::FixedNormalInverseWishart<N>::uniqueElements(), 1> mean_of_covariance = E(covariance_marginal);
    Eigen::Matrix<float, N, N> covariance_of_mean = var(mean_marginal);
    Eigen::Matrix<float, Distributions::FixedNormalInverseWishart<N>::uniqueElements(), Distributions::FixedNormalInverseWishart<N>::uniqueElements()> covariance_of_covariance = var(covariance_marginal);

    for (std::size_t i = 0; i < mvn_dim; ++i) {
        mvn.mu(i) = (i < N) ? mean_of_mean(i) : mean_of_covariance(i - N);
        for (std::size_t j = 0; j < mvn_dim; ++j) {
            if (i < N && j < N) {
                mvn.Sigma(i, j) = covariance_of_mean(i, j);
            } else if (i >= N && j >= N) {
                mvn.Sigma(i, j) = covariance_of_covariance(i - N, j - N);
            } else {
                mvn.Sigma(i, j) = 0.0f;
            }
        }
    }
    return mvn;
}

}
}