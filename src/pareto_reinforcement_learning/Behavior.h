#pragma once

#include "TaskPlanner.h"

namespace PRL {

    template <uint32_t M>
    class MultivariateCost {
        public:
            using CostVector = TP::Containers::FixedArray<M, float>;
        public:
            static constexpr uint32_t size() {return M;}
    };

    template <uint32_t M>
    class JointCostArray : public MultivariateCost<M>, public TP::ML::UCB {
        public:
            JointCostArray(float confidence) 
                : TP::ML::UCB(confidence)
            {}

            CostVector getRectifiedUCBVector(uint32_t state_visits) const {
                CostVector cv;
                auto mean = TP::Stats::E(m_updater.getEstimateNormal());
                for (uint32_t i = 0; i < M; ++i) {
                    cv[i] = m_ucb.getRectifiedCost(mean[i], state_visits);
                }
                return cv;
            }

            void pull(const CostVector& sample) {
                TP::ML::UCB::pull(); 
                m_updater.update(TP::convert<float, M>(sample));
            }
        
            inline TP::Stats::Distributions::FixedMultivariateNormal<M> getEstimateDistribution() const {
                return m_updater.getEstimateNormal();
            }

            inline uint32_t nSamples() const {return m_updater.nSamples();}

        private:
            TP::Stats::MultivariateGaussianUpdater<M> m_updater;
    };

    template <uint32_t M>
    class IndependentCostArray : public MultivariateCost<M> {
        public:
            IndependentCostArray(float confidence)
                : m_ucb(confidence)
            {}

            CostVector getRectifiedUCBVector(uint32_t state_visits) const {
                CostVector cv;
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

            void pull(const CostVector& sample) { 
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

    struct RewardBehavior {
        RewardBehavior(float confidence) 
            : ucb(confidence)
        {}

        float getUCB(uint32_t n_tasks_completed) const {
            //LOG("Reward ucb: " << ucb.getReward(getEstimateMean(), n_tasks_completed) << " with " << n_tasks_completed << " completed tasks");
            return ucb.getReward(getEstimateMean(), n_tasks_completed);
        }

        float getEstimateMean() const {
            return TP::Stats::E(updater.getEstimateNormal());
        }

        void pull(float sample) {
            ucb.pull();
            updater.update(sample);
        }

        TP::Stats::GaussianUpdater updater;
        TP::ML::UCB ucb;
    };

}