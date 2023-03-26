#include "lexSet.h"
#include "orderedPlanner.h"
#include<map>
    
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


template<> struct std::hash<OrderedPlanner::VisitedNode> {
    friend OrderedPlanner::VisitedNode;
    std::size_t operator()(const OrderedPlanner::VisitedNode& vn) const {
        return std::hash<int>()(vn.ind);
    }
};

std::shared_ptr<const OrderedPlanner::Plan> OrderedPlanner::Result::getPlan(float mu_max) const {
    for (const auto& pt : pareto_front) {
        if (pt.mu <= mu_max) {
            // Put plan on the heap once it is retrieved:
            return std::make_shared<const Plan>(pt.plan);
        }
    }
    return nullptr;
}

std::shared_ptr<const OrderedPlanner::Plan> OrderedPlanner::Result::getPlan(unsigned ind) const {
    int i = 0;
    for (const auto& pt : pareto_front) {
        if (i == ind) {
            // Put plan on the heap once it is retrieved:
            return std::make_shared<const Plan>(pt.plan);
            //return &pt.plan;
        }
        i++;
    }
    return nullptr;
}

const std::list<OrderedPlanner::Result::ParetoPoint>* OrderedPlanner::Result::getParetoFront() const {
    return &pareto_front;
}

unsigned OrderedPlanner::Result::addParetoPoint(float mu, float path_length, const Plan& plan) {
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
                return OrderedPlanner::Result::UPDATED;
            } else {
                return OrderedPlanner::Result::NEGLECTED;
            }
        } 
        if (path_length == pt.path_length) {
            if (mu < pt.mu) {
                pt.mu = mu;
                pt.plan = plan;
                return OrderedPlanner::Result::UPDATED;
            } else {
                return OrderedPlanner::Result::NEGLECTED;
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
                    return OrderedPlanner::Result::ADDED;
                }
            } 
        } else {
            check_after = true;
        }
        ParetoPoint new_pt = {mu, path_length, plan};
        if (check_before && check_after) {
            pareto_front.insert(iter, new_pt);
            return OrderedPlanner::Result::ADDED;
        } 
        if (iter == pareto_front.end()) {
            break;
        }
        std::advance(iter, 1);
    } 
    return OrderedPlanner::Result::NEGLECTED;
}

void OrderedPlanner::Result::printParetoFront() const {
    std::cout<<"Printing Pareto Front ("<<pareto_front.size()<<" elements):\n";
    for (const auto& pt : pareto_front) {
        std::cout<<" - mu: "<<pt.mu<<"   path_length: "<<pt.path_length<<"\n";
    }
}

void OrderedPlanner::Result::clear() {
    iterations = 0;
    pareto_front.clear();
}

OrderedPlanner::CostToGoal::CostToGoal(unsigned sz) : cost(sz, 0.0f), reachable(sz, false) {}

void OrderedPlanner::CostToGoal::resize(unsigned sz) {
    cost.resize(sz);
    reachable.resize(sz);
}

void OrderedPlanner::CostToGoal::clear() {
    cost.clear();
    reachable.clear();
}


OrderedPlanner::Node::Node() {}

OrderedPlanner::Node::Node(int ind_, float cost_, float f_cost_, float mu_, const std::vector<float>& cost_set_) : ind(ind_), cost(cost_), f_cost(f_cost_), mu(mu_), cost_set(cost_set_) {}

OrderedPlanner::VisitedNode::VisitedNode(int ind_, float mu_max_) : ind(ind_), mu_max(mu_max_) {}

bool OrderedPlanner::VisitedNode::operator==(const VisitedNode& vn_in) const {
    return ind == vn_in.ind && mu_max == vn_in.mu_max;
}

OrderedPlanner::OrderedPlanner(TransitionSystem<State>& ts_, bool verbose_, const std::string* bm_filepath_) : ts(ts_), verbose(verbose_), bm_filepath(bm_filepath_), bm(bm_filepath) {
    heuristic.first = false; // Heuristic is not generated
}

std::unique_ptr<OrderedPlanner::Node> OrderedPlanner::newNode() {
    return std::make_unique<Node>();
}

std::unique_ptr<OrderedPlanner::Node> OrderedPlanner::newNode(const Node& node) {
    std::unique_ptr<Node> new_node = std::make_unique<Node>();
    *new_node = node;
    return new_node;
}

std::unique_ptr<OrderedPlanner::Node> OrderedPlanner::newNode(gsz graph_sizes, const std::vector<int>& inds, float cost, float f_cost, float mu, const std::vector<float>& cost_set) {
    int p = Graph<float>::augmentedStateImage(inds, graph_sizes);
    return std::make_unique<Node>(p, cost, f_cost, mu, cost_set);
}

int OrderedPlanner::newNode(gsz graph_sizes, const std::vector<int>& inds, float cost, float f_cost, float mu, const std::vector<float>& cost_set, std::unordered_map<int, std::unique_ptr<Node>>& node_map) {
    int p = Graph<float>::augmentedStateImage(inds, graph_sizes);
    node_map.insert(std::make_pair(p, std::make_unique<Node>(p, cost, f_cost, mu, cost_set)));
    return p;
}

OrderedPlanner::Node* OrderedPlanner::newNode(const Node& node, std::unordered_map<int, std::unique_ptr<Node>>& node_map) {
    node_map.insert(std::make_pair(node.ind, std::make_unique<Node>(node.ind, node.cost, node.f_cost, node.mu, node.cost_set)));
    return node_map.at(node.ind).get(); 
}


bool OrderedPlanner::allAccepting(gsz graph_sizes, int p, const std::vector<DFA_EVAL*>& dfas) {

    // Get individual graph node indices and set curr node:
    std::vector<int> ret_inds;
    Graph<float>::augmentedStatePreImage(graph_sizes, p, ret_inds);
    
    for (int i=0; i<dfas.size(); ++i) {
        if (!dfas[i]->getDFA()->isAccepting(ret_inds[i+1])) {
            return false;
        }
    }
    return true;
}

bool OrderedPlanner::generateHeuristic(const std::vector<DFA_EVAL*>& dfas) {
    heuristic.second.clear();
    heuristic.second.resize(dfas.size());
    for (int i=0; i<dfas.size(); ++i) {
        std::vector<DFA_EVAL*> dfa = {dfas[i]};

        std::vector<int> single_graph_sizes = SymbolicMethods::getGraphSizes(ts, dfa);
        std::vector<int> acc_nodes = BFS(dfa);
        searchBackwards(dfa, acc_nodes, heuristic.second[i]);
        if (!heuristic.second[i].success) {
            std::cout<<"Error (generateHeuristic): Backwards search for dfa "<<i<<" failed!\n";
            return false;
        }
    }
    heuristic.first = true;
    return true;
}


float OrderedPlanner::getH(gsz graph_sizes, unsigned p) const {
    if (graph_sizes.size() - 1 == heuristic.second.size()) {
        std::vector<int> ret_inds;
        Graph<float>::augmentedStatePreImage(graph_sizes, p, ret_inds);

        // Determine the maximum min-cost-to-goal for each single product:
        float max_h_val = 0.0f;
        for (int i=0; i<graph_sizes.size()-1; ++i) {
            int p_i = Graph<float>::augmentedStateImage({ret_inds[0], ret_inds[i+1]}, {graph_sizes[0], graph_sizes[i+1]});
            if (heuristic.second[i].reachable[p_i]) {
                float c_h_val = heuristic.second[i].cost.at(p_i); // Check bounds
                if (max_h_val < c_h_val) {
                    max_h_val = c_h_val;
                }
            } else {
                std::cout<<"Error (getH): h-val for state: (ts: "<<ret_inds[0]<<", dfa "<<i<<": "<<ret_inds[i+1]<<" is not reachable!\n";
            }
        }
        return max_h_val;
    } else {
        std::cout<<"Error (getH): Heuristic was either not generated or scenario is mismatched\n";
        return 0.0f;
    }
}

template<typename Q>
void printQ(Q queue) {
	while (!queue.empty()) {
		std::cout<<" --Ind: "<<queue.top()->ind<<" mu: "<<queue.top()->mu<<" cost_set: ";
        for (auto item : queue.top()->cost_set) std::cout<<" "<<item;
        std::cout<<"\n";
		queue.pop();
	}
}

bool OrderedPlanner::search(const std::vector<DFA_EVAL*>& dfas, const std::function<float(const std::vector<float>&)>& setToMu, bool use_heuristic, bool single_query, float mu_sq) {
    /*
    Generates a pareto front between priority order and total path cost for a given planning scenario
    */

    // Set up the transition system:
	std::vector<const std::vector<std::string>*> total_alphabet(dfas.size());
	for (int i=0; i<dfas.size(); ++i) {
		total_alphabet[i] = dfas[i]->getAlphabetEVAL();
	}
    ts.mapStatesToLabels(total_alphabet);
    std::vector<int> graph_sizes = SymbolicMethods::getGraphSizes(ts, dfas);
    int p_space_size = 1;
    for (auto sz : graph_sizes) {
        p_space_size *= sz;
    }

    // If using heuristic, generate it:
    if (use_heuristic) {
        if (bm_filepath) bm.pushStartPoint("heuristic");
        bool h_success = generateHeuristic(dfas);
        if (!h_success) {
            std::cout<<"Error (search): Heuristic failed to generate\n";
            return false; 
        }
        if (bm_filepath) bm.measureMilli("heuristic");
    }

    // Node check and pq structures:
    std::unordered_map<int, bool> seen; // Checks if the node has ever been encountered 
    std::unordered_map<int, ParentNode> parents; // Holds parent node and action
    auto compareFCost  = [](const Node* p1, const Node* p2) {return p1->f_cost > p2->f_cost;};
    std::priority_queue<Node*, std::vector<Node*>, decltype(compareFCost)> pq(compareFCost); // Regular search queue
    struct FrontierNode {
        FrontierNode() {}
        FrontierNode(Node* node_, float mu_lower_) : node(node_), mu_lower(mu_lower_) {}
        Node* node;
        float mu_lower; // Lowest mu value in posteior set
    };
    std::vector<FrontierNode> frontier; // Stack that holds frontier nodes to be searched in the next iteration
    std::unordered_map<int, std::unique_ptr<Node>> node_map; // incorperates min cost

    // Init parameters used in search:
    result.clear();
    bool solution_found = false;
    int iterations = 0;
    success = false; // member variable
    float mu_max = (single_query) ? mu_sq : -1.0f;
    bool is_inf = (mu_max == -1.0f) ? true : false;


    // Create the init root node:
	std::vector<int> init_node_inds(dfas.size() + 1);
    std::vector<float> init_cost_set(dfas.size(), 0.0f);
	init_node_inds[0] = ts.getInitStateInd();
	for (int i=0; i<dfas.size(); ++i) {
		dfas[i]->reset();
		init_node_inds[i+1] = dfas[i]->getCurrNode();
	}
    int p_init = newNode(graph_sizes, init_node_inds, 0.0f, 0.0f, 0.0f, init_cost_set, node_map);
    seen[p_init] = true;
    parents[p_init] = {-1, "none"}; // No parent for init node

    pq.push(node_map[p_init].get());

    // Benchmark the entire search time:
    if (bm_filepath) {
        bm.pushStartPoint("search");
    }
    std::map<float, std::pair<double, int>> bm_cost_to_time;

    while (!pq.empty()) {
        Node* curr_leaf = pq.top();
        int p = curr_leaf->ind;
        pq.pop();

        // If the popped node does not satisfy mu constraint, simply remove from queue:
        float mu_p = curr_leaf->mu;
        if (!is_inf && mu_p >= mu_max) continue;
        iterations++;

        // Check if current node is a solution:
        bool all_accepting = allAccepting(graph_sizes, p, dfas);
        if (all_accepting) {
            mu_max = mu_p;
            Plan pl = extractPlan(graph_sizes, p, p_init, parents);
            result.addParetoPoint(mu_max, curr_leaf->cost, pl);
            is_inf = false;
            success = true;
            
            // If single query search, return after first solution is found:
            if (single_query) {
                result.iterations = iterations;
                if (verbose) std::cout<<"Single query iterations: "<<iterations<<std::endl;
                if (bm_filepath) {
                    bm.measureMilli("search");
                    bm.addCustomTimeAttr("sq_iterations", static_cast<double>(iterations), ""); // No units
                    bm.pushAttributesToFile();
                }
                success = true;
                return true;
            }
            
            if (bm_filepath) {
                bm_cost_to_time[curr_leaf->cost] = {bm.measureMilli("search", false), iterations};
            }
            
            // Add all frontier nodes to the queue, then restart search from frontier:
            while (!frontier.empty()) {
                if (frontier.back().mu_lower < mu_max) {
                    pq.push(frontier.back().node);
                    frontier.pop_back();
                } else {
                    frontier.pop_back();
                }
            }
            continue;
        }

        // Get connected product nodes:
        auto inclMe = [](int pp) {
            return true;
        }; 
        SymbolicMethods::ConnectedNodes con_nodes = SymbolicMethods::postNodes(ts, dfas, {p}, inclMe);

        // Cycle through all connected nodes:
        bool is_on_frontier = false;
        FrontierNode fn(curr_leaf, mu_max); // Init lower bound as largest quantity:
        for (int i=0; i<con_nodes.nodes.size(); ++i) {
            int pp = con_nodes.nodes[i];
            WL* edge = con_nodes.data[i];
            
            // Get individual graph node indices and set curr node:
            std::vector<int> ret_inds;
            Graph<float>::augmentedStatePreImage(graph_sizes, p, ret_inds);

            // Check if all nodes are accepting, if not add appropriate costs:
            float new_cost = curr_leaf->cost + edge->weight;
            Node node_candidate(pp, new_cost, new_cost, -1.0f, curr_leaf->cost_set); // Temp value for mu
            for (int i=0; i<dfas.size(); ++i) {
                if (!dfas[i]->getDFA()->isAccepting(ret_inds[i+1])) {
                    node_candidate.cost_set[i] += edge->weight;
                }
            }

            // Prune nodes:
            node_candidate.mu = setToMu(node_candidate.cost_set);
            if (!is_inf && node_candidate.mu >= mu_max) {
                continue;
            }

            // Add heuristic value:
            if (use_heuristic) {
                node_candidate.f_cost += getH(graph_sizes, pp);
            } 

            // Check if node was seen and a shorter path was found:
            std::pair<bool, Node*> updated = {false, nullptr};
            if (seen[pp]) {
                if (!is_inf && node_map.at(pp)->mu >= mu_max) {
                    updated.first = true;
                } else if (node_candidate.cost > node_map.at(pp)->cost && node_candidate.mu < node_map.at(pp)->mu) {
                    // A higher-cost, lower-mu candidate was found, making the parent node on the frontier:
                    is_on_frontier = true;
                    // Adjust the frontier mu accordingly:
                    if (fn.mu_lower > node_candidate.mu || fn.mu_lower == -1.0f) fn.mu_lower = node_candidate.mu;
                    continue;
                } else if ((node_candidate.cost <= node_map.at(pp)->cost && node_candidate.mu <= node_map.at(pp)->mu) &&
                        (node_candidate.cost != node_map.at(pp)->cost || node_candidate.mu != node_map.at(pp)->mu)) {
                    // A lower-cost and/or lower-mu candidate was found, making the candidate strictly better, thus update:
                    updated.first = true;
                } else if (node_candidate.cost < node_map.at(pp)->cost && node_candidate.mu > node_map.at(pp)->mu) {
                    // A lower-cost, higher-mu candidate was found, therefore update to the shorter path, but now the parent is on the frontier:
                    updated.first = true;

                    // Find the min mu in the post set of the parent:
                    SymbolicMethods::ConnectedNodes con_nodes_min_mu = SymbolicMethods::postNodes(ts, dfas, {parents[pp].par_ind}, inclMe);
                    float min_mu = node_candidate.mu;
                    int p_par = parents[pp].par_ind;
                    for (const auto& pp_par : con_nodes_min_mu.nodes) {

                        // Get individual graph node indices and set curr node:
                        std::vector<int> ret_inds;
                        Graph<float>::augmentedStatePreImage(graph_sizes, p_par, ret_inds);

                        // Check if all nodes are accepting, if not add appropriate costs:
                        std::vector<float> test_set = node_map.at(p_par)->cost_set;
                        for (int i=0; i<dfas.size(); ++i) {
                            if (!dfas[i]->getDFA()->isAccepting(ret_inds[i+1])) {
                                test_set[i] += edge->weight;
                            }
                        }
                        if (setToMu(test_set) < min_mu) min_mu = node_map.at(pp_par)->mu;
                    }
                    frontier.emplace_back(node_map.at(p_par).get(), min_mu);
                } else {
                    continue;
                }
                if (updated.first) {
                    parents[pp] = {p, edge->label};
                    (*node_map[pp]) = node_candidate;
                    updated.second = node_map[pp].get();
                }
            }

            // Made it thru all checks, add the node to the graph and queue
            // (store nodes on the heap to handle massive graphs):
            if (!updated.first) { // If not updated, name a new node
                Node* new_node = newNode(node_candidate, node_map);
                pq.push(new_node);
                seen[pp] = true;
                parents[pp] = {p, edge->label};
            } else { // If updated (A*) push the seen node back into the queue
                pq.push(updated.second);
            }
        }    
        if (is_on_frontier) frontier.push_back(fn);
    }
    if (verbose) std::cout<<"Iterations: "<<iterations<<std::endl;
    
    // Add stored benchmarks:
    if (bm_filepath) {
        for (const auto& pt : *(result.getParetoFront())) {
            bm.addAttribute("single_point_search", std::to_string(bm_cost_to_time.at(pt.path_length).first), std::to_string(pt.mu));
            bm.addAttribute("single_point_search_iterations", std::to_string(bm_cost_to_time.at(pt.path_length).second), std::to_string(pt.mu));
        }
        bm.measureMilli("search");
        bm.addCustomTimeAttr("iterations", static_cast<double>(iterations), ""); // No units
        bm.pushAttributesToFile();
    }

    result.iterations = iterations;
    return success;
}


    struct OpenNode {
        OpenNode() : f_set(2), g_set(2) {}
        OpenNode(int p_ind_, int node_ind_, const LexSet& f_set_, const std::vector<float>& g_set_, const std::vector<float>& cost_vector_) : p_ind(p_ind_), node_ind(node_ind_), f_set(2), g_set(g_set_), cost_vector(cost_vector_) {
            f_set = f_set_;
        }
        int p_ind;
        int node_ind;
        LexSet f_set; 
        std::vector<float> g_set;
        std::vector<float> cost_vector;
    };
void printON(OpenNode* n) {
    std::cout<<"P Ind: "<<n->p_ind<<std::endl;
    std::cout<<"P Ind: "<<n->node_ind<<std::endl;
    std::cout<<"F LexSet: "<<std::endl;
    n->f_set.print();
    std::cout<<"G Set: "<<std::endl;
    for (auto g : n->g_set) std::cout<<"  - "<<g<<std::endl;
}
template<typename Q>
void printQueueON(Q queue) {
	int toomuch = 0;
    std::cout<<"~~~~~Printing q:"<<std::endl;
	while (!queue.empty()) {
        std::cout<<" Element: "<<toomuch<<std::endl;
		toomuch++;

        printON(queue.top());
        queue.pop();
		if (toomuch > 50) {
			break;
		}
	}
}



bool OrderedPlanner::NAMOA(const std::vector<DFA_EVAL*>& dfas, const std::function<float(const std::vector<float>&)>& setToMu, bool use_heuristic, bool single_query, float mu_sq) {
    /*
    Generates a pareto front between priority order and total path cost for a given planning scenario
    */

    // Set up the transition system:
	std::vector<const std::vector<std::string>*> total_alphabet(dfas.size());
	for (int i=0; i<dfas.size(); ++i) {
		total_alphabet[i] = dfas[i]->getAlphabetEVAL();
	}
    ts.mapStatesToLabels(total_alphabet);
    std::vector<int> graph_sizes = SymbolicMethods::getGraphSizes(ts, dfas);
    int p_space_size = 1;
    for (auto sz : graph_sizes) {
        p_space_size *= sz;
    }

    // If using heuristic, generate it:
    if (use_heuristic) {
        if (bm_filepath) bm.pushStartPoint("heuristic");
        bool h_success = generateHeuristic(dfas);
        if (!h_success) {
            std::cout<<"Error (search): Heuristic failed to generate\n";
            return false; 
        }
        if (bm_filepath) bm.measureMilli("heuristic");
    }

    // Node check and pq structures:
    std::unordered_map<int, bool> seen; // Checks if the node has ever been encountered 

    struct ParentOpenNode {
        OpenNode* node;
        std::string action;
    };
    std::vector<std::unique_ptr<OpenNode>> node_list;
    std::unordered_map<int, ParentOpenNode> parents; // Holds parent node and action
    std::unordered_map<int, float> g_min; // Holds parent node and action
    std::unordered_map<int, bool> is_g_min_bounded; // Holds parent node and action
    auto compareFCost  = [](const OpenNode* p1, const OpenNode* p2) {return p1->f_set > p2->f_set;};
    std::priority_queue<OpenNode*, std::vector<OpenNode*>, decltype(compareFCost)> pq(compareFCost); // Regular search queue
    //std::vector<FrontierNode> frontier; // Stack that holds frontier nodes to be searched in the next iteration
    //std::unordered_map<int, std::unique_ptr<Node>> node_map; // incorperates min cost

    // Init parameters used in search:
    result.clear();
    bool solution_found = false;
    int iterations = 0;
    success = false; // member variable
    float mu_max = (single_query) ? mu_sq : -1.0f;
    bool is_inf = (mu_max == -1.0f) ? true : false;


    // Create the init root node:
	std::vector<int> init_node_inds(dfas.size() + 1);
    std::vector<float> init_cost_set(dfas.size(), 0.0f);
	init_node_inds[0] = ts.getInitStateInd();
	for (int i=0; i<dfas.size(); ++i) {
		dfas[i]->reset();
		init_node_inds[i+1] = dfas[i]->getCurrNode();
	}
    int p_init = Graph<int>::augmentedStateImage(init_node_inds, graph_sizes);
    //int p_init = newNode(graph_sizes, init_node_inds, 0.0f, 0.0f, 0.0f, init_cost_set, node_map);
    seen[p_init] = true;
    int curr_node_ind = 0;
    LexSet init_set(2);
    std::vector<float> init_cost_v(dfas.size(), 0.0f);
    std::vector<float> init_g(2, 0.0f);
    std::unique_ptr<OpenNode> init_onode = std::make_unique<OpenNode>(p_init, 0, init_set, init_g, init_cost_v);
    node_list.push_back(std::move(init_onode));
    parents[0] = {node_list.back().get(), "none"}; // No parent for init node

    pq.push(node_list.back().get());

    // Benchmark the entire search time:
    if (bm_filepath) {
        bm.pushStartPoint("boa_search");
    }
    std::map<float, std::pair<double, int>> bm_cost_to_time;

    float g_min_sgoal = -1.0f;
    while (!pq.empty()) {

        //printQueueON(pq);

        OpenNode* curr_leaf = pq.top();
        int p = curr_leaf->p_ind;
        pq.pop();

        //std::cout<<"selected element: "<<curr_leaf->node_ind<<std::endl;
        //int pause;
        //std::cin>>pause;

        //std::cout<<"iteration: "<<iterations<<" qsize: "<<pq.size()<<std::endl;
        //std::cout<<"seen size: "<<seen.size()<<" pspace size: "<< p_space_size<<std::endl;

        // If the popped node does not satisfy mu constraint, simply remove from queue:
        //float mu_p = curr_leaf->mu;
        if ((is_g_min_bounded[p] && curr_leaf->g_set[1] >= g_min.at(p)) || (g_min_sgoal >= 0.0f && curr_leaf->f_set.get()[1] >= g_min_sgoal)) {
        //if ((is_g_min_bounded[p] && curr_leaf->g_set[1] >= g_min.at(p))) {
            //std::cout<<"continuing"<<std::endl;
            continue;
        }
        iterations++;
        g_min[p] = curr_leaf->g_set[1];
        is_g_min_bounded[p] = true;

        // Check if current node is a solution:
        bool all_accepting = allAccepting(graph_sizes, p, dfas);
        if (all_accepting) {
            //std::cout<<"        IS ACCEPTING"<<std::endl;
            //mu_max = mu_p;

            Plan pl;//= extractPlanNAMOA(graph_sizes, p, p_init, parents);
            result.addParetoPoint(curr_leaf->g_set[1], curr_leaf->g_set[0], pl);

            //is_inf = false;
            success = true;
            
            // If single query search, return after first solution is found:
            if (single_query) {
                result.iterations = iterations;
                if (verbose) std::cout<<"Single query iterations: "<<iterations<<std::endl;
                if (bm_filepath) {
                    std::cout<<"IN SINGLE QUERY"<<std::endl;
                    bm.measureMilli("boa_search");
                    bm.addCustomTimeAttr("sq_iterations", static_cast<double>(iterations), ""); // No units
                    bm.pushAttributesToFile();
                }
                success = true;
                return true;
            }
            
            if (bm_filepath) {
                bm_cost_to_time[curr_leaf->g_set[0]] = {bm.measureMilli("boa_search", false), iterations};
            }
            
            //// Add all frontier nodes to the queue, then restart search from frontier:
            //while (!frontier.empty()) {
            //    if (frontier.back().mu_lower < mu_max) {
            //        pq.push(frontier.back().node);
            //        frontier.pop_back();
            //    } else {
            //        frontier.pop_back();
            //    }
            //}
            g_min_sgoal = curr_leaf->g_set[1];
            continue;
        }

        // Get connected product nodes:
        auto inclMe = [](int pp) {
            return true;
        }; 
        SymbolicMethods::ConnectedNodes con_nodes = SymbolicMethods::postNodes(ts, dfas, {p}, inclMe);

        // Cycle through all connected nodes:
        //bool is_on_frontier = false;
        //FrontierNode fn(curr_leaf, mu_max); // Init lower bound as largest quantity:
        for (int i=0; i<con_nodes.nodes.size(); ++i) {
            int pp = con_nodes.nodes[i];
            WL* edge = con_nodes.data[i];
            
            // Get individual graph node indices and set curr node:
            std::vector<int> ret_inds;
            Graph<float>::augmentedStatePreImage(graph_sizes, p, ret_inds);

            // Check if all nodes are accepting, if not add appropriate costs:
            float new_cost = curr_leaf->g_set[0] + edge->weight;
            //Node node_candidate(pp, new_cost, new_cost, -1.0f, curr_leaf.cost_set); // Temp value for mu
            LexSet pp_ls(2);

            std::unique_ptr<OpenNode> new_node = std::make_unique<OpenNode>(pp, node_list.size(), pp_ls, curr_leaf->g_set, curr_leaf->cost_vector);
            node_list.push_back(std::move(new_node));
            OpenNode* nn = node_list.back().get();
            //OpenNode node_candidate(pp, p, edge->label, pp_ls, curr_leaf.g_set, curr_leaf.cost_vector);


            for (int i=0; i<dfas.size(); ++i) {
                if (!dfas[i]->getDFA()->isAccepting(ret_inds[i+1])) {
                    nn->cost_vector[i] += edge->weight;
                }
            }
            // Add cost vector:
            nn->g_set[0] = new_cost;
            nn->g_set[1] = setToMu(nn->cost_vector);

            // Add heuristic value:
            if (use_heuristic) {
                std::vector<float> new_f_set = {nn->g_set[0] + getH(graph_sizes, pp), nn->g_set[1]};
                nn->f_set = new_f_set;
            } else {
                std::vector<float> new_f_set = {nn->g_set[0], nn->g_set[1]};
                nn->f_set = new_f_set;
            }

            //std::cout<<"------------> connected node: "<<std::endl;
            //printON(nn);

            parents[node_list.size()-1] = {nn, edge->label};
            if ((is_g_min_bounded[pp] && nn->g_set[1] >= g_min.at(pp)) || (g_min_sgoal >= 0.0f && nn->f_set.get()[1] >= g_min_sgoal)) {
            //if ((is_g_min_bounded[pp] && nn->g_set[1] >= g_min.at(pp))) {
                continue;
            }
            seen[pp] = true;
            pq.push(nn);
            //std::cout<<"pushing..."<<std::endl;


            //// Prune nodes:
            //node_candidate.mu = setToMu(node_candidate.cost_set);
            //if (!is_inf && node_candidate.mu >= mu_max) {
            //    continue;
            //}


            // Check if node was seen and a shorter path was found:
            //std::pair<bool, Node*> updated = {false, nullptr};
            //if (seen[pp]) {
            //    if (!is_inf && node_map.at(pp)->mu >= mu_max) {
            //        updated.first = true;
            //    } else if (node_candidate.cost > node_map.at(pp)->cost && node_candidate.mu < node_map.at(pp)->mu) {
            //        // A higher-cost, lower-mu candidate was found, making the parent node on the frontier:
            //        is_on_frontier = true;
            //        // Adjust the frontier mu accordingly:
            //        if (fn.mu_lower > node_candidate.mu || fn.mu_lower == -1.0f) fn.mu_lower = node_candidate.mu;
            //        continue;
            //    } else if ((node_candidate.cost <= node_map.at(pp)->cost && node_candidate.mu <= node_map.at(pp)->mu) &&
            //            (node_candidate.cost != node_map.at(pp)->cost || node_candidate.mu != node_map.at(pp)->mu)) {
            //        // A lower-cost and/or lower-mu candidate was found, making the candidate strictly better, thus update:
            //        updated.first = true;
            //    } else if (node_candidate.cost < node_map.at(pp)->cost && node_candidate.mu > node_map.at(pp)->mu) {
            //        // A lower-cost, higher-mu candidate was found, therefore update to the shorter path, but now the parent is on the frontier:
            //        updated.first = true;

            //        // Find the min mu in the post set of the parent:
            //        SymbolicMethods::ConnectedNodes con_nodes_min_mu = SymbolicMethods::postNodes(ts, dfas, {parents[pp].par_ind}, inclMe);
            //        float min_mu = node_candidate.mu;
            //        int p_par = parents[pp].par_ind;
            //        for (const auto& pp_par : con_nodes_min_mu.nodes) {

            //            // Get individual graph node indices and set curr node:
            //            std::vector<int> ret_inds;
            //            Graph<float>::augmentedStatePreImage(graph_sizes, p_par, ret_inds);

            //            // Check if all nodes are accepting, if not add appropriate costs:
            //            std::vector<float> test_set = node_map.at(p_par)->cost_set;
            //            for (int i=0; i<dfas.size(); ++i) {
            //                if (!dfas[i]->getDFA()->isAccepting(ret_inds[i+1])) {
            //                    test_set[i] += edge->weight;
            //                }
            //            }
            //            if (setToMu(test_set) < min_mu) min_mu = node_map.at(pp_par)->mu;
            //        }
            //        frontier.emplace_back(node_map.at(p_par).get(), min_mu);
            //    } else {
            //        continue;
            //    }
            //    if (updated.first) {
            //        parents[pp] = {p, edge->label};
            //        (*node_map[pp]) = node_candidate;
            //        updated.second = node_map[pp].get();
            //    }
            //}

            //// Made it thru all checks, add the node to the graph and queue
            //// (store nodes on the heap to handle massive graphs):
            //if (!updated.first) { // If not updated, name a new node
            //    Node* new_node = newNode(node_candidate, node_map);
            //    pq.push(new_node);
            //    seen[pp] = true;
            //    parents[pp] = {p, edge->label};
            //} else { // If updated (A*) push the seen node back into the queue
            //    pq.push(updated.second);
            //}
        }    
        //if (is_on_frontier) frontier.push_back(fn);
    }
    if (verbose) std::cout<<"Iterations: "<<iterations<<std::endl;
    
    // Add stored benchmarks:
    if (bm_filepath) {
        for (const auto& pt : *(result.getParetoFront())) {
            bm.addAttribute("single_point_search", std::to_string(bm_cost_to_time.at(pt.path_length).first), std::to_string(pt.mu));
            bm.addAttribute("single_point_search_iterations", std::to_string(bm_cost_to_time.at(pt.path_length).second), std::to_string(pt.mu));
        }
        bm.measureMilli("boa_search");
        bm.addCustomTimeAttr("boa_iterations", static_cast<double>(iterations), ""); // No units
        bm.pushAttributesToFile();
    }

    result.iterations = iterations;
    return success;
}

OrderedPlanner::Plan OrderedPlanner::extractPlan(gsz graph_sizes, int p_acc, int p_init, const std::unordered_map<int, ParentNode>& parents) {
    int curr_ind = p_acc;
    Plan plan; 
    Plan reverse_plan;
    std::vector<int> reverse_state_inds;
    while (curr_ind != p_init) {
        // Get individual graph node indices and set curr node:
        std::vector<int> ret_inds;
        Graph<float>::augmentedStatePreImage(graph_sizes, curr_ind, ret_inds);
        reverse_plan.action_sequence.push_back(parents.at(curr_ind).par_action);
        reverse_plan.state_sequence.push_back(ts.getState(ret_inds[0]));
        reverse_state_inds.push_back(ret_inds[0]);
        curr_ind = parents.at(curr_ind).par_ind;
    }
    std::vector<int> ret_inds;
    Graph<float>::augmentedStatePreImage(graph_sizes, curr_ind, ret_inds);
    reverse_plan.state_sequence.push_back(ts.getState(ret_inds[0])); // Init state
    reverse_state_inds.push_back(ret_inds[0]);
    
    // Reverse the plan:
    plan.action_sequence.resize(reverse_plan.action_sequence.size());
    plan.state_sequence.resize(reverse_plan.state_sequence.size());

    int i = 0;
    if (verbose) std::cout<<"Extracting plan... \n - Printing action sequence: ";
    for (auto rit = reverse_plan.action_sequence.rbegin(); rit != reverse_plan.action_sequence.rend(); ++rit) {
        plan.action_sequence[i] = *rit;
        if (verbose) std::cout<<" -> "<<*rit;
        i++;
    }
    if (verbose) std::cout<<"\n";

    i = 0;
    if (verbose) {
        std::cout<<" - Printing state index sequence: "; 
        for (auto rit = reverse_state_inds.rbegin(); rit != reverse_state_inds.rend(); ++rit) { 
            std::cout<<" -> "<<*rit;
        }
        std::cout<<"\n";
        i++;
    }
    
    i = 0;
    for (auto rit = reverse_plan.state_sequence.rbegin(); rit != reverse_plan.state_sequence.rend(); ++rit) {
        plan.state_sequence[i] = *rit;
        i++;
    }
    return plan;
}

const OrderedPlanner::Result* OrderedPlanner::getResult() const {
    return (success) ? &result : nullptr;
}

std::vector<int> OrderedPlanner::BFS(const std::vector<DFA_EVAL*>& dfas) {
    /*
    Breath first search that finds and returns all accepting reachable product states
    */

    std::vector<int> accepting_states;

    std::vector<int> graph_sizes = SymbolicMethods::getGraphSizes(ts, dfas);

    // Node check and pq structures:
    std::unordered_map<int, bool> visited; // Checks if the node has been visited (checked for acceptance)
    std::unordered_map<int, bool> seen; // Checks if the node has been encountered

    // Create the init root node:
	std::vector<int> init_node_inds(dfas.size() + 1);
	init_node_inds[0] = ts.getInitStateInd();
	for (int i=0; i<dfas.size(); ++i) {
		dfas[i]->reset();
		init_node_inds[i+1] = dfas[i]->getCurrNode();
	}
    int p_init = Graph<float>::augmentedStateImage(init_node_inds, graph_sizes);
    visited[p_init] = true;
    seen[p_init] = true;

    std::queue<int> q;
    q.push(p_init);
    while (!q.empty()) {
        int p = q.front();
        q.pop();

        // Check if current node is accepting, if so add to solution pool
        if (allAccepting(graph_sizes, p, dfas)) {
            accepting_states.push_back(p);
        }
        
        visited[p] = true;

        // Get connected product nodes:
        auto inclMe = [&seen](int pp) {
            // If node was already seen, dont include it:
            return (seen[pp]) ? false : true;
        }; 
        std::vector<int> post_set = SymbolicMethods::post(ts, dfas, {p}, inclMe);
        
        // Add new nodes to queue and mark as seen:
        for (auto pp : post_set) {
            seen[pp] = true;
            q.push(pp);
        }
    }
    return accepting_states;
}

void OrderedPlanner::searchBackwards(const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& root_acc_nodes, CostToGoal& cost_to_goal) {
    /*
    Generates min-cost-to-goal weighting for each backwards reachable product state in scenario
    */

    // Assume transition labels were already mapped:
    std::vector<int> graph_sizes = SymbolicMethods::getGraphSizes(ts, dfas);
    int p_space_size = 1;
    for (auto sz : graph_sizes) {
        p_space_size *= sz;
    }
    cost_to_goal.clear();
    cost_to_goal.resize(p_space_size); // Incorperates 'seen' and 'min_cost'
    cost_to_goal.success = false;

    // Node check and pq structures (use arrays since the entire backwards-reachable graph will be searched):
    std::vector<bool> visited(p_space_size, false); // Checks if the node has been visited
    std::vector<std::pair<int, std::string>> children(p_space_size); // Holds parent node and action
    auto compare  = [](const Node* p1, const Node* p2) {return p1->f_cost > p2->f_cost;};
    std::priority_queue<Node*, std::vector<Node*>, decltype(compare)> pq(compare);
    std::unordered_map<int, std::unique_ptr<Node>> node_map;

    // Create the init root accepting nodes:
    Node init_fill_node;
    init_fill_node.cost = 0.0f;
    init_fill_node.cost_set = {};
    init_fill_node.f_cost = 0.0f;
    for (auto p_acc : root_acc_nodes) {
        init_fill_node.ind = p_acc;
        Node* init_node_ptr = newNode(init_fill_node, node_map);
        visited[p_acc] = true;
        children[p_acc] = {-1, "none"}; // No child for init node
        cost_to_goal.cost[p_acc] = 0.0f;
        cost_to_goal.reachable[p_acc] = true;
        pq.push(init_node_ptr);
    }

    // Init parameters used in search:
    bool solution_found = false;
    int iterations = 0;
    success = false; // member variable

    while (!pq.empty()) {
        iterations++;
        Node* curr_leaf = pq.top();
        int pp = curr_leaf->ind;
        pq.pop();
        visited[pp] = true;

        // Get pre connected product nodes:
        auto inclMe = [&visited](int p) {
            // Dont consider visited nodes:
            return (visited[p]) ? false : true;
        }; 
        SymbolicMethods::ConnectedNodes con_nodes = SymbolicMethods::preNodes(ts, dfas, {pp}, inclMe);

        // Cycle through all connected nodes:
        for (int i=0; i<con_nodes.nodes.size(); ++i) {
            int p = con_nodes.nodes[i];
            WL* edge = con_nodes.data[i];
            
            // Get individual graph node indices and set curr node:
            std::vector<int> con_ret_inds;
            Graph<float>::augmentedStatePreImage(graph_sizes, p, con_ret_inds);

            // Check if all nodes are accepting, if not add appropriate costs:
            float new_cost = curr_leaf->cost + edge->weight;
            Node node_candidate(p, new_cost, new_cost, 0.0f, {});

            // Check if node was seen and a shorter path was found:
            bool updated = false;
            if (cost_to_goal.reachable[p]) {
                if (node_candidate.cost < cost_to_goal.cost[p]) {
                    children[p] = {pp, edge->label};
                    node_map[p]->cost = node_candidate.cost;
                    node_map[p]->f_cost = node_candidate.f_cost;
                    cost_to_goal.cost[p] = node_candidate.cost;
                    updated = true;
                } 
                continue;
            }

            // Made it thru all checks, add the node to the graph and queue
            // (store nodes on the heap to handle massive graphs):
            Node* new_node = newNode(node_candidate, node_map);
            pq.push(new_node);
            cost_to_goal.reachable[p] = true;
            cost_to_goal.cost[p] = node_candidate.cost;
            children[p] = {pp, edge->label};
        }    
    }
    cost_to_goal.success = true;
}

