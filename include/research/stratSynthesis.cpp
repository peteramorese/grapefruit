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
                        S_incl[pp] = true;
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
        //std::cout<<"s: "<<s<<" q: "<<q<<std::endl;
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
                            //std::cout<<"  -ADDING s: "<<sp<<" q: "<<qp<<std::endl;
                            pre_set.push_back(pp);
                            S_incl[pp] = true;
                        }
                    }
                }
            } else  {
                if (!dfa->getDFA()->isAccepting(q)) {
                    int pp = Graph<int>::augmentedStateImage({sp, q}, graph_sizes);
                    if (!S_incl[pp]) { //utilize default constructed value (false)
                        //std::cout<<"ADDING s: "<<sp<<std::endl;
                        pre_set.push_back(pp);
                        S_incl[pp] = true;
                    }

                }
            }
        }
    }
    //int pause;
    //std::cin>>pause;
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
                if (dfa->eval((*lbls)[j], true)) {
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
                S_incl[pp] = true;
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
                for (auto& lbl : (*lbls)) {
                    if (dfa->eval(lbl, true)) {
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
                S_incl[pp] = true;
            }
        }
    }
    return post_set;
}

template<class T>
typename Game<T>::Strategy RiskAvoidStrategy<T>::synthesize(Game<T>& game, DFA_EVAL* dfa) {
    typename Game<T>::Strategy strategy;
    std::vector<DFA_EVAL*> dfas = {dfa}; // Use this for preferences later
    std::vector<int> graph_sizes = {game.size(), dfa->getDFA()->size()};
    unsigned p_space_size = game.size() * dfa->getDFA()->size();
    std::cout<<"p_space_size: "<<p_space_size<<std::endl;
    strategy.success = false;
    strategy.policy.resize(p_space_size, "");
    strategy.region.resize(p_space_size, false);

    std::vector<bool> visited(p_space_size, false);

    std::vector<int> risk(p_space_size, -1); // infinity
    std::vector<int> O_init; // region set (init)
    std::unordered_map<int, bool> O; // region set (inclusion map)

    const std::vector<unsigned>* accepting_states = dfa->getDFA()->getAcceptingStates();
	std::vector<const std::vector<std::string>*> total_alphabet(1);
    total_alphabet[0] = dfa->getAlphabetEVAL();

    //int p_init = Graph<int>::augmentedStateImage({0, dfa->getDFA()->getInitState()}, graph_sizes);
    //strategy.region[p] = 

    game.mapStatesToLabels(total_alphabet);
    for (auto q_acc : *accepting_states) {
        for (int j=0; j<game.size(); ++j) {
            //std::cout<<"j: "<<j<<std::endl;
            int p = Graph<int>::augmentedStateImage({j, static_cast<int>(q_acc)}, graph_sizes);
            if (game.getState(j).second == 0) { // accepting states can only be system states
                strategy.region[p] = true;

                //std::vector<int> ret_inds_O;
                //Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds_O);
                //std::cout<<"   > printing O s: "<<ret_inds_O[0]<<", q: "<<ret_inds_O[1]<<", p: "<<p<<std::endl;

                std::cout<<"   > setting O to 0 (s: "<<j<<", q: "<<q_acc<<", p: "<<p<<")"<<std::endl;
                risk[p] = 0;
                O_init.push_back(p);
                O[p] = true;
            } else {
                strategy.policy[p] = "ENV";
            }
            visited[p] = true;
        }
    } 

    std::vector<int> S = pre(game, dfa, graph_sizes, O_init, 0);

    //std::vector<int> ret_inds_test;
    //Graph<int>::augmentedStatePreImage(graph_sizes, 22, ret_inds_test);
    //std::cout<<"TEST : "<<ret_inds_test[0]<<", q: "<<ret_inds_test[1]<<", p: "<<22<<std::endl;

    //std::cout<<"O init size: "<<O_init.size()<<std::endl;
    //std::cout<<"S size: "<<S.size()<<std::endl;
    bool updated = true;
    int iterations = 0;
    while (updated) {
        for (auto& p : S) {
            std::vector<int> ret_inds_S;
            Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds_S);
            std::cout<<" > printing S s: "<<ret_inds_S[0]<<", q: "<<ret_inds_S[1]<<", p: "<<p<<std::endl;
        }
        //std::cout<<"in loop"<<std::endl;
        iterations++;
        //std::cout<<"iter: "<<iterations<<std::endl;
        //std::cout<<"S size: "<<S.size()<<std::endl;
        updated = false;
        for (auto& p : S) {
            std::vector<int> ret_inds;
            Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
            int s = ret_inds[0];
            int q = ret_inds[1];
            std::cout<<"\nCURRENT p: "<<p<<", s: "<<s<<", q: "<<q<<std::endl;

            //if (p ==192) {
            //    std::cout<<"                       192 in loop player: "<<game.getState(s).second <<std::endl;
            //}
            visited[p] = true;

            if (game.getState(s).second == 0) { // system player
                std::cout<<"System state"<<std::endl;
                // Compute min(r(Post(p))) 

                //bool debug = false;
                //if (p == 200) {
                //    std::cout<<" FOUND P " <<std::endl;
                //    debug = true;

                //}

                std::vector<int> post_set = post(game, dfa, graph_sizes, {p}, 0);
                int min_val = -1;
                bool begin = true;
                int min_state = -1;
                for (auto& pp : post_set) {
                    std::vector<int> ret_inds_pp;
                    Graph<int>::augmentedStatePreImage(graph_sizes, pp, ret_inds_pp);
                    std::cout<<" -Post (s: "<<ret_inds_pp[0]<<", q: "<<ret_inds_pp[1]<<", p: "<<pp<<")"<<risk[pp]<<std::endl;

                    if (risk[pp] != -1) {
                        if (begin || risk[pp] < min_val) {
                            begin = false;
                            min_val = risk[pp];
                            min_state = pp;
                        }
                    }

                    //std::cout<<" risk["<<pp<<"] "<<risk[pp]<<", MIN VAL: "<<min_val<<std::endl;
                    //int pause;
                    //std::cin>>pause;

                }
                if (min_val == -1) {
                    std::cout<<"Error: Post() set is all infinity\n";
                    return strategy;
                }


                if (min_val < risk[p] || risk[p] == -1) {

                    //if (debug) {
                    //int pause; std::cin>>pause;
                    //}

                    updated = true;
                    risk[p] = min_val;
                    std::vector<int> ret_inds_min_state;
                    Graph<int>::augmentedStatePreImage(graph_sizes, min_state, ret_inds_min_state);
                    int min_state_s = ret_inds_min_state[0];
                    WL* edge = game.getData(s, min_state_s);
                    //std::cout<<"trying edge: "<<std::endl;
                    std::cout<<"    policy(s: "<<s<<", q: "<<q<<", p: "<<p<<"): "<< edge->label<<" with min risk val: "<<min_val<<std::endl;
                    strategy.policy[p] = edge->label;
                }

                //if (debug) {
                //    int pause;
                //    std::cin>>pause;
                //}
                
            } else { // environment player
                std::cout<<"Environment state: "<<std::endl;
                int r_before = risk[p];
                std::vector<int> post_set = post(game, dfa, graph_sizes, {p}, 0);
                // Check if Post(p) is contained in O
                bool contained = true;
                int max_val = 0;
                for (auto& pp : post_set) {

                    std::vector<int> ret_inds_pp;
                    Graph<int>::augmentedStatePreImage(graph_sizes, pp, ret_inds_pp);
                    std::cout<<" -Post (s: "<<ret_inds_pp[0]<<", q: "<<ret_inds_pp[1]<<", p: "<<pp<<")"<<risk[pp]<<std::endl;

                    if (O[pp]) { // Determine if p is a risk state
                        //if (risk[pp] == -1) {
                        //    std::cout<<"FOUND AN INF IN O ";
                        //    int pause;
                        //    std::cin;
                        //}
                        if (risk[pp] > max_val) {
                            max_val = risk[pp];
                        }
                    } else { // Get max post value (min/max game)
                        //if (p == 526) {
                        //    std::cout<<" FOUND PSTATE: "<< pp<< "NOT IN O FROM 526"<<std::endl;
                        //    int pause;
                        //    std::cin>>pause;
                        //}
                        //std::cout<<"found risk state!"<<std::endl;
                        visited[pp] = true;
                        strategy.policy[pp] = "VIO";
                        contained = false;
                    }
                }
                if (contained) {
                    // Risk state
                    std::cout<<"    (contained) Setting r["<<p<<"] = "<<max_val<<std::endl;
                    risk[p] = max_val;
                } else {
                    // Non risk state:
                    std::cout<<"    (NOT contained) Setting r["<<p<<"] = "<<max_val + 1<<std::endl;
                    risk[p] = max_val + 1;
                }
                if (r_before != risk[p]) {
                    updated = true;
                }
            }

        //int pause;
        //std::cin>>pause;

        }
        for (auto& p : S) {
            std::cout<<"  > adding: "<<p<<" to O"<<std::endl;
            O[p] = true;
            strategy.region[p] = true;
        }
        S = pre(game, dfa, graph_sizes, S, 0);

        //pause;
        //std::cin>>pause;
        //for (auto& p : S) {
        //    std::vector<int> ret_inds;
        //    Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
        //    int s = ret_inds[0];
        //    std::cout<<"s in S:"<<s<<std::endl;
        //}

    }

    for (int i=0; i<risk.size(); ++i) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, i, ret_inds);
        int s = ret_inds[0]; // game state
        std::cout<<" risk (s: "<<s<<", q: "<<ret_inds[1]<<", p: "<<i<<")"<<risk[i]<<std::endl;
    }
	//for (auto& r : risk) {
	//	std::cout<<"risk: "<<r<<std::endl;
	//}
    strategy.success = O[game.getInitStateInd()]; // Is the init state included in O?
    return strategy;
}



template class RiskAvoidStrategy<State>;
//template class RiskAvoidStrategy<BlockingState>;