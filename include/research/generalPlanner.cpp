#include "generalPlanner.h"

template<class T> 
std::vector<int> GeneralPlanner<T>::post(TransitionSystem<T>& ts, std::vector<DFA_EVAL*> dfas, const std::vector<int>& set) {
    std::unordered_map<int, bool> S_incl;
    std::vector<int> post_set;
    std::vector<int> graph_sizes(dfas.size() + 1);
    graph_sizes[0] = ts.size();
    for (int i=0; i<dfas.size() ++i) {
        graph_sizes[i+1] = dfas[i]->getDFA()->size();
    }
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
                for (auto& lbl : (*lbls)); ++k) {
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