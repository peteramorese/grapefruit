#pragma once

#include "tools/Containers.h"
#include "tools/Serializer.h"

#include "statistics/Normal.h"
#include "statistics/NormalGamma.h"
#include "statistics/NormalInverseWishart.h"
#include "statistics/StatTools.h"


namespace GF {
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

        MultivariateGaussianUpdater(const Eigen::Matrix<float, N, 1>& default_mu) 
            : m_sample_set(Eigen::Matrix<float, N, 1>::Zero())
        {
            m_niw.mu = default_mu;
        }

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

        const Distributions::FixedNormalInverseWishart<N>& priorDist() const {return m_niw;}
        Distributions::FixedNormalInverseWishart<N>& priorDist() {return m_niw;}

        Distributions::FixedNormalInverseWishart<N> tempPosterior(const Eigen::Matrix<float, N, 1>& sample) const {
            /* Get the posterior as if the sample was added, but sample is not kept */
            PosteriorSampleSet<Eigen::Matrix<float, N, 1>> posterior_sample_set(&m_sample_set, sample);
            Distributions::FixedNormalInverseWishart<N> posterior = m_niw.posterior(posterior_sample_set); // calculate as if sample was part of the set
            return posterior;
        }

        inline uint32_t nSamples() const {return m_sample_set.size();}

        void serialize(Serializer& szr) const {
            YAML::Emitter& out = szr.get();

            std::size_t sample_i = 0;
            for (const auto& sample : m_sample_set.getSamples()) {
                out << YAML::Key << sample_i++ << YAML::Value << YAML::BeginSeq;
                for (std::size_t i = 0; i < N; ++i) {
                    out << sample(i);
                }
                out << YAML::EndSeq;
            }
        }

        void deserialize(const Deserializer& dszr) {
            const YAML::Node& data = dszr.get();
            std::map<uint32_t, std::vector<float>> sample_set = data.as<std::map<uint32_t, std::vector<float>>>();
            for (auto&[_, sample_vec] : sample_set) {
                ASSERT(sample_vec.size() == N, "Sample dimension does not match");
                Eigen::Matrix<float, N, 1> sample;
                for (std::size_t d = 0; d < N; ++d) {
                    sample(d) = sample_vec[d];
                }
                m_sample_set.add(sample);
            }
        }

    private:
        Distributions::FixedNormalInverseWishart<N> m_niw;
        SampleSet<Eigen::Matrix<float, N, 1>> m_sample_set;
};

}
}