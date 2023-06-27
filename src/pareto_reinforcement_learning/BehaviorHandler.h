#pragma once

#include <memory>

#include "TaskPlanner.h"

#include "HistoryNode.h"
#include "Storage.h"
#include "Behavior.h"

namespace PRL {

    template <class SYMBOLIC_GRAPH_T, uint64_t N>
    class BehaviorHandler : public Storage<JointCostArray<N>> {
        public:
            using CostVector = TP::Containers::FixedArray<N, float>;
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

            CostVector getCostVector(const TaskHistoryNode<TP::WideNode>& src_node, const TaskHistoryNode<TP::WideNode>& dst_node, const TP::DiscreteModel::Action& action) {
                TP::Node src_model_node = m_product->getUnwrappedNode(src_node.base_node).ts_node;
                return this->getElement(src_model_node, action).getRectifiedUCBVector(m_state_visits);
            }

            inline const std::shared_ptr<SYMBOLIC_GRAPH_T>& getProduct() const {return m_product;}

            void visit(const TaskHistoryNode<TP::WideNode>& node, const TP::DiscreteModel::Action& action, const CostVector& sample) {
                TP::Node src_model_node = m_product->getUnwrappedNode(node.base_node).ts_node;
                ++m_state_visits;
                this->getElement(src_model_node, action).pull(sample);
            }

            void serialize(TP::Serializer& szr) {
                YAML::Emitter& out = szr.get();
                out << YAML::BeginSeq;
                for (auto&[nap, joint_cost_array] : this->m_node_action_pair_elements) {
                    out << YAML::BeginMap;
                    out << YAML::Key << "Node" << YAML::Value << nap.node;
                    out << YAML::Key << "Action" << YAML::Value << nap.action;
                    //out << YAML::Key << ""
                }
                out << YAML::EndSeq;
            }
            
            void deserialize(TP::Deserializer& dszr) {

            }

        private:
            std::shared_ptr<SYMBOLIC_GRAPH_T> m_product;
            uint8_t m_completed_tasks_horizon = 1;
            uint32_t m_state_visits = 0;

    };

}