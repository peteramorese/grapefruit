#pragma once

#include "core/State.h"
#include "core/Graph.h"

namespace TP {
namespace Planner {

    struct Plan {
        bool success;
        std::vector<Node> node_sequence;
        std::vector<DiscreteModel::State> state_sequence;
        std::vector<Action> action_sequence;
    };
}
}