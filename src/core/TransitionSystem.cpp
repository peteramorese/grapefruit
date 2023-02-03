#include "TransitionSystem.h"


namespace DiscreteModel {

    // TransitionSystemGenerator

    const std::shared_ptr<TransitionSystem> TransitionSystemGenerator::generate(const TransitionSystemProperties& props) {
        std::shared_ptr<TransitionSystem> ts = std::make_shared<TransitionSystem>();

        // Copy the propositions into the transition system
        for (const auto& prop : props.propositions) {
            ts->m_propositions[prop.getName()] = prop;
        }

        // Hold all of the unique states, mapping to the index inside of the final state container
        std::unordered_map<State, uint32_t> unique_state_container;

        // Add initial state
        ts->m_state_container.addStateIfUnique(props.init_state);
    }

}
