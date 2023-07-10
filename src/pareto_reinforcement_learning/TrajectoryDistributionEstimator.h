#pragma once

#include "Grapefruit.h"

namespace PRL {

template <std::size_t N>
class TrajectoryDistributionConvolver {
    public:
        TrajectoryDistributionConvolver() = default;
        TrajectoryDistributionConvolver(std::size_t capacity) 
        {
            m_individual_distributions.reserve(capacity);
        }

        void add(const GF::Stats::Distributions::FixedNormalInverseWishart<N>& niw) {
            /* NIW's must be persistent */
            m_mvn.convolveWith(GF::Stats::MomentMatch::niw2mvn(niw));
            //LOG("mm mvn mean \n" << GF::Stats::MomentMatch::niw2mvn(niw).mu);
            m_individual_distributions.push_back(niw);
        }

        inline const GF::Stats::Distributions::FixedMultivariateNormal<GF::Stats::Distributions::FixedNormalInverseWishart<N>::uniqueElements()>& getConvolutedEstimateMVN() const {return m_mvn;}

        const std::vector<GF::Stats::Distributions::FixedNormalInverseWishart<N>>& getIndividualDistributions() const {return m_individual_distributions;}

        std::vector<GF::Stats::Distributions::FixedMultivariateNormal<N>> getIndividualEstimateDistributions() const {
            std::vector<GF::Stats::Distributions::FixedMultivariateNormal<N>> individual_est_dists;
            individual_est_dists.reserve(m_individual_distributions.size());
            for (auto niw : m_individual_distributions) {
                GF::Stats::Distributions::FixedMultivariateT<N> mean_marginal = niw.meanMarginal();
                GF::Stats::Distributions::FixedInverseWishart<N> covariance_marginal = GF::Stats::minimalWishartToWishart(niw.covarianceMarginal());

                // Certainty equivalence estimate:
                individual_est_dists.emplace_back(GF::Stats::E(mean_marginal), GF::Stats::E(covariance_marginal));
            }
            return individual_est_dists;
        }

    protected:
        GF::Stats::Distributions::FixedMultivariateNormal<GF::Stats::Distributions::FixedNormalInverseWishart<N>::uniqueElements()> m_mvn;
        std::vector<GF::Stats::Distributions::FixedNormalInverseWishart<N>> m_individual_distributions;
};


template <std::size_t N>
class TrajectoryDistributionUpdaters : public TrajectoryDistributionConvolver<N> {
    public:
        TrajectoryDistributionUpdaters() = default;
        TrajectoryDistributionUpdaters(std::size_t capacity) 
            : TrajectoryDistributionConvolver<N>(capacity)
        {
            m_individual_updaters.reserve(capacity);
        }

        void add(const GF::Stats::MultivariateGaussianUpdater<N>& updater) {
            /* Updaters must be persistent */
            //LOG("mm niw mean: \n" << updater.dist().mu);
            TrajectoryDistributionConvolver<N>::add(updater.dist());
            //PAUSE;
            m_individual_updaters.push_back(&updater);
        }

        inline const std::vector<const GF::Stats::MultivariateGaussianUpdater<N>*>& getIndividualUpdaters() const {return m_individual_updaters;}

    private:
        std::vector<const GF::Stats::MultivariateGaussianUpdater<N>*> m_individual_updaters;
};

}