#pragma once

#include "TaskPlanner.h"

namespace PRL {

    template <uint64_t M>
    class MultivariateCost {
        public:
            using CostVector = TP::Containers::FixedArray<M, float>;
        public:
            static constexpr uint32_t size() {return M;}
    };

    template <uint64_t M>
    class JointCostArray : public MultivariateCost<M>, public TP::ML::UCB {
        public:
            JointCostArray(float confidence) 
                : TP::ML::UCB(confidence)
            {}

            typename MultivariateCost<M>::CostVector getRectifiedUCBVector(uint32_t state_visits) const {
                typename MultivariateCost<M>::CostVector cv;
                auto mean = TP::Stats::E(m_updater.getEstimateNormal());
                for (uint32_t i = 0; i < M; ++i) {
                    cv[i] = getRectifiedCost(mean[i], state_visits);
                }
                return cv;
            }

            void pull(const MultivariateCost<M>::CostVector& sample) {
                TP::ML::UCB::pull(); 
                m_updater.update(sample);
            }
        
            inline TP::Stats::Distributions::FixedMultivariateNormal<M> getEstimateDistribution() const {
                return m_updater.getEstimateNormal();
            }

            inline uint32_t nSamples() const {return m_updater.nSamples();}

            const TP::Stats::MultivariateGaussianUpdater<M>& getUpdater() const {return m_updater;}
            TP::Stats::MultivariateGaussianUpdater<M>& getUpdater() {return m_updater;}

        private:
            TP::Stats::MultivariateGaussianUpdater<M> m_updater;
    };

    template <uint64_t M>
    class IndependentCostArray : public MultivariateCost<M> {
        public:
            IndependentCostArray(float confidence)
                : m_ucb(confidence)
            {}

            typename MultivariateCost<M>::CostVector getRectifiedUCBVector(uint32_t state_visits) const {
                typename MultivariateCost<M>::CostVector cv;
                for (uint32_t i = 0; i < M; ++i) {
                    cv[i] = m_ucb.getRectifiedCost(TP::Stats::E(m_updaters[i].getEstimateNormal()), state_visits);
                }
                return cv;
            }

            TP::Containers::FixedArray<M, TP::Stats::Distributions::Normal> getEstimateDistributions() const {
                TP::Containers::FixedArray<M, TP::Stats::Distributions::Normal> distributions;
                for (uint32_t i = 0; i < M; ++i) {
                    distributions[i] = m_updaters[i].getEstimateNormal();
                }
                return distributions;
            }

            void pull(const MultivariateCost<M>::CostVector& sample) { 
                m_ucb.pull(); 
                for (uint32_t i = 0; i < M; ++i) {
                    m_updaters[i].update(sample[i]);
                }
            }

            inline uint32_t nSamples() const {return m_updaters[0].nSamples();}

        private:
            TP::Containers::FixedArray<M, TP::Stats::GaussianUpdater> m_updaters;
            TP::ML::UCB m_ucb;

    };

}