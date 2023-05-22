#pragma once

#include "Normal.h"
#include "NormalInverseWishart.h"

namespace TP {
namespace Stats {

template <std::size_t N>
Distributions::FixedMultivariateNormal<Distributions::FixedNormalInverseWishart<N>::uniqueElements()> matchNIWToMVN(const Distributions::FixedNormalInverseWishart<N>& niw) {
    Distributions::FixedInverseWishart<N> covariance_marginal = niw.covarianceMarginal();
    FixedMultivariateT<N> mean_marginal = meanMarginal();
    Eigen::Matrix<float, Distributions::FixedNormalInverseWishart<N>::uniqueElements(), 1> mean_concatentated;
    mean_concatentated << mean_marginal.mu << covariance_marginal
    return FixedMultivariateNormal<FixedNormalInverseWishart<N>::uniqueElements()>(

    );
}

}
}