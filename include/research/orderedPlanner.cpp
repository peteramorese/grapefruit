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


SymbolicMethods::ConnectedNodes SymbolicMethods::postNodes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe) {
    std::unordered_map<int, bool> S_incl;
    ConnectedNodes post_set_nodes;
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
                post_set_nodes.nodes.push_back(pp);
                post_set_nodes.data.push_back(data_list[i]);
                S_incl[pp] = true;
            }
        }
    }
    return post_set_nodes;
}

SymbolicMethods::ConnectedNodes SymbolicMethods::preNodes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe) {
    std::unordered_map<int, bool> S_incl;
    ConnectedNodes pre_set_nodes;
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
                        pre_set_nodes.nodes.push_back(p);
                        pre_set_nodes.data.push_back(data_list[i]);
                        S_incl[p] = true;
                    }
                }
            } else {
                continue;
            }
        }
    }
    return pre_set_nodes;
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

void OrderedPlanner::Result::printParetoFront() const {
    std::cout<<"Printing Pareto Front ("<<pareto_front.size()<<" elements):\n";
    for (const auto& pt : pareto_front) {
        std::cout<<" - mu: "<<pt.mu<<"   path_length: "<<pt.path_length<<"\n";
    }
}

OrderedPlanner::Node::Node() {}
OrderedPlanner::Node::Node(int ind_, float cost_, float f_cost_, const std::vector<float>& cost_set_) : ind(ind_), cost(cost_), f_cost(f_cost_), cost_set(cost_set_) {}

OrderedPlanner::OrderedPlanner(bool verbose_) : verbose(verbose_) {}

std::unique_ptr<OrderedPlanner::Node> OrderedPlanner::newNode() {
    return std::make_unique<Node>();
}

std::unique_ptr<OrderedPlanner::Node> OrderedPlanner::newNode(const Node& node) {
    std::unique_ptr<Node> new_node = std::make_unique<Node>();
    *new_node = node;
    return new_node;
}

std::unique_ptr<OrderedPlanner::Node> OrderedPlanner::newNode(const std::vector<int>& inds, float cost, float f_cost, const std::vector<float>& cost_set) {
    int p = Graph<float>::augmentedStateImage(inds, graph_sizes);
    return std::make_unique<Node>(p, cost, f_cost, cost_set);
}

int OrderedPlanner::newNode(const std::vector<int>& inds, float cost, float f_cost, const std::vector<float>& cost_set, std::unordered_map<int, std::unique_ptr<Node>>& node_map) {
    int p = Graph<float>::augmentedStateImage(inds, graph_sizes);
    std::unique_ptr<Node> node = std::make_unique<Node>(p, cost, f_cost, cost_set);
    node_map[p] = std::move(node);
    return p;
}

OrderedPlanner::Node* OrderedPlanner::newNode(const Node& node, std::unordered_map<int, std::unique_ptr<Node>>& node_map) {
    std::unique_ptr<Node> new_node = std::make_unique<Node>();
    *new_node = node;
    node_map[node.ind] = std::move(new_node);
    return node_map[node.ind].get(); // Return moved ptr
}

bool OrderedPlanner::search(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::function<float(const std::vector<float>&)>& setToMu, bool use_heuristic) {
    // Set up the transition system:
    ts_ptr = &ts;
	std::vector<const std::vector<std::string>*> total_alphabet(dfas.size());
	for (int i=0; i<dfas.size(); ++i) {
		total_alphabet[i] = dfas[i]->getAlphabetEVAL();
	}
    ts.mapStatesToLabels(total_alphabet);
    graph_sizes = SymbolicMethods::getGraphSizes(ts, dfas);
    int p_space_size = 1;
    for (auto sz : graph_sizes) {
        p_space_size *= sz;
    }

    // Node check and pq structures:
    std::unordered_map<int, bool> visited; // Checks if the node has been visited
    std::unordered_map<int, bool> seen; // Checks if the node has been encountered
    std::unordered_map<int, std::pair<int, std::string>> parents; // Holds parent node and action
    std::unordered_map<int, float> min_cost; // Min cost to get to node
    auto compare  = [](const Node* p1, const Node* p2) {return p1->f_cost > p2->f_cost;};
    std::priority_queue<Node*, std::vector<Node*>, decltype(compare)> pq(compare);
    std::unordered_map<int, std::unique_ptr<Node>> node_map;

    // Create the init root node:
	std::vector<int> init_node_inds(dfas.size() + 1);
    std::vector<float> init_cost_set(dfas.size(), 0.0f);
	init_node_inds[0] = ts.getInitStateInd();
	for (int i=0; i<dfas.size(); ++i) {
		dfas[i]->reset();
		init_node_inds[i+1] = dfas[i]->getCurrNode();
	}
    int p_init = newNode(init_node_inds, 0.0f, 0.0f, init_cost_set, node_map);
    visited[p_init] = true;
    seen[p_init] = true; // No parent for init node
    parents[p_init] = {-1, "none"}; // No parent for init node
    min_cost[p_init] = 0.0f; 
    pq.push(node_map[p_init].get());

    // Init parameters used in search:
    bool solution_found = false;
    int iterations = 0;
    success = false; // member variable
    float mu_max = -1.0f; // Unset

    while (!pq.empty()) {
        //std::cout<<"pq size: "<<pq.size()<<std::endl;
        iterations++;
        Node* curr_leaf = pq.top();
        int p = curr_leaf->ind;
        pq.pop();
        visited[p] = true;

        // Get individual graph node indices and set curr node:
        std::vector<int> ret_inds;
        Graph<float>::augmentedStatePreImage(graph_sizes, p, ret_inds);
        
        // If the popped node is accepting, it is a solution:
        bool all_accepting = true;
        for (int i=0; i<dfas.size(); ++i) {
            if (!dfas[i]->getDFA()->isAccepting(ret_inds[i+1])) {
                all_accepting = false;
                break;
            }
        }
        if (all_accepting) {
            //std::cout<<"set: "<<std::endl;
            //for (auto item : curr_leaf->cost_set) std::cout<<" cost set: "<<item<<std::endl;
            float new_mu = setToMu(curr_leaf->cost_set);  
            if (new_mu < mu_max || !success) {
                mu_max = new_mu;
                if (verbose) std::cout<<"Found solution! mu: "<<mu_max<<", path length: "<<curr_leaf->cost<<std::endl;
                Plan pl = extractPlan(p, p_init, parents);
                result.addParetoPoint(mu_max, curr_leaf->cost, pl);
                success = true;
            }
            continue;
        }

        // Get connected product nodes:
        auto inclMe = [use_heuristic, &visited](int pp) {
            // If Djkstra's, dont consider visited nodes:
            return (!use_heuristic && visited[pp]) ? false : true;
        }; 
        SymbolicMethods::ConnectedNodes con_nodes = SymbolicMethods::postNodes(ts, dfas, {p}, inclMe);

        // Cycle through all connected nodes:
        for (int i=0; i<con_nodes.nodes.size(); ++i) {
            int pp = con_nodes.nodes[i];
            WL* edge = con_nodes.data[i];
            
            // Get individual graph node indices and set curr node:
            std::vector<int> con_ret_inds;
            Graph<float>::augmentedStatePreImage(graph_sizes, pp, con_ret_inds);

            // Check if all nodes are accepting, if not add appropriate costs:
            float new_cost = curr_leaf->cost + edge->weight;
            Node node_candidate = {pp, new_cost, new_cost, curr_leaf->cost_set};
            for (int i=0; i<dfas.size(); ++i) {
                if (!dfas[i]->getDFA()->isAccepting(con_ret_inds[i+1])) {
                    node_candidate.cost_set[i] += edge->weight;
                }
            }

            // Prune nodes:
            float mu = setToMu(node_candidate.cost_set);
            if (success && mu > mu_max) {
                continue;
            } 

            // Check if node was seen and a short path was found:
            bool updated = false;
            if (seen[pp]) {
                if (node_candidate.cost < min_cost[pp]) {
                    parents[pp] = {p, edge->label};
                    updated = true;
                } 
                // Do not continue if 1) using A*, and 2) the node was updated:
                if (!updated || !use_heuristic) {
                    continue;
                }
            }

            if (use_heuristic) {
                // TODO: get 'h_cost' value (A*) and add it to 'cost'
            } 

            // Made it thru all checks, add the node to the graph and queue
            // (store nodes on the heap to handle massive graphs):
            Node* new_node = newNode(node_candidate, node_map);
            pq.push(new_node);
            seen[pp] = true;
            min_cost[pp] = node_candidate.cost;
            parents[pp] = {p, edge->label};
        }    
    }
    return success;
}

OrderedPlanner::Plan OrderedPlanner::extractPlan(int p_acc, int p_init, const std::unordered_map<int, std::pair<int, std::string>>& parents) {
    int curr_ind = p_acc;
    Plan plan; 
    Plan reverse_plan;
    std::vector<int> reverse_state_inds;
    while (curr_ind != p_init) {
        // Get individual graph node indices and set curr node:
        std::vector<int> ret_inds;
        Graph<float>::augmentedStatePreImage(graph_sizes, curr_ind, ret_inds);

        reverse_plan.action_sequence.push_back(parents.at(curr_ind).second);
        reverse_plan.state_sequence.push_back(ts_ptr->getState(ret_inds[0]));
        reverse_state_inds.push_back(ret_inds[0]);
        curr_ind = parents.at(curr_ind).first;
    }
    std::vector<int> ret_inds;
    Graph<float>::augmentedStatePreImage(graph_sizes, curr_ind, ret_inds);
    reverse_plan.state_sequence.push_back(ts_ptr->getState(ret_inds[0])); // Init state
    
    // Reverse the plan:
    plan.action_sequence.resize(reverse_plan.action_sequence.size());
    plan.state_sequence.resize(reverse_plan.state_sequence.size());

    int i = 0;
    if (verbose) std::cout<<"Extracting plan... \n - Printing action sequence: ";
    for (auto rit = reverse_plan.action_sequence.rbegin(); rit != reverse_plan.action_sequence.rend(); ++rit) {
        plan.action_sequence[i] = *rit;
        if (verbose) std::cout<<" -> "<<*rit;
    }
    if (verbose) std::cout<<"\n";

    i = 0;
    if (verbose) {
        std::cout<<" - Printing state index sequence: "; 
        for (auto rit = reverse_state_inds.rbegin(); rit != reverse_state_inds.rend(); ++rit) { 
            std::cout<<" -> "<<*rit;
        }
        std::cout<<"\n";
    }
    for (auto rit = reverse_plan.state_sequence.rbegin(); rit != reverse_plan.state_sequence.rend(); ++rit) {
        plan.state_sequence[i] = *rit;
    }
    return plan;
}

const OrderedPlanner::Result* OrderedPlanner::getResult() const {
    return (success) ? &result : nullptr;
}