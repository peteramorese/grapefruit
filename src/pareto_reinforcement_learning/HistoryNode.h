#pragma once

namespace PRL {

template <typename BASE_NODE_T>
struct FancyNode {
    BASE_NODE_T base_node;

    FancyNode(BASE_NODE_T base_node_) : base_node(base_node_) {}
    operator BASE_NODE_T() const {return base_node;}
};

template <typename BASE_NODE_T>
struct StepHistoryNode : public FancyNode<BASE_NODE_T> {
    uint32_t step;

    bool operator<(const StepHistoryNode& other) const {
        if (this->base_node < other.base_node) {
            return true;
        } else if (this->base_node == other.base_node) {
            return step < other.step;
        }
        return false;
    }

    StepHistoryNode() = delete;
    StepHistoryNode(BASE_NODE_T base_node_, uint32_t step_) : FancyNode<BASE_NODE_T>(base_node_), step(step_)  {}
};

template <typename BASE_NODE_T>
struct TaskHistoryNode : public FancyNode<BASE_NODE_T> {
    uint8_t n_completed_tasks;

    bool operator<(const TaskHistoryNode& other) const {
        if (this->base_node < other.base_node) {
            return true;
        } else if (this->base_node == other.base_node) {
            return n_completed_tasks < other.n_completed_tasks;
        }
        return false;
    }


    TaskHistoryNode() = delete;
    TaskHistoryNode(BASE_NODE_T base_node_, uint8_t n_completed_tasks_) : FancyNode<BASE_NODE_T>(base_node_), n_completed_tasks(n_completed_tasks_) {}
};

}
