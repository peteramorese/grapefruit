#pragma once

#include "Grapefruit.h"
#include "BehaviorHandler.h"

namespace PRL {

template <class SYMBOLIC_GRAPH_T, uint64_t N>
class TrueBehavior : public Storage<GF::Stats::Distributions::FixedMultivariateNormalSampler<N>> {
    public:
        using Distribution = GF::Stats::Distributions::FixedMultivariateNormal<N>;
        using DistributionSampler = GF::Stats::Distributions::FixedMultivariateNormalSampler<N>;
        using CostVector = GF::Containers::FixedArray<N, float>;
    public:
        TrueBehavior(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, const Distribution& default_cost_distribution)
            : Storage<DistributionSampler>(GF::Stats::Distributions::FixedMultivariateNormalSampler<N>(default_cost_distribution))
            , m_product(product)
        {}

        GF::Containers::FixedArray<N, float> sample(GF::WideNode src_node, GF::WideNode dst_node, const GF::DiscreteModel::Action& action) {
            GF::Containers::FixedArray<N, float> rectified_sample;
            GF::Node src_model_node = m_product->getUnwrappedNode(src_node).ts_node;
            Eigen::Matrix<float, N, 1> sample = GF::RNG::mvnrand(this->getElement(src_model_node, action));
            for (uint32_t i = 0; i < N; ++i) {
                rectified_sample[i] = GF::max(sample(i), 0.0f);
            }
            return rectified_sample;
        }

        CostVector getCostVector(const TaskHistoryNode<GF::WideNode>& src_node, const TaskHistoryNode<GF::WideNode>& dst_node, const GF::DiscreteModel::Action& action) {
            GF::Node src_model_node = m_product->getUnwrappedNode(src_node).ts_node;
            const GF::Stats::Distributions::FixedMultivariateNormalSampler<N>& sampler = this->getElement(src_model_node, action);
            CostVector ret;
            GF::fromColMatrix<float, N>(sampler.dist().mu, ret);
            return ret;
        }

        void print() const {
            LOG("True distributions:");
            Eigen::IOFormat fmt(4, 0, ", ", "\n", "     [", "]");
            for (const auto&[nap, sampler] : this->m_node_action_pair_elements) {
                GF::DiscreteModel::State s = this->m_product->getModel().getGenericNodeContainer()[this->m_product->getUnwrappedNode(nap.node).ts_node];
                PRINT_NAMED("State: " << s.to_str() << " action: " << nap.action, "");
                PRINT("   Mean:\n" << sampler.mean().format(fmt));
                PRINT("   Covariance:\n"  << sampler.dist().Sigma.format(fmt));
            }
        }

    protected:
        std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;

};

template <uint64_t N>
class GridWorldTrueBehavior : public TrueBehavior<
    GF::DiscreteModel::SymbolicProductAutomaton<
            GF::DiscreteModel::TransitionSystem, 
            GF::FormalMethods::DFA, 
            GF::DiscreteModel::ModelEdgeInheritor<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA>>, N> {
    public:
        using SymbolicProductGraph = GF::DiscreteModel::SymbolicProductAutomaton<
            GF::DiscreteModel::TransitionSystem, 
            GF::FormalMethods::DFA, 
            GF::DiscreteModel::ModelEdgeInheritor<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA>>;

    public:
        GridWorldTrueBehavior(const std::shared_ptr<SymbolicProductGraph>& product)
            : TrueBehavior<SymbolicProductGraph, N>(product, GF::Stats::Distributions::FixedMultivariateNormal<N>())
        {}

        GridWorldTrueBehavior(const std::shared_ptr<SymbolicProductGraph>& product, const std::string& filepath)
            : TrueBehavior<SymbolicProductGraph, N>(product, GF::Stats::Distributions::FixedMultivariateNormal<N>())
        {
            deserializeConfig(filepath);
        }


        void deserializeConfig(const std::string& filepath) {
            GF::DiscreteModel::GridWorldAgentProperties props = GF::DiscreteModel::GridWorldAgent::deserializeConfig(filepath);
            YAML::Node data;
            try {
                data = YAML::LoadFile(filepath);

                bool make_pos_semi_def = data["Square Covariance"] ? data["Square Covariance"].as<bool>() : false;

                ASSERT(data["PRL Cost Objectives"], "Missing cost objectives for PRL");
                std::vector<std::string> objective_labels = data["PRL Cost Objectives"].as<std::vector<std::string>>();
                ASSERT(objective_labels.size() == N, "Number of objectives (" << objective_labels.size() <<") does not match compile-time dimension (" << N << ")");

                // Find the default transition cost:
                ASSERT(data["PRL Default Cost"], "Missing default transition cost for PRL");

                YAML::Node default_cost_node = data["PRL Default Cost"]; 
                std::vector<float> default_mean = default_cost_node["Mean"].as<std::vector<float>>();
                std::vector<float> default_minimal_covariance = default_cost_node["Covariance"].as<std::vector<float>>();
                ASSERT(default_mean.size() == N, "Mean (" << default_mean.size() <<") dimension does not match compile-time dimension (" << N << ")");
                ASSERT(default_minimal_covariance.size() == GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 
                    "Mean (" << default_minimal_covariance.size() <<") dimension does not match compile-time minimal covariance dimension (" << GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements() << ")");

                // Convert to Eigen
                Eigen::Matrix<float, N, 1> default_mean_converted;
                Eigen::Matrix<float, GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 1> default_minimal_cov_converted;
                for (uint32_t i = 0; i < N; ++i)
                    default_mean_converted(i) = default_mean[i];

                for (uint32_t i = 0; i < GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(); ++i)
                    default_minimal_cov_converted(i) = default_minimal_covariance[i];
                
                // Create distribution to be used for each (s,a) in region
                GF::Stats::Distributions::FixedMultivariateNormal<N> default_dist;
                default_dist.mu = default_mean_converted;
                default_dist.setSigmaFromUniqueElementVector(default_minimal_cov_converted);
                if (make_pos_semi_def)
                    default_dist.Sigma = default_dist.Sigma * default_dist.Sigma;
                ASSERT(GF_IS_COV_POS_SEMI_DEF(default_dist.Sigma), "Default Cost Covariance: \n" << default_dist.Sigma <<"\nis not positive semi-definite");

                this->m_default_na_element = GF::Stats::Distributions::FixedMultivariateNormalSampler<N>(default_dist);


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
                        ASSERT(minimal_covariance.size() == GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 
                            "Mean (" << minimal_covariance.size() <<") dimension does not match compile-time minimal covariance dimension (" << GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements() << ")");

                        // Convert to Eigen
                        Eigen::Matrix<float, N, 1> mean_converted;
                        Eigen::Matrix<float, GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 1> minimal_cov_converted;
                        for (uint32_t i = 0; i < N; ++i)
                            mean_converted(i) = mean[i];

                        for (uint32_t i = 0; i < GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(); ++i)
                            minimal_cov_converted(i) = minimal_covariance[i];
                        
                        // Create distribution to be used for each (s,a) in region
                        GF::Stats::Distributions::FixedMultivariateNormal<N> region_dist;
                        region_dist.mu = mean_converted;
                        region_dist.setSigmaFromUniqueElementVector(minimal_cov_converted);
                        if (make_pos_semi_def)
                            region_dist.Sigma = region_dist.Sigma * region_dist.Sigma;

                        ASSERT(GF_IS_COV_POS_SEMI_DEF(region_dist.Sigma), "Region: " << region.label << " Cost Covariance: \n" << region_dist.Sigma <<"\nis not positive semi-definite");
                        
                        for (uint32_t i = region.lower_left_x; i <= region.upper_right_x; ++i) {
                            for (uint32_t j = region.lower_left_y; j <= region.upper_right_y; ++j) {
                                auto ts = this->m_product->getModel();
                                GF::DiscreteModel::State s(ts.getStateSpace().lock().get());	
                                s[GF::DiscreteModel::GridWorldAgent::s_x_coord_label] = x_labels[i];
                                s[GF::DiscreteModel::GridWorldAgent::s_y_coord_label] = y_labels[j];
                                if (!ts.getGenericNodeContainer().contains(s))
                                    continue;
                                GF::Node model_node = ts.getGenericNodeContainer()[s];
                                for (const auto& outgoing_edge : ts.outgoingEdges(model_node)) {
                                    GF::Stats::Distributions::FixedMultivariateNormalSampler<N>& sampler = this->getElement(model_node, outgoing_edge.action);
                                    GF::Stats::Distributions::FixedMultivariateNormal<N> dist = sampler.dist(); // copy out the distribution
                                    dist.convolveWith(region_dist);
                                    sampler = GF::Stats::Distributions::FixedMultivariateNormalSampler<N>(dist);
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
        //        GF::DiscreteModel::State s = m_product->getModel().getGenericNodeContainer()[nap.node];

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