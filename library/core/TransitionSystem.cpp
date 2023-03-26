#include "TransitionSystem.h"


namespace TP {
namespace DiscreteModel {

    // TransitionSystemGenerator

    std::shared_ptr<TransitionSystem> TransitionSystemGenerator::generate(TransitionSystemProperties& props) {
        std::shared_ptr<TransitionSystem> ts = std::make_shared<TransitionSystem>(props.ss);

        // Copy the propositions into the transition system
        for (const auto& prop : props.propositions) {
            ts->addProposition(prop);
        }

        // Hold all of the unique states, mapping to the index inside of the final state container
        std::unordered_map<State, uint32_t> unique_state_container;

        // Add initial state
        ts->m_node_container.tryInsert(props.init_state);

        // Generate all possible states:
        std::vector<State> all_states;
        props.init_state.getStateSpace()->generateAllStates(all_states);

        uint32_t state_ind = 0;
        while (state_ind < ts->m_node_container.size()) {
            for (const auto& dst_state : all_states) {
                State src_state = ts->m_node_container[state_ind]; // pull by copy since the reference may be invalidated
                if (!(src_state == dst_state)) {
                    for (auto& cond : props.conditions) {
                        if (cond.evaluate(src_state, dst_state)) {
                            ts->Graph<TransitionSystemLabel>::connect(state_ind, ts->m_node_container.tryInsert(dst_state).first, TransitionSystemLabel(cond.getActionCost(), cond.getActionLabel()));
                        }
                    }
                }
            }
            ++state_ind;
        }
        ts->addAlphabet(props.alphabet);
        return ts;
    }

    // TransitionSystem

    bool TransitionSystem::parseAndObserve(const State& state, const std::string& observation) const {

        if (observation == "1") return true;

        std::vector<std::string> proposition_stack(1);
        std::vector<bool> bool_stack(1);
        std::vector<bool> negate_next_stack = {false};
        std::vector<char> prev_operator_stack = {'\0'};
        bool collapse = false;

        for (uint32_t i = 0; i < observation.size(); ++i) {
            char character = observation[i];
            bool sub_eval = false;

            switch (character) {
                case '!': negate_next_stack.back() = !negate_next_stack.back(); continue;
                case ' ': continue;
                case '(': 
                    proposition_stack.push_back("");
                    bool_stack.push_back(true);
                    negate_next_stack.push_back(false);
                    prev_operator_stack.push_back('\0');
                    continue;
                case ')':
                    ASSERT(bool_stack.size() > 1, "Found ')' with out opening bracket");
                    if (!collapse) sub_eval = evaluateAllPropositionsAtState(proposition_stack.back(), state);
                    if (negate_next_stack.back()) sub_eval = !sub_eval;
                    proposition_stack.back().clear();
                    switch (prev_operator_stack.back()) {
                        case '\0': bool_stack.back() = sub_eval; break;
                        case '|': bool_stack.back() = bool_stack.back() || sub_eval; break;
                        case '&': bool_stack.back() = bool_stack.back() && sub_eval; break;
                    }

                    negate_next_stack.pop_back();
                    prev_operator_stack.pop_back();
                    proposition_stack.pop_back();
                    sub_eval = bool_stack.back();
                    bool_stack.pop_back();
                    collapse = true;
            }
            bool at_end = i == observation.size() - 1;
            if (character == '|' || character == '&' || at_end) {
                if (at_end) proposition_stack.back().push_back(character);
                if (!collapse) sub_eval = evaluateAllPropositionsAtState(proposition_stack.back(), state);
                collapse = false;
                if (negate_next_stack.back()) {
                    sub_eval = !sub_eval;
                    negate_next_stack.back() = false;
                }
                proposition_stack.back().clear();
                switch (prev_operator_stack.back()) {
                    case '\0': bool_stack.back() = sub_eval; break;
                    case '|': bool_stack.back() = bool_stack.back() || sub_eval; break;
                    case '&': bool_stack.back() = bool_stack.back() && sub_eval; break;
                }
                prev_operator_stack.back() = character;
            } else {
                proposition_stack.back().push_back(character);
            }
        }
        return bool_stack.front();
    }

    void TransitionSystem::addAlphabet(const FormalMethods::Alphabet& alphabet) {
        if (alphabet.size() == 0) return;
        m_observation_container.resize(size());
        for (Node state_ind = 0; state_ind < m_node_container.size(); ++state_ind) {
            addObservationsToNode(state_ind, alphabet);
        }
    }

    void TransitionSystem::addObservationsToNode(Node node, const FormalMethods::Alphabet& alphabet) {
        for (const auto& observation : alphabet) {
            //LOG("parsing and observing state: " << m_node_container[node].to_str() << " w obs: " << observation);
            if (parseAndObserve(m_node_container[node], observation)) {
                //LOG("found!!");
                m_observation_container.addObservationToNode(node, observation);
            }
        }
    }

    void TransitionSystem::print() const {
        LOG("Printing transition system");
        uint32_t node_ind = 0;
        for (const auto& list : m_graph) {
            PRINT_NAMED("State " << node_ind, m_node_container[node_ind].to_str() << " is connected to:");
            ++node_ind;
            for (uint32_t i=0; i < list.forward.size(); ++i) {
                PRINT_NAMED("    - child State " << list.forward.nodes[i], m_node_container[list.forward.nodes[i]].to_str() << " with edge (action: " << list.forward.edges[i].action << ", cost: " << list.forward.edges[i].cost << ")");
            }
        }
    }

} // namespace DiscreteModel
} // namespace TP
