#include "stratSynthesis.h"


template<typename T> 
std::vector<int> RiskAvoidStrategy::pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> pre_set;
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, s, ret_inds);
        int s = ret_inds[0]; // game state
        int q = ret_inds[1]; // automaton state
        dfa->set(q);
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        game.getParentNodes(s, node_list);
        game.getParentData(s, data_list);
        const std::vector<std::string>* lbls = game.returnStateLabels(s);
        for (int i = 0; i<node_list.size(); ++i) {
            int sp = node_list[i];
            std::string temp_str = data_list[i]->label;

            std::vector<int> parent_dfa_nodes;
            dfa->getParentNodesWithLabels(lbls, parent_dfa_nodes);
            for (auto& qp : parent_dfa_nodes) {
                if (!dfa->getDFA()->isAccepting(qp)) { // no need to iterate over accepting states
                    int pp = Graph<int>::augmentedStateImage({sp, qp}, graph_sizes);
                    if (!S_incl[pp]) { //utilize default constructed value (false)
                        pre_set.push_back(pp);
                    }
                }
            }
        }
    }
    return pre_set;
}

template<typename T> 
std::vector<int> RiskAvoidStrategy::pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> pre_set;
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, s, ret_inds);
        int s = ret_inds[0]; // game state
        int q = ret_inds[1]; // automaton state
        dfa->set(q);
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        game.getParentNodes(s, node_list);
        game.getParentData(s, data_list);
        const std::vector<std::string>* lbls = game.returnStateLabels(s);
        for (int i = 0; i<node_list.size(); ++i) {
            int sp = node_list[i];
            std::string temp_str = data_list[i]->label;

            if (game.getState(s).second == evolve_player) {
                std::vector<int> parent_dfa_nodes;
                dfa->getParentNodesWithLabels(lbls, parent_dfa_nodes);
                for (auto& qp : parent_dfa_nodes) {
                    if (!dfa->getDFA()->isAccepting(qp)) { // no need to iterate over accepting states
                        int pp = Graph<int>::augmentedStateImage({sp, qp}, graph_sizes);
                        if (!S_incl[pp]) { //utilize default constructed value (false)
                            pre_set.push_back(pp);
                        }
                    }
                }
            } else  {
                int pp = Graph<int>::augmentedStateImage({sp, q}, graph_sizes);
                if (!S_incl[pp]) { //utilize default constructed value (false)
                    pre_set.push_back(pp);
                }
            }
        }
    }
    return pre_set;
}

template<typename T>
RiskAvoidStrategy::Strategy RiskAvoidStrategy::synthesize(Game<T>& game, DFA_EVAL* dfa) {
    Strategy strategy;
    std::vector<DFA_EVAL*> dfas = {dfa}; // Use this for preferences later
    std::vector<int> graph_sizes = {game.size(), dfa->getDFA()->size()};
    unsigned p_space_size = game.size() * dfa->size();
    strategy.policy.resize(p_space_size, "none");
    strategy.region.resize(p_space_size, false);

    std::vector<int> risk(p_space_size, -1); // infinity
    std::vector<int> O; // region set

    const std::vector<unsigned>* accepting_states = dfa->getDFA()->getAcceptingStates();
    for (int i=0; i<accepting_states.size(); ++i) {
        for (int j=0; j<game.size(); ++j) {
            if (game.getState(j).second == 0) { // accepting states can only be system states
                int s = Graph<int>::augmentedStateImage({j, accepting_states[i]}, graph_sizes);
                strategy.region[s] = true;
                risk[s] = 0;
                O.push_back(s);

            }
        }
    } 

    std::vector<int> S = pre(game, dfa, graph_sizes, O, 0);

}
