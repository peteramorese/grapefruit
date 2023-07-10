#pragma once

#include "Grapefruit.h"

namespace PRL {

    template <uint64_t M>
    class MultivariateCost {
        public:
            using CostVector = GF::Containers::FixedArray<M, float>;
        public:
            static constexpr uint32_t size() {return M;}
    };

    template <uint64_t M>
    class JointCostArray : public MultivariateCost<M>, public GF::ML::UCB {
        public:
            JointCostArray() = default;

            JointCostArray(float confidence) 
                : GF::ML::UCB(confidence)
            {}

            JointCostArray(float confidence, const Eigen::Matrix<float, M, 1>& default_mean) 
                : GF::ML::UCB(confidence)
                , m_updater(default_mean)
            {}

            typename MultivariateCost<M>::CostVector getRectifiedUCBVector(uint32_t state_visits) const {
                typename MultivariateCost<M>::CostVector cv;
                auto mean = GF::Stats::E(m_updater.getEstimateNormal());
                for (uint32_t i = 0; i < M; ++i) {
                    cv[i] = getRectifiedCost(mean[i], state_visits);
                }
                return cv;
            }

            void pull(const MultivariateCost<M>::CostVector& sample) {
                GF::ML::UCB::pull(); 
                m_updater.update(sample);
            }
        
            inline GF::Stats::Distributions::FixedMultivariateNormal<M> getEstimateDistribution() const {
                return m_updater.getEstimateNormal();
            }

            inline uint32_t nSamples() const {return m_updater.nSamples();}

            const GF::Stats::MultivariateGaussianUpdater<M>& getUpdater() const {return m_updater;}
            GF::Stats::MultivariateGaussianUpdater<M>& getUpdater() {return m_updater;}

            void serialize(GF::Serializer& szr) {
                YAML::Emitter& out = szr.get();
                out << YAML::Key << "UCB" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Confidence" << YAML::Value << m_confidence;
                out << YAML::Key << "N" << YAML::Value << m_n;
                out << YAML::EndMap;

                out << YAML::Key << "Updater" << YAML::Value << YAML::BeginMap;
                m_updater.serialize(szr);
                out << YAML::EndMap;
            }

            void deserialize(const GF::Deserializer& dszr) {
                const YAML::Node& node = dszr.get();
                
                // Get the UCB data
                YAML::Node ucb_node = node["UCB"];
                this->m_confidence = ucb_node["Confidence"].as<float>();
                this->m_n = ucb_node["N"].as<uint32_t>();

                // Deserialize the guassian updater
                m_updater.deserialize(GF::Deserializer(node["Updater"]));
            }

        private:
            GF::Stats::MultivariateGaussianUpdater<M> m_updater;
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
                    cv[i] = m_ucb.getRectifiedCost(GF::Stats::E(m_updaters[i].getEstimateNormal()), state_visits);
                }
                return cv;
            }

            GF::Containers::FixedArray<M, GF::Stats::Distributions::Normal> getEstimateDistributions() const {
                GF::Containers::FixedArray<M, GF::Stats::Distributions::Normal> distributions;
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
            GF::Containers::FixedArray<M, GF::Stats::GaussianUpdater> m_updaters;
            GF::ML::UCB m_ucb;

    };

}