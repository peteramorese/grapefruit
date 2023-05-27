#pragma once

#include "TaskPlanner.h"

namespace PRL {

template <std::size_t N>
class TrajectoryDistributionConvolver {
    public:
        void add(const TP::Stats::Distributions::FixedNormalInverseWishart<N>& niw) {
            /* NIW's must be persistent */
            m_mvn.convolveWith(TP::Stats::MomentMatch::niw2mvn(niw));
            //LOG("mm mvn mean \n" << TP::Stats::MomentMatch::niw2mvn(niw).mu);
            m_individual_distributions.push_back(niw);
        }

        inline const TP::Stats::Distributions::FixedMultivariateNormal<TP::Stats::Distributions::FixedNormalInverseWishart<N>::uniqueElements()>& getConvolutedEstimateMVN() const {return m_mvn;}

        const std::vector<TP::Stats::Distributions::FixedNormalInverseWishart<N>>& getIndividualDistributions() const {return m_individual_distributions;}

        std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> getIndividualEstimateDistributions() const {
            std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> individual_est_dists;
            individual_est_dists.reserve(m_individual_distributions.size());
            for (auto niw : m_individual_distributions) {
                TP::Stats::Distributions::FixedMultivariateT<N> mean_marginal = niw.meanMarginal();
                TP::Stats::Distributions::FixedInverseWishart<N> covariance_marginal = TP::Stats::minimalWishartToWishart(niw.covarianceMarginal());

                // Certainty equivalence estimate:
                individual_est_dists.emplace_back(TP::Stats::E(mean_marginal), TP::Stats::E(covariance_marginal));
            }
            return individual_est_dists;
        }

    protected:
        TP::Stats::Distributions::FixedMultivariateNormal<TP::Stats::Distributions::FixedNormalInverseWishart<N>::uniqueElements()> m_mvn;
        std::vector<TP::Stats::Distributions::FixedNormalInverseWishart<N>> m_individual_distributions;
};


template <std::size_t N>
class TrajectoryDistributionUpdaters : public TrajectoryDistributionConvolver<N> {
    public:
        TrajectoryDistributionUpdaters() = default;

        void add(TP::Stats::MultivariateGaussianUpdater<N>& updater) {
            /* Updaters must be persistent */
            //LOG("mm niw mean: \n" << updater.dist().mu);
            TrajectoryDistributionConvolver<N>::add(updater.dist());
            //PAUSE;
            m_individual_updaters.push_back(&updater);
        }

        inline const std::vector<TP::Stats::MultivariateGaussianUpdater<N>*>& getIndividualUpdaters() const {return m_individual_updaters;}

    private:
        std::vector<TP::Stats::MultivariateGaussianUpdater<N>*> m_individual_updaters;
};

}