#pragma once

#include "tools/Containers.h"
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

        const Distributions::NormalGamma& dist() const {return m_ng;}

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
        MultivariateGaussianUpdater() 
            : m_sample_set(Eigen::Matrix<float, N, 1>::Zero())
        {}

        MultivariateGaussianUpdater(const Eigen::Matrix<float, N, 1>& mu, const Eigen::Matrix<float, N, N>& Lambda, float kappa, float nu)
            : m_niw(mu, Lambda, kappa, nu)
            , m_sample_set(Eigen::Matrix<float, N, 1>::Zero())
        {}

        inline void update(const Eigen::Matrix<float, N, 1>& sample) {
            m_sample_set.add(sample);
        }

        inline void update(const Containers::FixedArray<N, float>& sample) {
            Eigen::Matrix<float, N, 1> out;
            toColMatrix<float, N>(sample, out);
            m_sample_set.add(out);
        }

        Distributions::FixedMultivariateNormal<N> getEstimateNormal() const {
            Distributions::FixedNormalInverseWishart<N> posterior = m_niw.posterior(m_sample_set);
            return Distributions::FixedMultivariateNormal<N>(
                E(posterior.meanMarginal()),
                E(minimalWishartToWishart(posterior.covarianceMarginal()))
            );
        }

        //Distributions::FixedMultivariateNormal<N> getEstimateNormalPosterior(const Eigen::Matrix<float, N, 1>& sample) {
        //    Distributions::FixedNormalInverseWishart<N> posterior = tempPosterior(sample);
        //    return Distributions::FixedMultivariateNormal<N>(
        //        E(posterior.meanMarginal()),
        //        1.0f / posterior.kappa * E(minimalWishartToWishart(posterior.covarianceMarginal()))
        //    );
        //}

        Distributions::FixedNormalInverseWishart<N> dist() const {return m_niw.posterior(m_sample_set);}
        Distributions::FixedNormalInverseWishart<N> tempPosterior(const Eigen::Matrix<float, N, 1>& sample) {
            /* Get the posterior as if the sample was added, but sample is not kept */
            m_sample_set.add(sample); // add the sample to the sample set
            Distributions::FixedNormalInverseWishart<N> posterior = m_niw.posterior(m_sample_set); // calculate as if sample was part of the set
            m_sample_set.pop_back(); // remove the sample from the set
            return posterior;
        }

        inline uint32_t nSamples() const {return m_sample_set.size();}

    private:
        Distributions::FixedNormalInverseWishart<N> m_niw;
        SampleSet<Eigen::Matrix<float, N, 1>> m_sample_set;
};

}
}