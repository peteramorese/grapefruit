#pragma once

#include "Grapefruit.h"

#include "models/GridWorldAgent.h"

namespace PRL {

struct RandomRegionSpec {
    std::string label;
    std::string proposition;
    std::string color;
    uint32_t n_cells;
};

template <uint64_t N>
struct RandomGridWorldProperties {
    uint32_t n_x;
    uint32_t n_y;
    uint32_t n_obstacle_cells = 0;
    std::vector<RandomRegionSpec> region_specs;
    GF::Containers::FixedArray<N, float> true_mean_lower_bound;
    GF::Containers::FixedArray<N, float> true_mean_upper_bound;
    GF::Containers::FixedArray<N, float> estimate_mean_lower_bound;
    GF::Containers::FixedArray<N, float> estimate_mean_upper_bound;
};

template <uint64_t N>
class RandomGridWorldGenerator {
    public:
        using EdgeInheritor = GF::DiscreteModel::ModelEdgeInheritor<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA>;
        using SymbolicGraph = GF::DiscreteModel::SymbolicProductAutomaton<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA, EdgeInheritor>;
        using BehaviorHandlerType = BehaviorHandler<SymbolicGraph, N>;

        struct Targets {
            GF::DiscreteModel::GridWorldAgentProperties props;
            std::shared_ptr<GF::DiscreteModel::TransitionSystem> ts;
            std::shared_ptr<SymbolicGraph> product;
            std::shared_ptr<TrueBehavior<SymbolicGraph, N>> true_behavior;
            std::shared_ptr<BehaviorHandlerType> behavior_handler;
        };

    public:
        static RandomGridWorldProperties<N> deserializeConfig(const std::string& filepath) {
            RandomGridWorldProperties<N> props;

            YAML::Node data;
            try {
                data = YAML::LoadFile(filepath);

                props.n_x = data["Grid X"].as<uint32_t>();
                props.n_y = data["Grid Y"].as<uint32_t>();
                std::vector<std::string> region_labels = data["Region Labels"].as<std::vector<std::string>>();

                props.n_obstacle_cells = data["Obstacles"].as<uint32_t>();

                for (const auto& label : region_labels) {
                    YAML::Node region_node = data[label]; 
                    RandomRegionSpec spec{label, label, "red", region_node["N Cells"].as<uint32_t>()};
                    props.region_specs.push_back(std::move(spec));
                }

                auto getMeanVec = [&data](const std::string& key, GF::Containers::FixedArray<N, float>& array) {
                    std::vector<float> vec = data[key].as<std::vector<float>>();
                    ASSERT(vec.size() == N, "Number elements does not match dimension for key " << key);
                    for (uint32_t i = 0; i < N; ++i) {
                        ASSERT(vec[i] >= 0.0f, "Bound must be positive");
                        array[i] = vec[i];
                    }

                };

                getMeanVec("True Mean Lower Bound", props.true_mean_lower_bound);
                getMeanVec("True Mean Upper Bound", props.true_mean_upper_bound);
                getMeanVec("Estimate Mean Lower Bound", props.estimate_mean_lower_bound);
                getMeanVec("Estimate Mean Upper Bound", props.estimate_mean_upper_bound);

            } catch (YAML::ParserException e) {
                ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
            }
            return props;
        }

        static Targets generate(const RandomGridWorldProperties<N>& random_model_props, const std::vector<GF::FormalMethods::DFAptr>& dfas, float confidence) {
            Targets targets;
            
            // Generate the grid world properties
            targets.props.n_x = random_model_props.n_x;
            targets.props.n_y = random_model_props.n_y;
            
            // Assume starting cell is (0, 0)
            targets.props.init_coordinate_x = 0;
            targets.props.init_coordinate_y = 0;

            uint32_t n_cells = random_model_props.n_x * random_model_props.n_y;

            // Determine the number of cells that are obstacles or regions
            uint32_t non_default_cells = random_model_props.n_obstacle_cells;
            for (const auto& region_spec : random_model_props.region_specs) {
                non_default_cells += region_spec.n_cells;
            }
            ASSERT(non_default_cells < n_cells, "Number of obstacle and region cells exceeds total number of cells in grid");

            GF::Containers::SizedArray<std::size_t> graph_sizes = {random_model_props.n_x, random_model_props.n_y};

            // Keep track of which cells are occupied
            std::vector<bool> occupied(n_cells, false);

            // Randomly make obstacles
            for (uint32_t i = 0; i < random_model_props.n_obstacle_cells; ++i) {
                uint32_t wrapped_cell = GF::RNG::srandi(1, n_cells);
                for (uint32_t i = wrapped_cell; i < n_cells; ++i) {
                    if (!occupied[i]) {
                        occupied[i] = true;
                        GF::Containers::SizedArray<uint32_t> unwrapped_cell = GF::AugmentedNodeIndex::unwrap(i, graph_sizes);
                        GF::DiscreteModel::Obstacle obs{std::string(), unwrapped_cell[0], unwrapped_cell[0], unwrapped_cell[1], unwrapped_cell[1]};
                        targets.props.environment.obstacles.push_back(std::move(obs));
                        --n_cells;
                        break;
                    }
                }
            }

            // Randomly make proposition regions
            for (const auto& region_spec : random_model_props.region_specs) {
                for (uint32_t i = 0; i < region_spec.n_cells; ++i) {
                    uint32_t wrapped_cell = GF::RNG::srandi(0, n_cells);
                    for (uint32_t i = wrapped_cell; i < n_cells; ++i) {
                        if (!occupied[i]) {
                            occupied[i] = true;
                            GF::Containers::SizedArray<uint32_t> unwrapped_cell = GF::AugmentedNodeIndex::unwrap(i, graph_sizes);
                            GF::DiscreteModel::RectangleGridWorldRegion region;
                            region.label = region_spec.label;
                            region.proposition = region_spec.proposition;
                            region.color = region_spec.color;
                            region.lower_left_x = unwrapped_cell[0];
                            region.upper_right_x = unwrapped_cell[0];
                            region.lower_left_y = unwrapped_cell[1];
                            region.upper_right_y = unwrapped_cell[1];
                            targets.props.environment.regions.push_back(std::move(region));
                            --n_cells;
                            break;
                        }
                    }
                }
            }

            // Generate the transition system
            targets.ts = GF::DiscreteModel::GridWorldAgent::generate(targets.props);

            GF::FormalMethods::Alphabet combined_alphbet;
            for (const auto& dfa : dfas) {
                combined_alphbet = combined_alphbet + dfa->getAlphabet();
            }
            targets.ts->addAlphabet(combined_alphbet);

            targets.product = std::make_shared<SymbolicGraph>(targets.ts, dfas);
            targets.true_behavior = std::make_shared<TrueBehavior<SymbolicGraph, N>>(targets.product, GF::Stats::Distributions::FixedMultivariateNormal<N>());
            targets.behavior_handler = std::make_shared<BehaviorHandlerType>(targets.product, 1, confidence);

            // For each node and action, make a distribution for the true behavior as well as the estimated behavior
            for (auto node : targets.ts->nodes()) {
                for (const auto& action : {"left", "right", "up", "down", "stay"}) {
                    GF::Stats::Distributions::FixedMultivariateNormal<N> true_distribution;
                    Eigen::Matrix<float, N, 1>& niw_mu = targets.behavior_handler->getElement(node, action).getUpdater().priorDist().mu;
                    for (uint32_t i = 0; i < N; ++i) {
                        true_distribution.mu(i) = GF::RNG::srandf(random_model_props.true_mean_lower_bound[i], random_model_props.true_mean_upper_bound[i]);
                        true_distribution.Sigma(i,i) = 0.1f * true_distribution.mu(i);
                        niw_mu(i) = GF::RNG::srandf(random_model_props.estimate_mean_lower_bound[i], random_model_props.estimate_mean_upper_bound[i]);
                    }
                    targets.true_behavior->getElement(node, action) = GF::Stats::Distributions::FixedMultivariateNormalSampler<N>(true_distribution);
                }
            }

            return targets;
        }

        static std::shared_ptr<TrueBehavior<SymbolicGraph, N>> makeRandomTrueBehavior(const RandomGridWorldProperties<N>& random_model_props, const std::shared_ptr<SymbolicGraph>& product) { 
            // For each node and action, make a distribution for the true behavior as well as the estimated behavior
            std::shared_ptr<TrueBehavior<SymbolicGraph, N>> true_behavior = std::make_shared<TrueBehavior<SymbolicGraph, N>>(product, GF::Stats::Distributions::FixedMultivariateNormal<N>());
            for (auto node : product->getModel().nodes()) {
                for (const auto& action : {"left", "right", "up", "down", "stay"}) {
                    GF::Stats::Distributions::FixedMultivariateNormal<N> true_distribution;
                    //Eigen::Matrix<float, N, 1>& niw_mu = targets.behavior_handler->getElement(node, action).getUpdater().priorDist().mu;
                    for (uint32_t i = 0; i < N; ++i) {
                        true_distribution.mu(i) = GF::RNG::srandf(random_model_props.true_mean_lower_bound[i], random_model_props.true_mean_upper_bound[i]);
                        true_distribution.Sigma(i,i) = 0.1f * true_distribution.mu(i);
                        //niw_mu(i) = GF::RNG::srandf(random_model_props.estimate_mean_lower_bound[i], random_model_props.estimate_mean_upper_bound[i]);
                    }
                    true_behavior->getElement(node, action) = GF::Stats::Distributions::FixedMultivariateNormalSampler<N>(true_distribution);
                }
            }
            return true_behavior;
        }
};

}
