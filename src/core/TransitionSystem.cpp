#include "TransitionSystem.h"


namespace DiscreteModel {

    // TransitionSystemGenerator

    const std::shared_ptr<TransitionSystem> TransitionSystemGenerator::generate(TransitionSystemProperties& props) {
        std::shared_ptr<TransitionSystem> ts = std::make_shared<TransitionSystem>();

        // Copy the propositions into the transition system
        for (const auto& prop : props.propositions) {
            ts->m_propositions[prop.getName()] = prop;
        }

        // Hold all of the unique states, mapping to the index inside of the final state container
        std::unordered_map<State, uint32_t> unique_state_container;

        // Add initial state
        ts->m_state_container.tryInsert(props.init_state);

        // Generate all possible states:
        std::vector<State> all_states;
        props.init_state.getStateSpace()->generateAllStates(all_states);

        uint32_t state_ind = 0;
        while (state_ind < ts->m_state_container.size()) {
            for (const auto& dst_state : all_states) {
                State src_state = ts->m_state_container[state_ind]; // pull by copy since the reference may be invalidated
                if (!(src_state == dst_state)) {
                    for (auto& cond : props.conditions) {
                        if (cond.evaluate(src_state, dst_state)) {
                            ts->Graph<TransitionSystemLabel>::connect(state_ind, ts->m_state_container.tryInsert(dst_state).first, TransitionSystemLabel(cond.getActionCost(), cond.getActionLabel()));
                        }
                    }
                }
            }
            ++state_ind;
        }
        return ts;
    }

    // TransitionSystem

    void TransitionSystem::print() const {
        LOG("Printing transition system");
        uint32_t node_ind = 0;
        for (const auto& list : m_graph) {
            PRINT_NAMED("State " << node_ind, m_state_container[node_ind].to_str() << " is connected to:");
            ++node_ind;
            for (uint32_t i=0; i < list.forward.size(); ++i) {
                PRINT_NAMED("    - child State " << list.forward.nodes[i], m_state_container[list.forward.nodes[i]].to_str() << " with edge (action: " << list.forward.edges[i].action << ", cost: " << list.forward.edges[i].cost << ")");
            }
        }
    }

}
