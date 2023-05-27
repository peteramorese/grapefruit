#pragma once

#include "TaskPlanner.h"
#include "BehaviorHandler.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint64_t N>
class TrueBehavior : public Storage<TP::Stats::Distributions::FixedMultivariateNormalSampler<N>> {
    public:
        using Distribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;
        using DistributionSampler = TP::Stats::Distributions::FixedMultivariateNormalSampler<N>;
    public:
        TrueBehavior(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, const Distribution& default_cost_distribution)
            : Storage<DistributionSampler>(TP::Stats::Distributions::FixedMultivariateNormalSampler<N>(default_cost_distribution))
            , m_product(product)
        {}

        TP::Containers::FixedArray<N, float> sample(TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action) {
            TP::Containers::FixedArray<N, float> rectified_sample;
            TP::Node src_model_node = m_product->getUnwrappedNode(src_node).ts_node;
            Eigen::Matrix<float, N, 1> sample = TP::RNG::mvnrand(this->getElement(src_model_node, action));
            for (uint32_t i = 0; i < N; ++i) {
                rectified_sample[i] = TP::max(sample(i, 1), 0.0f);
            }
            return rectified_sample;
        }

    protected:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;

};

template <uint64_t N>
class GridWorldTrueBehavior : public TrueBehavior<
    TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>, N> {
    public:
        using SymbolicProductGraph = TP::DiscreteModel::SymbolicProductAutomaton<
            TP::DiscreteModel::TransitionSystem, 
            TP::FormalMethods::DFA, 
            TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>>;

    public:
        GridWorldTrueBehavior(const std::shared_ptr<SymbolicProductGraph>& product)
            : TrueBehavior<SymbolicProductGraph, N>(product, TP::Stats::Distributions::FixedMultivariateNormal<N>())
        {}

        GridWorldTrueBehavior(const std::shared_ptr<SymbolicProductGraph>& product, const std::string& filepath)
            : TrueBehavior<SymbolicProductGraph, N>(product, TP::Stats::Distributions::FixedMultivariateNormal<N>())
        {
            deserializeConfig(filepath);
        }

        void print() const {
            LOG("True distributions:");
            Eigen::IOFormat fmt(4, 0, ", ", "\n", "     [", "]");
            for (const auto&[nap, sampler] : this->m_node_action_pair_elements) {
                TP::DiscreteModel::State s = this->m_product->getModel().getGenericNodeContainer()[this->m_product->getUnwrappedNode(nap.node).ts_node];
                PRINT_NAMED("State: " << s.to_str() << " action: " << nap.action, "");
                PRINT("   Mean:\n" << sampler.mean().format(fmt));
                PRINT("   Covariance:\n"  << sampler.dist().Sigma.format(fmt));
            }
        }


        void deserializeConfig(const std::string& filepath) {
            TP::DiscreteModel::GridWorldAgentProperties props = TP::DiscreteModel::GridWorldAgent::deserializeConfig(filepath);
            YAML::Node data;
            try {
                data = YAML::LoadFile(filepath);

                ASSERT(data["PRL Cost Objectives"], "Missing cost objectives for PRL");
                std::vector<std::string> objective_labels = data["PRL Cost Objectives"].as<std::vector<std::string>>();
                ASSERT(objective_labels.size() == N, "Number of objectives (" << objective_labels.size() <<") does not match compile-time dimension (" << N << ")");

                // Find the default transition cost:
                ASSERT(data["PRL Default Cost"], "Missing default transition cost for PRL");

                YAML::Node default_cost_node = data["PRL Default Cost"]; 
                std::vector<float> default_mean = default_cost_node["PRL Cost Mean"].as<std::vector<float>>();
                std::vector<float> default_minimal_covariance = default_cost_node["PRL Cost Covariance"].as<std::vector<float>>();
                ASSERT(default_mean.size() == N, "Mean (" << default_mean.size() <<") dimension does not match compile-time dimension (" << N << ")");
                ASSERT(default_minimal_covariance.size() == TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 
                    "Mean (" << default_minimal_covariance.size() <<") dimension does not match compile-time minimal covariance dimension (" << TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements() << ")");

                // Convert to Eigen
                Eigen::Matrix<float, N, 1> default_mean_converted;
                Eigen::Matrix<float, TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 1> default_minimal_cov_converted;
                for (uint32_t i = 0; i < N; ++i)
                    default_mean_converted(i) = default_mean[i];

                for (uint32_t i = 0; i < TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(); ++i)
                    default_minimal_cov_converted(i) = default_minimal_covariance[i];
                
                // Create distribution to be used for each (s,a) in region
                TP::Stats::Distributions::FixedMultivariateNormal<N> default_dist;
                default_dist.mu = default_mean_converted;
                default_dist.setSigmaFromUniqueElementVector(default_minimal_cov_converted);
                this->m_default_na_element.resetDist(default_dist);


                std::vector<std::string> x_labels(props.n_x);
                std::vector<std::string> y_labels(props.n_y);
                for (int i=0; i<props.n_x; ++i) {
                    x_labels[i] = "x" + std::to_string(i);
                }
                for (int i=0; i<props.n_y; ++i) {
                    y_labels[i] = "y" + std::to_string(i);
                }

                for (const auto& region : props.environment.regions) {
                    YAML::Node region_node = data[region.label];

                    if (region_node["PRL Cost Mean"] && region_node["PRL Cost Covariance"]) {
                        // Read in the mean and covariance
                        std::vector<float> mean = region_node["PRL Cost Mean"].as<std::vector<float>>();
                        std::vector<float> minimal_covariance = region_node["PRL Cost Covariance"].as<std::vector<float>>();
                        ASSERT(mean.size() == N, "Mean (" << mean.size() <<") dimension does not match compile-time dimension (" << N << ")");
                        ASSERT(minimal_covariance.size() == TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 
                            "Mean (" << minimal_covariance.size() <<") dimension does not match compile-time minimal covariance dimension (" << TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements() << ")");

                        // Convert to Eigen
                        Eigen::Matrix<float, N, 1> mean_converted;
                        Eigen::Matrix<float, TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 1> minimal_cov_converted;
                        for (uint32_t i = 0; i < N; ++i)
                            mean_converted(i) = mean[i];

                        for (uint32_t i = 0; i < TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(); ++i)
                            minimal_cov_converted(i) = minimal_covariance[i];
                        
                        // Create distribution to be used for each (s,a) in region
                        TP::Stats::Distributions::FixedMultivariateNormal<N> region_dist;
                        region_dist.mu = mean_converted;
                        region_dist.setSigmaFromUniqueElementVector(minimal_cov_converted);

                        for (uint32_t i = region.lower_left_x; i <= region.upper_right_x; ++i) {
                            for (uint32_t j = region.lower_left_y; j <= region.upper_right_y; ++j) {
                                auto ts = this->m_product->getModel();
                                TP::DiscreteModel::State s(ts.getStateSpace().lock().get());	
                                s[TP::DiscreteModel::GridWorldAgent::s_x_coord_label] = x_labels[i];
                                s[TP::DiscreteModel::GridWorldAgent::s_y_coord_label] = y_labels[j];
                                TP::Node model_node = ts.getGenericNodeContainer()[s];
                                for (const auto& outgoing_edge : ts.getOutgoingEdges(model_node)) {
                                    TP::Stats::Distributions::FixedMultivariateNormalSampler<N>& sampler = this->getElement(model_node, outgoing_edge.action);
                                    TP::Stats::Distributions::FixedMultivariateNormal<N> dist = sampler.dist(); // copy out the distribution
                                    LOG("dist: \n" << dist.Sigma);
                                    PAUSE;
                                    dist.convolveWith(region_dist);
                                    sampler.resetDist(dist); // place in new distribution
                                }
                            }
                        }
                    }
                }
            } catch (YAML::ParserException e) {
                ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
            }
        }
        //void compare(const RewardCostBehaviorHandler<SYMBOLIC_GRAPH_T, M>& behavior_handler) const {
        //    for (const auto&[nap, element] : this->m_node_action_pair_elements) {
        //        TP::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[nap.node];

        //        float true_dist_mu = element[0].mu;
        //        float true_dist_sigma2 = element[0].sigma_2;

        //        auto cba = behavior_handler.lookupNAElement(nap.node, nap.action);
        //        auto estimate_dist = cba.getEstimateDistributions()[0];
        //        float estimate_dist_mu = estimate_dist.mu;
        //        float estimate_dist_sigma2 = estimate_dist.sigma_2;

        //        PRINT_NAMED("State: " << s.to_str() << " Action: " << nap.action, 
        //            "Mu: (est: " << estimate_dist_mu << " act: " << true_dist_mu << "), Var: (est: " << estimate_dist_sigma2 << " act: " << true_dist_sigma2 <<"), N samples: " << cba.nSamples());
        //    }
        //}


};



}