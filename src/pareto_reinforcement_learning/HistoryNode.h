#pragma once

namespace PRL {

template <typename BASE_NODE_T>
struct StepHistoryNode {
    StepHistoryNode() = delete;
    StepHistoryNode(uint32_t step_, BASE_NODE_T base_node_) : step(step_), base_node(base_node_) {}

    uint32_t step;
    BASE_NODE_T base_node;

    operator BASE_NODE_T() const {return base_node;}
};

}