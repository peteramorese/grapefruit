#include "stratSynthesis.h"


template<class T> 
std::vector<int> RiskAvoidStrategy<T>::pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> pre_set;
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
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



template<class T> 
std::vector<int> RiskAvoidStrategy<T>::pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> pre_set;
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
        int s = ret_inds[0]; // game state
        int q = ret_inds[1]; // automaton state
        //std::cout<<"setting q: "<<q<<std::endl;
        dfa->set(q);
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        game.getParentNodes(s, node_list);
        game.getParentData(s, data_list);
        //std::cout<<"state: "<<s<<std::endl;
        const std::vector<std::string>* lbls = game.returnStateLabels(s);
        //for (auto& lbl : *lbls) {
        //    std::cout<<"lbl: " <<lbl<<std::endl;
        //}
        for (int i = 0; i<node_list.size(); ++i) {
            int sp = node_list[i];
            std::string temp_str = data_list[i]->label;

            if (game.getState(s).second == evolve_player) {
                //std::cout<<"good player!"<<std::endl;
                std::vector<int> parent_dfa_nodes;
                dfa->getParentNodesWithLabels(lbls, parent_dfa_nodes);
                //std::cout<<"par dfa nodes size: "<<parent_dfa_nodes.size()<<std::endl;
                for (auto& qp : parent_dfa_nodes) {
                    //std::cout<<"qp: "<<qp<<std::endl;
                    if (!dfa->getDFA()->isAccepting(qp)) { // no need to iterate over accepting states
                        //std::cout<<"not accepting!"<<std::endl;
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
    int pause;
    std::cin>>pause;
    return pre_set;
}


template<class T> 
std::vector<int> RiskAvoidStrategy<T>::post(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> post_set;
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
        int s = ret_inds[0]; // game state
        int q = ret_inds[1]; // automaton state
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        game.getConnectedNodes(s, node_list);
        game.getConnectedData(s, data_list);
        bool found_connection = false;
        for (int i = 0; i<node_list.size(); ++i) {
            int sp = node_list[i];
            dfa->set(q);
            const std::vector<std::string>* lbls = game.returnStateLabels(sp);
            std::string temp_str = data_list[i]->label;
            for (int j = 0; j<lbls->size(); ++j) {
                if (dfa->eval((*lbls)[i], true)) {
                    found_connection = true;
                    break;
                }

            }
            if (!found_connection) {
                std::cout<<"Error (post): Did not find connectivity in DFA.\n";
                return post_set;
            }
            int pp = Graph<int>::augmentedStateImage({sp, dfa->getCurrNode()}, graph_sizes);
            if (!S_incl[pp]) { //utilize default constructed value (false)
                post_set.push_back(pp);
            }
        }
    }
    return post_set;
}



template<class T> 
std::vector<int> RiskAvoidStrategy<T>::post(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> post_set;
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
        int s = ret_inds[0]; // game state
        int q = ret_inds[1]; // automaton state
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        game.getConnectedNodes(s, node_list);
        game.getConnectedData(s, data_list);
        bool found_connection = false;
        for (int i = 0; i<node_list.size(); ++i) {
            int sp = node_list[i];
            int pp;
            if (game.getState(sp).second == evolve_player) {
                dfa->set(q);
                const std::vector<std::string>* lbls = game.returnStateLabels(sp);
                std::string temp_str = data_list[i]->label;
                for (int j = 0; j<lbls->size(); ++j) {
                    if (dfa->eval((*lbls)[i], true)) {
                        found_connection = true;
                        break;
                    }

                }
                if (!found_connection) {
                    std::cout<<"Error (post): Did not find connectivity in DFA.\n";
                    return post_set;
                }
                pp = Graph<int>::augmentedStateImage({sp, dfa->getCurrNode()}, graph_sizes);
            } else {
                pp = Graph<int>::augmentedStateImage({sp, q}, graph_sizes);

            }
            if (!S_incl[pp]) { //utilize default constructed value (false)
                post_set.push_back(pp);
            }
        }
    }
    return post_set;
}

template<class T>
typename RiskAvoidStrategy<T>::Strategy RiskAvoidStrategy<T>::synthesize(Game<T>& game, DFA_EVAL* dfa) {
    Strategy strategy;
    std::vector<DFA_EVAL*> dfas = {dfa}; // Use this for preferences later
    std::vector<int> graph_sizes = {game.size(), dfa->getDFA()->size()};
    unsigned p_space_size = game.size() * dfa->getDFA()->size();
    std::cout<<"p_space_size: "<<p_space_size<<std::endl;
    strategy.policy.resize(p_space_size, "none");
    strategy.region.resize(p_space_size, false);

    std::vector<int> risk(p_space_size, -1); // infinity
    std::vector<int> O_init; // region set (init)
    std::unordered_map<int, bool> O; // region set (inclusion map)

    const std::vector<unsigned>* accepting_states = dfa->getDFA()->getAcceptingStates();
	std::vector<const std::vector<std::string>*> total_alphabet(1);
    total_alphabet[0] = dfa->getAlphabetEVAL();

    game.mapStatesToLabels(total_alphabet);
    for (auto q_acc : *accepting_states) {
        for (int j=0; j<game.size(); ++j) {
            //std::cout<<"j: "<<j<<std::endl;
            if (game.getState(j).second == 0) { // accepting states can only be system states
                int p = Graph<int>::augmentedStateImage({j, static_cast<int>(q_acc)}, graph_sizes);
                strategy.region[s] = true;
                risk[p] = 0;
                O_init.push_back(s);
                O[p] = true;
            }
        }
    } 

    std::vector<int> S = pre(game, dfa, graph_sizes, O_init, 0);
    //std::cout<<"O init size: "<<O_init.size()<<std::endl;
    //std::cout<<"S size: "<<S.size()<<std::endl;
    //for (auto& p : S) {
    //    std::cout<<"p in S:"<<p<<std::endl;
    //}
    bool updated = true;
    while (updated) {
        //std::cout<<"in loop"<<std::endl;
        updated = false;
        for (auto& p : S) {
            std::cout<<"in second lolp"<<std::endl;
            std::vector<int> ret_inds;
            Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
            int s = ret_inds[0];
            int q = ret_inds[1];
            if (game.getState(p).second == 0) { // system player
                std::cout<<"system state"<<std::endl;
                // Compute min(r(Post(p))) 
                std::vector<int> post_set = post(game, dfa, graph_sizes, {p}, 0);
                int min_val = -1;
                bool begin = true;
                int min_state = -1;
                //for (int i = 0; i<post_set.size(); ++i) {
                for (auto& pp : post_set) {
                    if (risk[pp] != -1) {
                        if (begin || risk[pp] < min_val) {
                            begin = false;
                            min_val = risk[pp];
                            min_state = pp;
                        }
                    }
                }
                if (min_val == -1) {
                    std::cout<<"Error: Post() set is all infinity\n";
                }
                if (min_val < risk[p]) {
                    updated = true;
                    risk[p];
                    WL* edge = game.getData(p, min_state);
                    strategy.policy[p] = edge->label;
                }
            } else { // environment player
                std::cout<<"env state"<<std::endl;

                int r_before = risk[p];
                std::vector<int> post_set = post(game, dfa, graph_sizes, {p}, 0);
                // Check if Post(p) is contained in O
                bool contained = true;
                int max_val = 0;
                for (auto& pp : post_set) {
                    if (O[pp]) { // Determine if p is a risk state
                        if (risk[pp] > max_val) {
                            max_val = risk[pp];
                        }
                    } else { // Get max post value (min/max game)
                        contained = false;
                    }
                }
                if (contained) {
                    // Risk state
                    risk[p] = max_val;
                } else {
                    // Non risk state:
                    risk[p] = max_val + 1;
                }
                if (r_before != risk[p]) {
                    updated = true;
                }
            }
        }
        for (auto& p : S) {
            O[p] = true;
        }
        S = pre(game, dfa, graph_sizes, S, 0);
    }
    return strategy;
}



template class RiskAvoidStrategy<State>;
//template class RiskAvoidStrategy<BlockingState>;