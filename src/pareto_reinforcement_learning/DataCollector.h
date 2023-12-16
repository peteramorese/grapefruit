#pragma once

#include "Grapefruit.h"

#include "Regret.h"

namespace PRL {

template <uint64_t N>
class DataCollector {
    public:
        using SymbolicProductGraph = GF::DiscreteModel::SymbolicProductAutomaton<
            GF::DiscreteModel::TransitionSystem, 
            GF::FormalMethods::DFA, 
            GF::DiscreteModel::ModelEdgeInheritor<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA>>;

        using BehaviorHandlerType = BehaviorHandler<SymbolicProductGraph, N>;

        using PathSolution = GF::GraphSearch::PathSolution<
            typename SearchProblem<N, BehaviorHandlerType>::node_t, 
            typename SearchProblem<N, BehaviorHandlerType>::edge_t>;

        using TrajectoryDistribution = GF::Stats::Distributions::FixedMultivariateNormal<N>;

        using Plan = GF::Planner::Plan<SearchProblem<N, BehaviorHandlerType>>;

        struct Instance {
            public:
                Instance(DataCollector* super)
                    : m_super(super)
                {
                    for (uint32_t i = 0; i < N; ++i) {
                        cost_sample[i] = 0.0f;
                    }
                }

                std::vector<PathSolution> paths;
                GF::ParetoFront<typename SearchProblem<N, BehaviorHandlerType>::cost_t> ucb_pf;
                std::vector<TrajectoryDistribution> trajectory_distributions;
                uint32_t selected_plan_index = 0;
                GF::Containers::FixedArray<N, float> cost_sample;

                float getRegret() const {
                    ASSERT(static_cast<bool>(m_super->m_regret_handler), "No regret handler was given");
                    ASSERT(m_regret.second, "Cannot access regret data for unfinished instance");
                    return m_regret.first;
                }

                float getBias() const {
                    ASSERT(static_cast<bool>(m_super->m_regret_handler), "No regret handler was given");
                    ASSERT(m_regret.second, "Cannot access regret data for unfinished instance");
                    return m_bias;
                }
            private:
                void setRegret(float regret) {m_regret = {regret, true};}
                void setBias(float bias) {m_bias = bias;}
                std::pair<float, bool> m_regret = {0.0f, false};
                float m_bias;
                
                DataCollector* m_super;
                friend class DataCollector;
        };

    public:

    public:
        DataCollector(const std::shared_ptr<SymbolicProductGraph>& product, 
            const TrajectoryDistribution& preference, 
            const std::shared_ptr<Regret<SymbolicProductGraph, N>>& regret_handler = nullptr)
            : m_product(product)
            , m_preference(preference)
            , m_regret_handler(regret_handler)
            , m_buffer_instance(this)
        {
            //for (uint32_t i = 0; i < N; ++i) {
            //    m_cumulative_cost[i] = 0.0f;
            //}
        }

        // Fill instance data in place
        Instance& getCurrentInstance() {return m_buffer_instance;}
        
        // Access a previous instance's data
        const Instance& getInstance(uint32_t i) const {
            ASSERT(i < m_instances.size(), "Index out of range for accessing previous instance");
            return m_instances[i];
        }

        // Finish filling the current instance data, push the data back and reset the in-place buffer
        void finishInstance() {
            // Add the cumulative sampled cost
            m_cumulative_cost += m_buffer_instance.cost_sample;

            // Add the number of steps
            m_steps += m_buffer_instance.paths[m_buffer_instance.selected_plan_index].edge_path.size();

            // If a regret handler was given, calculate regret
            if (m_regret_handler) {
                SymbolicProductGraph::node_t starting_node = m_buffer_instance.paths[0].node_path[0]; // first node of the first path
                m_buffer_instance.setRegret(m_regret_handler->getRegret(starting_node, m_buffer_instance.cost_sample));
                m_buffer_instance.setBias(m_regret_handler->getBias(starting_node, m_buffer_instance.trajectory_distributions));
            }

            // Move the buffer instance into the instance array and reset the buffer
            m_instances.push_back(std::move(m_buffer_instance));
            m_buffer_instance = Instance(this);
        }

        // Observe cross-instance quantities
        const GF::Containers::FixedArray<N, float>& cumulativeCost() const {return m_cumulative_cost;}
        uint32_t steps() const {return m_steps;}
        uint32_t numInstances() const {return m_instances.size();}
        float cumulativeRegret() const {ASSERT(m_regret_handler, "No regret handler was given"); return m_cumulative_regret;}
        GF::Containers::FixedArray<N, float> avgCostPerInstance() const {
            GF::Containers::FixedArray<N, float> avg;
            for (uint32_t i = 0; i < N; ++i)
                avg[i] = m_cumulative_cost[i] / static_cast<float>(m_instances.size());
            return avg;
        }

        void serialize(GF::Serializer& szr, bool exclude_plans = false) const {
            static_assert(N == 2, "Does not support serialization of more than two cost objectives");

            YAML::Emitter& out = szr.get();
            out << YAML::Key << "PRL Preference Mean";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.mu(0) << m_preference.mu(1);
            out << YAML::EndSeq;
            out << YAML::Key << "PRL Preference Covariance";
            out << YAML::Value << YAML::BeginSeq;
            // Cost (1) is x axis, reward (0) is y axis
            out << m_preference.Sigma(0, 0) << m_preference.Sigma(0, 1) << m_preference.Sigma(1, 1);
            out << YAML::EndSeq;
            out << YAML::Key << "Instances" << YAML::Value << m_instances.size();


            LOG("Serializing " << m_instances.size() << " instances");

            float cumulative_regret = 0.0f;

            for (uint32_t i = 0; i < m_instances.size(); ++i) {
                const Instance& instance = m_instances[i];
                
                // Instance index
                out << YAML::Key << "Instance " + std::to_string(i); 
                out << YAML::Value << YAML::BeginMap;

                if (!exclude_plans) {
                    // Chosen plan index
                    out << YAML::Key << "Chosen Plan" << YAML::Value << "Candidate Plan " + std::to_string(instance.selected_plan_index);

                    // Serialize each plan and associated data
                    for (uint32_t i = 0; i < instance.paths.size(); ++i) {
                        std::string title = "Candidate Plan " + std::to_string(i);
                        out << YAML::Key << title << YAML::Value << YAML::BeginMap;
                        Plan plan(instance.paths[i], instance.ucb_pf[i], m_product, true);
                        plan.serialize(szr, title);

                        out << YAML::Key << "Plan Mean Mean" << YAML::Value << YAML::BeginSeq;
                        out << instance.trajectory_distributions[i].mu(0) << instance.trajectory_distributions[i].mu(1);
                        out << YAML::EndSeq;

                        out << YAML::Key << "Plan Mean Covariance" << YAML::Value << YAML::BeginSeq;
                        out << instance.trajectory_distributions[i].Sigma(0, 0) << instance.trajectory_distributions[i].Sigma(0, 1) << instance.trajectory_distributions[i].Sigma(1, 1);
                        out << YAML::EndSeq;

                        out << YAML::Key << "Plan Pareto UCB" << YAML::Value << YAML::BeginSeq;
                        out << instance.ucb_pf[i][0] << instance.ucb_pf[i][1];
                        out << YAML::EndSeq;

                        out << YAML::EndMap;
                    }

                }

                // Cumulative cost sample for the chosen plan
                out << YAML::Key << "Sample" << YAML::Value << YAML::BeginSeq;
                out << instance.cost_sample[0] << instance.cost_sample[1] << YAML::EndSeq;

                // Regret
                float instance_regret = instance.getRegret();
                out << YAML::Key << "Regret" << YAML::Value << instance_regret;

                // Cumulative Regret
                cumulative_regret += instance_regret;
                out << YAML::Key << "Cumulative Regret" << YAML::Value << cumulative_regret;

                // Bias
                out << YAML::Key << "Bias" << YAML::Value << instance.getBias();

                out << YAML::EndMap;
            }
        }

    private:
        std::shared_ptr<SymbolicProductGraph> m_product;
        std::vector<Instance> m_instances;
        TrajectoryDistribution m_preference;
        std::shared_ptr<Regret<SymbolicProductGraph, N>> m_regret_handler;

        Instance m_buffer_instance;

        GF::Containers::FixedArray<N, float> m_cumulative_cost;
        uint32_t m_steps = 0u;
        float m_cumulative_regret = 0.0f;
};

}