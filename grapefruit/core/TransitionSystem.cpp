#include "TransitionSystem.h"
#include "tools/Debug.h"

namespace GF {
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
        std::vector<bool> bool_stack(1); // automatically gets set using the null previous operator
        std::vector<bool> negate_next_stack = {false};
        std::vector<char> prev_operator_stack = {'\0'};
        bool collapse = false;

        bool sub_eval = false;
        for (uint32_t i = 0; i < observation.size(); ++i) {
            char character = observation[i];

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
                    sub_eval = false;
                    ASSERT(bool_stack.size() > 1, "Found ')' with out opening bracket");
                    if (!collapse) {
                        sub_eval = evaluatePropositionAtState(proposition_stack.back(), state);
                    }
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
                if (!collapse) sub_eval = evaluatePropositionAtState(proposition_stack.back(), state);
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
            if (parseAndObserve(m_node_container[node], observation)) {
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

    void TransitionSystem::rprint() const {
        LOG("Printing transition system in reverse");
        uint32_t node_ind = 0;
        for (const auto& list : m_graph) {
            PRINT_NAMED("State " << node_ind, m_node_container[node_ind].to_str() << " is connected to:");
            ++node_ind;
            for (uint32_t i=0; i < list.forward.size(); ++i) {
                PRINT_NAMED("    - parent State " << list.backward.nodes[i], m_node_container[list.backward.nodes[i]].to_str() << " with edge (action: " << list.forward.edges[i].action << ", cost: " << list.backward.edges[i].cost << ")");
            }
        }
    }

    void TransitionSystem::serialize(GF::Serializer& szr) const {
        YAML::Emitter& out = szr.get();

        out << YAML::Key << "State Space" << YAML::Value << YAML::BeginMap;
        m_ss->serialize(szr);
        out << YAML::EndMap;


        out << YAML::Key << "Graph" << YAML::Value << YAML::BeginMap;

        uint32_t node_ind = 0;
        for (const auto& list : m_graph) {
            out << YAML::Key << "State " + std::to_string(node_ind) << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "Vars" << YAML::Value;
            m_node_container[node_ind].serialize(szr);
            out << YAML::Key << "Neighbors" << YAML::Value << YAML::BeginMap;
            ++node_ind;
            for (uint32_t i=0; i < list.forward.size(); ++i) {
                out << YAML::Key << "State " + std::to_string(list.forward.nodes[i]) << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Vars" << YAML::Value;
                m_node_container[list.forward.nodes[i]].serialize(szr);
                out << YAML::Key << "Action" << YAML::Value << list.forward.edges[i].action;
                out << YAML::Key << "Cost" << YAML::Value << list.forward.edges[i].cost;
                out << YAML::EndMap;
            }
            out << YAML::EndMap; // Neighbors
            out << YAML::EndMap;
        }
        out << YAML::EndMap; // Graph

        out << YAML::Key << "Propositions" << YAML::Value << YAML::BeginSeq;
        for (const auto&[name, prop] : m_propositions) {
            out << YAML::BeginMap;
            prop.serialize(szr);
            out << YAML::EndMap;
        }
        out << YAML::EndSeq; // Propositions

        out << YAML::EndMap;
    }

    void TransitionSystem::deserialize(const GF::Deserializer& dszr) {
        const YAML::Node& node = dszr.get();

        m_ss.reset(new StateSpace);
        m_ss->deserialize(node["State Space"]);

        YAML::Node graph_node = node["Graph"];
        for (auto src_it = graph_node.begin(); src_it != graph_node.end(); ++src_it) {
            const YAML::Node& src_node = src_it->second;
            State src(m_ss.get());
            src = src_node["Vars"].as<std::vector<std::string>>();
            
            const YAML::Node& neighbors = src_node["Neighbors"];
            for (auto dst_it = neighbors.begin(); dst_it != neighbors.end(); ++dst_it) {
                const YAML::Node& dst_node = dst_it->second;
                State dst(m_ss.get());
                dst = dst_node["Vars"].as<std::vector<std::string>>();
                float cost = dst_node["Cost"].as<float>();
                std::string action = dst_node["Action"].as<std::string>();
                connect(src, dst, TransitionSystemLabel(cost, action));
            }
        }

        YAML::Node propositions_node = node["Propositions"];
        for (auto prop_it = propositions_node.begin(); prop_it != propositions_node.end(); ++prop_it) {
            const YAML::Node& prop_node = *prop_it;

            Condition proposition;
            proposition.deserialize(prop_node);

            addProposition(proposition);
        }
    }

} // namespace DiscreteModel
} // namespace GF
