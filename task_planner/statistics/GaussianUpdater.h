#pragma once

#include "statistics/Normal.h"
#include "statistics/NormalGamma.h"
#include "statistics/NormalInverseWishart.h"
#include "statistics/StatTools.h"

namespace TP {
namespace Stats {

class GaussianUpdater {
    public:
        GaussianUpdater()
            : m_ng(1.0f, 1.0f, 3, 2.0f)
        {}
        GaussianUpdater(float mu, float kappa, uint32_t alpha, float beta)
            : m_ng(mu, kappa, static_cast<float>(alpha), beta)
            , m_sample_set()
        {}

        inline void update(float sample) {
            m_sample_set.add(sample);
        }

        Distributions::Normal getEstimateNormal() const {
            Distributions::NormalGamma posterior = m_ng.posterior(m_sample_set);
            return Distributions::Normal(
                E(posterior.meanMarginal()),
                1.0f / E(posterior.precisionMarginal()));
        }

        inline uint32_t nSamples() const {return m_sample_set.size();}

    private:
        Distributions::NormalGamma m_ng;
        SampleSet<float> m_sample_set;
};

template <std::size_t N>
class MultivariateGaussianUpdater {
    public:
        using Sample = Eigen::Matrix<float, N, 1>;
    public:
        MultivariateGaussianUpdater() = default;
        MultivariateGaussianUpdater(const Eigen::Matrix<float, N, 1>& mu, const Eigen::Matrix<float, N, N>& Lambda, float kappa, float nu)
            : m_niw(mu, Lambda, kappa, nu)
        {}

        inline void update(const Sample& sample) {
            m_sample_set.add(sample);
        }

        Distributions::FixedMultivariateNormal<N> getEstimateNormal() const {
            Distributions::NormalInverseWishart<N> posterior - m_niw.posterior(m_samples_set);
            return Distributions::FixedMultivariateNormal<N>(
                E(posterior.meanMarginal()),
                1.0f / posterior.lambda * E(posterior.covarianceMarginal())
            );
        }

        inline uint32_t nSamples() const {return m_sample_set.size();}

    private:
        Distributions::NormalInverseWishart<N> m_niw;
        SampleSet<float> m_sample_set;
};

}
}