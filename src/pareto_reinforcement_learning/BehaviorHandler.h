#pragma once

#include <memory>

#include "Grapefruit.h"

#include "HistoryNode.h"
#include "Storage.h"
#include "Behavior.h"

namespace PRL {

    template <class SYMBOLIC_GRAPH_T, uint64_t N>
    class BehaviorHandler : public Storage<JointCostArray<N>> {
        public:
            using CostVector = GF::Containers::FixedArray<N, float>;
        public:
            BehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, uint8_t completed_tasks_horizon, float ucb_confidence)
                : Storage<JointCostArray<N>>(JointCostArray<N>(ucb_confidence))
                , m_product(product)
                , m_completed_tasks_horizon(completed_tasks_horizon)
            {}

            BehaviorHandler(const std::shared_ptr<SYMBOLIC_GRAPH_T>& product, uint8_t completed_tasks_horizon, float ucb_confidence, const Eigen::Matrix<float, N, 1>& default_mean)
                : Storage<JointCostArray<N>>(JointCostArray<N>(ucb_confidence, default_mean))
                , m_product(product)
                , m_completed_tasks_horizon(completed_tasks_horizon)
            {}
            
            static constexpr std::size_t size() noexcept {return N;}
            uint8_t getCompletedTasksHorizon() const {return m_completed_tasks_horizon;}

            CostVector getCostVector(const TaskHistoryNode<GF::WideNode>& src_node, const TaskHistoryNode<GF::WideNode>& dst_node, const GF::DiscreteModel::Action& action) {
                GF::Node src_model_node = m_product->getUnwrappedNode(src_node.base_node).ts_node;
                return this->getElement(src_model_node, action).getRectifiedUCBVector(m_state_visits);
            }

            inline const std::shared_ptr<SYMBOLIC_GRAPH_T>& getProduct() const {return m_product;}

            void visit(const TaskHistoryNode<GF::WideNode>& node, const GF::DiscreteModel::Action& action, const CostVector& sample) {
                GF::Node src_model_node = m_product->getUnwrappedNode(node.base_node).ts_node;
                ++m_state_visits;
                this->getElement(src_model_node, action).pull(sample);
            }

            void serialize(GF::Serializer& szr) const {
                YAML::Emitter& out = szr.get();
                out << YAML::Key << "Behavior Handler" << YAML::Value;
                out << YAML::BeginSeq;
                for (auto&[nap, joint_cost_array] : this->m_node_action_pair_elements) {
                    out << YAML::BeginMap;
                    out << YAML::Key << "Node" << YAML::Value << nap.node;
                    out << YAML::Key << "Action" << YAML::Value << nap.action;
                    out << YAML::Key << "Updater" << YAML::Value << YAML::BeginMap;
                    joint_cost_array.serialize(szr);
                    out << YAML::EndMap << YAML::EndMap;
                }
                out << YAML::EndSeq;
            }
            
            void deserialize(const GF::Deserializer& dszr) {

                const YAML::Node& node = dszr.get();
                YAML::Node behavior_handler_node = node["Behavior Handler"];
                for (YAML::iterator it = behavior_handler_node.begin(); it != behavior_handler_node.end(); ++it) {
                    const YAML::Node& nap_node = *it;
                    JointCostArray<N> joint_cost_array;
                    GF::Deserializer updater_dszr(nap_node["Updater"]);
                    joint_cost_array.deserialize(updater_dszr);
                    typename Storage<JointCostArray<N>>::NodeActionPair nap(nap_node["Node"].as<GF::Node>(), nap_node["Action"].as<GF::DiscreteModel::Action>());
                    this->m_node_action_pair_elements.insert(std::make_pair(nap, joint_cost_array));
                }
            }

        private:
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            uint8_t m_completed_tasks_horizon = 1;
            uint32_t m_state_visits = 0;
    };

}