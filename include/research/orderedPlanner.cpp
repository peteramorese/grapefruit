#include "orderedPlanner.h"
    
///////////// SYMBOLIC STATIC METHODS ///////////// 

std::vector<int> SymbolicMethods::getGraphSizes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas) {
    std::vector<int> graph_sizes(dfas.size() + 1);
    graph_sizes[0] = ts.size();
    for (int i=0; i<dfas.size(); ++i) {
        graph_sizes[i+1] = dfas[i]->getDFA()->size();
    }
    return graph_sizes;
}

std::vector<int> SymbolicMethods::post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> post_set;
    std::vector<int> graph_sizes = getGraphSizes(ts, dfas);
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
        int s = ret_inds[0]; // game state
        //int q = ret_inds[1]; // automaton state
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        ts.getConnectedNodes(s, node_list);
        ts.getConnectedData(s, data_list);
        for (int i = 0; i<node_list.size(); ++i) {
            int sp = node_list[i];
            //dfa->set(q);
            for (int j=0; j<dfas.size(); ++j) {
                dfas[j]->set(ret_inds[j+1]);
            }
            const std::vector<std::string>* lbls = ts.returnStateLabels(sp);
            std::string temp_str = data_list[i]->label;
            bool found_connection = true;
            for (auto& dfa : dfas) {
                bool found = false;
                for (auto& lbl : (*lbls)) {
                    if (dfa->eval(lbl, true)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    found_connection = false;
                    break;
                }
            }
            if (!found_connection) {
                continue;
            }
            std::vector<int> inds(dfas.size() + 1);
            inds[0] = sp;
            for (int j=0; j<dfas.size(); ++j) {
                inds[j+1] = dfas[j]->getCurrNode();
            }
            int pp = Graph<int>::augmentedStateImage(inds, graph_sizes);
            if (!S_incl[pp]) { //utilize default constructed value (false)
                post_set.push_back(pp);
                S_incl[pp] = true;
            }
        }
    }
    return post_set;
}

std::vector<int> SymbolicMethods::pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> pre_set;
    std::vector<int> graph_sizes = getGraphSizes(ts, dfas);
    for (auto& pp : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, pp, ret_inds);
        int sp = ret_inds[0]; // game state
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        ts.getParentNodes(sp, node_list);
        ts.getParentData(sp, data_list);
        const std::vector<std::string>* lbls = ts.returnStateLabels(sp);
        for (int i = 0; i<node_list.size(); ++i) {
            for (int j=0; j<dfas.size(); ++j) {
                dfas[j]->set(ret_inds[j+1]);
            }
            int s = node_list[i];
            std::string temp_str = data_list[i]->label;

            std::vector<std::vector<int>> temp_par_container(dfas.size()); // Array of lists of parent nodes for each DFA
            std::vector<int> node_list_sizes(dfas.size());
            int parent_node_list_size = 1;
            bool found_connection;
            for (int ii=0; ii<dfas.size(); ++ii) {
                found_connection = dfas[ii]->getParentNodesWithLabels(lbls, temp_par_container[ii]);
                parent_node_list_size *= temp_par_container[ii].size();
                node_list_sizes[ii] = temp_par_container[ii].size();
                if (!found_connection) {
                    break;
                }
            }
            if (found_connection){
                for (int ii=0; ii<parent_node_list_size; ++ii) {
                    std::vector<int> node(dfas.size() + 1); // Temp unique node
                    // Use augmented state to cycle through every combination of parent dfa nodes:
                    std::vector<int> ret_list_inds;
                    Graph<float>::augmentedStatePreImage(node_list_sizes, ii, ret_list_inds);
                    node[0] = s;
                    for (int iii=0; iii<dfas.size(); ++iii) {
                        node[iii+1] = temp_par_container[iii][ret_list_inds[iii]];
                    }
                    int p = Graph<float>::augmentedStateImage(node, graph_sizes);
                    if (!S_incl[p]) { //utilize default constructed value (false)
                        pre_set.push_back(p);
                        S_incl[p] = true;
                    }
                }
            } else {
                continue;
            }
        }
    }
    return pre_set;
}

std::vector<int> SymbolicMethods::post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> post_set;
    std::vector<int> graph_sizes = getGraphSizes(ts, dfas);
    for (auto& p : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
        int s = ret_inds[0]; // game state
        //int q = ret_inds[1]; // automaton state
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        ts.getConnectedNodes(s, node_list);
        ts.getConnectedData(s, data_list);
        for (int i = 0; i<node_list.size(); ++i) {
            int sp = node_list[i];
            //dfa->set(q);
            for (int j=0; j<dfas.size(); ++j) {
                dfas[j]->set(ret_inds[j+1]);
            }
            const std::vector<std::string>* lbls = ts.returnStateLabels(sp);
            std::string temp_str = data_list[i]->label;
            bool found_connection = true;
            for (auto& dfa : dfas) {
                bool found = false;
                for (auto& lbl : (*lbls)) {
                    if (dfa->eval(lbl, true)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    found_connection = false;
                    break;
                }
            }
            if (!found_connection) {
                continue;
            }
            std::vector<int> inds(dfas.size() + 1);
            inds[0] = sp;
            for (int j=0; j<dfas.size(); ++j) {
                inds[j+1] = dfas[j]->getCurrNode();
            }
            int pp = Graph<int>::augmentedStateImage(inds, graph_sizes);
            if (!S_incl[pp] && inclMe(pp)) { //utilize default constructed value (false)
                post_set.push_back(pp);
                S_incl[pp] = true;
            }
        }
    }
    return post_set;
}

std::vector<int> SymbolicMethods::pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> pre_set;
    std::vector<int> graph_sizes = getGraphSizes(ts, dfas);
    for (auto& pp : set) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, pp, ret_inds);
        int sp = ret_inds[0]; // game state
        std::vector<int> node_list;
        std::vector<WL*> data_list;
        ts.getParentNodes(sp, node_list);
        ts.getParentData(sp, data_list);
        const std::vector<std::string>* lbls = ts.returnStateLabels(sp);
        for (int i = 0; i<node_list.size(); ++i) {
            for (int j=0; j<dfas.size(); ++j) {
                dfas[j]->set(ret_inds[j+1]);
            }
            int s = node_list[i];
            std::string temp_str = data_list[i]->label;

            std::vector<std::vector<int>> temp_par_container(dfas.size()); // Array of lists of parent nodes for each DFA
            std::vector<int> node_list_sizes(dfas.size());
            int parent_node_list_size = 1;
            bool found_connection;
            for (int ii=0; ii<dfas.size(); ++ii) {
                found_connection = dfas[ii]->getParentNodesWithLabels(lbls, temp_par_container[ii]);
                parent_node_list_size *= temp_par_container[ii].size();
                node_list_sizes[ii] = temp_par_container[ii].size();
                if (!found_connection) {
                    break;
                }
            }
            if (found_connection){
                for (int ii=0; ii<parent_node_list_size; ++ii) {
                    std::vector<int> node(dfas.size() + 1); // Temp unique node
                    // Use augmented state to cycle through every combination of parent dfa nodes:
                    std::vector<int> ret_list_inds;
                    Graph<float>::augmentedStatePreImage(node_list_sizes, ii, ret_list_inds);
                    node[0] = s;
                    for (int iii=0; iii<dfas.size(); ++iii) {
                        node[iii+1] = temp_par_container[iii][ret_list_inds[iii]];
                    }
                    int p = Graph<float>::augmentedStateImage(node, graph_sizes);
                    if (!S_incl[p] && inclMe(p)) { //utilize default constructed value (false)
                        pre_set.push_back(p);
                        S_incl[p] = true;
                    }
                }
            } else {
                continue;
            }
        }
    }
    return pre_set;
}



///////////// ORDERED PLANNER ///////////// 

const OrderedPlanner::Plan* OrderedPlanner::Result::getPlan(float mu_max) const {
    for (const auto& pt : pareto_front) {
        if (pt.mu <= mu_max) {
            return &pt.plan;
        }
    }
    return nullptr;
}

const OrderedPlanner::Plan* OrderedPlanner::Result::getPlan(unsigned ind) const {
    int i = 0;
    for (const auto& pt : pareto_front) {
        if (i == ind) {
            return &pt.plan;
        }
        i++;
    }
    return nullptr;
}

const std::list<OrderedPlanner::Result::ParetoPoint>* OrderedPlanner::Result::getParetoFront() const {
    return &pareto_front;
}

bool OrderedPlanner::Result::addParetoPoint(float mu, float path_length, const Plan& plan) {
    auto iter = pareto_front.begin();
    if (pareto_front.size() == 0) {
        pareto_front.push_back({mu, path_length, plan});
        return true;
    }
    while (1) {
        // Check if a more optimal pareto point is found:
        auto& pt = *iter;
        if (mu == pt.mu) {
            if (path_length < pt.path_length) {
                pt.path_length = path_length;
                pt.plan = plan;
                return true;
            } else {
                return false;
            }
        } 
        if (path_length == pt.path_length) {
            if (mu < pt.mu) {
                pt.mu = mu;
                pt.plan = plan;
                return true;
            } else {
                return false;
            }
        }
        auto it_before = std::next(iter, -1);
        bool check_before = false;
        bool check_after = false;
        if (iter != pareto_front.begin()) {
            if (mu > (*it_before).mu && path_length < (*it_before).path_length) {
                check_before = true;
            }
        } else {
            check_before = true;
        }
        if (iter != pareto_front.end()) {
            if (mu < pt.mu)  {
                if (path_length > pt.path_length) {
                    check_after = true;
                } else {
                    pareto_front.insert(iter, {mu, path_length, plan});
                    auto it_remove = iter;
                    while (path_length <= (*it_remove).path_length) {
                        std::advance(iter, 1);
                        pareto_front.erase(it_remove);
                        it_remove = iter;
                    }
                    return true;
                }
            } 
        } else {
            check_after = true;
        }
        ParetoPoint new_pt = {mu, path_length, plan};
        if (check_before && check_after) {
            pareto_front.insert(iter, new_pt);
            return true;
        } 
        if (iter == pareto_front.end()) {
            break;
        }
        std::advance(iter, 1);
    } 
    return false;
}

void OrderedPlanner::Result::printParetoFront() {
    std::cout<<"Printing Pareto Front ("<<pareto_front.size()<<" elements):\n";
    for (const auto& pt : pareto_front) {
        std::cout<<" - mu: "<<pt.mu<<"   path_length: "<<pt.path_length<<"\n";
    }
}

bool OrderedPlanner::search(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::function<float(std::vector<int>)>& setToMu) {
	std::vector<const std::vector<std::string>*> total_alphabet(dfas.size());
	for (int i=0; i<dfas.size(); ++i) {
		total_alphabet[i] = dfas[i]->getAlphabetEVAL();
	}
    ts.mapStatesToLabels(total_alphabet);

}