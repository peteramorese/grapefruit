#include<fstream>
#include<chrono>
#include<ctime>
#include<unordered_map>
#include<memory>
#include "symbSearch.h"
#include "benchmark.h"

using std::cout;
using std::endl;
//template<class T> using shptr = std::shared_ptr<T>;
//template<class T> using unptr = std::unique_ptr<T>;
//template<class T> using wkptr = std::weak_ptr<T>;

SymbSearch::PlanResult::PlanResult(int num_dfas, float mu) : pathcost(num_dfas, mu) {}

SymbSearch::StrategyResult::StrategyResult(int graph_size) : reachability(graph_size, false), action_map(graph_size), success(false) {}

SymbSearch::SymbSearch() : 
	benchmark(nullptr), 
	use_benchmark(false),
	verbose(false), 
	dfas_set(false), 
	TS_set(false), 
	mu_set(false) {}

SymbSearch::SymbSearch(const std::string* bench_mark_session_, bool verbose_) : 
	benchmark(bench_mark_session_), 
	verbose(verbose_), 
	dfas_set(false), 
	TS_set(false), 
	mu_set(false) {

	if (bench_mark_session_) {
		use_benchmark = true;
	} else {
		use_benchmark = false;
	}
}

void SymbSearch::setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_) {
	//node_list.clear();
	//set_list.clear();
	dfa_list_ordered = dfa_list_ordered_;
	num_dfas = dfa_list_ordered->size();
	dfas_set = true;
}

void SymbSearch::setTransitionSystem(TS_EVAL<State>* TS_) {
	TS = TS_;	
	TS_set = true;
}

void SymbSearch::setFlexibilityParam(float mu_) {
	mu = mu_;
	mu_set = true;
}


template<typename LS>
std::weak_ptr<IVFlexLex<LS>> SymbSearch::newNode(unsigned num_dfas, float mu, std::vector<std::shared_ptr<IVFlexLex<LS>>>& node_container) {
	std::shared_ptr<IVFlexLex<LS>> node_i = std::make_shared<IVFlexLex<LS>>(num_dfas, mu);
	node_container.push_back(std::move(node_i));
	std::weak_ptr<IVFlexLex<LS>> ret_node_i = node_container.back();
	return ret_node_i;
}

//DetourLex* SymbSearch::newSet() {
//	DetourLex* set_i = new DetourLex(mu, num_dfas);
//	set_list.push_back(set_i);
//	return set_i;
//}

//LexSet* SymbSearch::newSetLS(unsigned set_size) {
//	LexSet* set_i = new LexSet(0.0f, set_size);
//	set_list_ls.push_back(set_i);
//	return set_i;
//}

template<typename LS>
std::weak_ptr<LS> SymbSearch::newSet(unsigned set_size, std::vector<std::shared_ptr<LS>>& set_container) {
	std::shared_ptr<LS> set_i = std::make_shared<LS>(set_size, 0.0f);
	set_container.push_back(std::move(set_i));
	std::weak_ptr<LS> ret_set_i = set_container.back();
	return ret_set_i;
}

template<typename Q>
void SymbSearch::printQueue(Q queue) {
	int toomuch = 0;
	while (!queue.empty()) {
		cout<<"Ind: "<<queue.top().first<<endl;
		cout<<"LexSet: ";
		queue.top().second->print();
		queue.pop();
		toomuch++;
		if (toomuch > 50) {
			break;
		}
	}
}

template<typename Q_f>
void SymbSearch::printQueueFloat(Q_f queue) {
	while (!queue.empty()) {
		cout<<"Ind: "<<queue.top().first<<", Weight: "<<queue.top().second<<endl;
		queue.pop();
	}
}

bool SymbSearch::spaceSearch(TS_EVAL<State>* TS_sps, std::vector<DFA_EVAL*>* dfa_list_sps, spaceWeight& spw, std::function<float(float, unsigned int)> spwFunc, int max_depth) {

	/* Arguments:

	- TS_sps: Transition system
	- dfa_list_sps: List of dfas in product
	- spw: Space weight (product state weights)
	- spwFunc: Space weight function: spwFunc(float ts_cost_to_goal, unsigned depth_to_goal)

	*/

	bool depth_limiting_g = (max_depth > 0) ? true : false;
	const int num_dfa_sps = dfa_list_sps->size();
	if (!TS_sps->isReversible()) {
		cout<<"Error: Cannot perform space search on irreversible graphs\n";
		return false;
	}
	for (int i=0; i<num_dfa_sps; ++i) {
		if (!dfa_list_sps->operator[](i)->getDFA()->isReversible()) {
			cout<<"Error: Cannot perform space search on irreversible graphs\n";
			return false;
		}
	} 

	std::vector<const std::vector<std::string>*> total_alphabet(num_dfa_sps);
	std::vector<int> graph_sizes;
	graph_sizes.push_back(TS_sps->size());
	int p_space_size = TS_sps->size();
	for (int i=0; i<num_dfa_sps; ++i) {
		total_alphabet[i] = dfa_list_sps->operator[](i)->getAlphabetEVAL();
		graph_sizes.push_back(dfa_list_sps->operator[](i)->getDFA()->size());
		p_space_size *= dfa_list_sps->operator[](i)->getDFA()->size();
	}
	spw.state_weights.resize(p_space_size, 0.0);
	spw.reachability.resize(p_space_size, false);
	
	minWeight min_w(p_space_size);

	auto compare = [](std::pair<int, float> pair1, std::pair<int, float> pair2) {
		return pair1.second > pair2.second;
	};

	bool exit_failure = false;
	std::pair<bool, std::vector<int>> accepting;
	for (int round=0; round<2; ++round) {
		bool depth_limiting = (round == 0) ? false : depth_limiting_g;
		std::string search_type = (round == 0) ? "forward" : "reverse";
		std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float>>, decltype(compare)> pq(compare);
		std::unordered_map<int, int> depth_map;

		// Tree to keep history as well as parent node list:
		bool found_target_state = false;
		std::vector<bool> visited(p_space_size, false);
		//Graph<float> tree;
		
		// Root tree node has index zero with weight zero
		std::pair<int, float> curr_leaf;
		if (round == 0) {
			TS_sps->reset();
			for (auto dfa_ptr : *(dfa_list_sps)) {
				dfa_ptr->reset();
			}
			std::vector<int> init_node_inds(num_dfa_sps + 1);
			init_node_inds[0] = TS_sps->getCurrNode();
			for (int i=0; i<num_dfa_sps; ++i) {
				init_node_inds[i+1] = dfa_list_sps->operator[](i)->getCurrNode();
			}
			int init_node_prod_ind = Graph<float>::augmentedStateImage(init_node_inds, graph_sizes);
			min_w.is_inf[init_node_prod_ind] = false;
			min_w.min_weight[init_node_prod_ind] = 0;
			visited[init_node_prod_ind] = true;
			curr_leaf.first = init_node_prod_ind;
			curr_leaf.second = 0.0f;
			pq.push(curr_leaf);
		} else {
			if (!accepting.first) {
				cout<<"Error: Did not find target in first search.\n";
				return false;
			}
			int ROOT_STATE = p_space_size; // this is the root state ind, guaranteed to be larger than any prod state ind
			min_w.reset();
			for (auto acc_prod_state : accepting.second) {
				min_w.is_inf[acc_prod_state] = false;
				min_w.min_weight[acc_prod_state] = 0;
				visited[acc_prod_state] = true;
				spw.state_weights[acc_prod_state] = spwFunc(0.0f, 0); // 0.0 edge wieght, 0 depth
				spw.reachability[acc_prod_state] = true;
				std::vector<int> sol_inds;
				Graph<float>::augmentedStatePreImage(graph_sizes, acc_prod_state, sol_inds);
				curr_leaf.first = acc_prod_state;
				curr_leaf.second = 0.0f;
				pq.push(curr_leaf);
				if (depth_limiting) {
					depth_map[acc_prod_state] = 0; // Depth of pin states is defined as zero
				}
				//tree.connect(ROOT_STATE, {acc_prod_state, nullptr}); // the root state is the merged accepting state
			}
		}
		float min_accepting_cost = -1;
		int prev_leaf_ind = -1;
		int prod_solution_ind;
		int iterations = 0;

		while (pq.size() > 0) {
			iterations++;
			curr_leaf = pq.top();
			pq.pop();
			int curr_leaf_ind = curr_leaf.first;
			int curr_leaf_depth;
			if (depth_limiting) {
				curr_leaf_depth = depth_map.at(curr_leaf_ind);
				cout<<"curr_leaf_depth: "<<curr_leaf_depth<<endl;
				if (curr_leaf_depth >= max_depth) {
					continue;
				}
			}
			float curr_leaf_weight = curr_leaf.second;
			if (!min_w.is_inf[curr_leaf_ind]) {
				if (curr_leaf_weight > min_w.min_weight[curr_leaf_ind]) {
					continue;
				}
			}
			visited[curr_leaf_ind] = true;
			std::vector<int> ret_inds;
			Graph<float>::augmentedStatePreImage(graph_sizes, curr_leaf_ind, ret_inds);
			TS_sps->set(ret_inds[0]);
			for (int i=0; i<num_dfa_sps; ++i) {
				dfa_list_sps->operator[](i)->set(ret_inds[i+1]);
			}
			std::vector<int> con_nodes;
			std::vector<WL*> con_data;
			con_data.clear();
			if (round == 0) {
				TS_sps->getConnectedDataEVAL(con_data);
				TS_sps->getConnectedNodesEVAL(con_nodes);
			} else {
				TS_sps->getParentDataEVAL(con_data);
				TS_sps->getParentNodesEVAL(con_nodes);
			
			}
			for (int j=0; j<con_data.size(); ++j) {
				TS_sps->set(ret_inds[0]);
				for (int i=0; i<num_dfa_sps; ++i) {
					dfa_list_sps->operator[](i)->set(ret_inds[i+1]);
				}
				std::string temp_str = con_data[j]->label;
				float temp_weight = con_data[j]->weight;
				bool found_connection = true;
				int prev_ts_ind;
				if (round == 0) {
					found_connection = TS_sps->eval(temp_str, true); // second arg tells wether or not to evolve on graph
				} else {
					prev_ts_ind = TS_sps->getCurrNode();
					found_connection = TS_sps->evalReverse(temp_str, true); // second arg tells wether or not to evolve on graph
				
				}
				if (!found_connection) {
					cout<<"Error ("<<search_type<<"): Did not find connectivity in TS. Current node: "<<TS_sps->getCurrNode()<<", Action: "<<temp_str<<endl;
					return false;
				}
				const std::vector<std::string>* lbls;
				if (round == 0){
					lbls = TS_sps->returnStateLabels(TS_sps->getCurrNode());
				} else {
					lbls = TS_sps->returnStateLabels(prev_ts_ind);
				}
				found_connection = false;
				bool found_true = true;
				std::vector<std::vector<int>> parent_node_list(num_dfa_sps); // Array of nodes arrays [(TS, DFA1, DFA2, ...), (), (), ...]
				if (round == 0) {
					for (int i=0; i<num_dfa_sps; ++i) {
						found_connection = false;
						for (int ii=0; ii<lbls->size(); ++ii) {
							if (dfa_list_sps->operator[](i)->eval(lbls->operator[](ii), true)) {
								found_connection = true;
								break;
							}
						}
						if (!found_connection) {
							cout<<"Error ("<<search_type<<"): Did not find connectivity in DFA. Current node: "<<TS_sps->getCurrNode()<<", Action: "<<temp_str<<endl;
							return false;
						}
					}
				} else {
					std::vector<std::vector<int>> temp_par_container(num_dfa_sps); // Array of lists of parent nodes for each DFA
					std::vector<int> node_list_sizes(num_dfa_sps);
					int parent_node_list_size = 1;
					for (int ii=0; ii<num_dfa_sps; ++ii) {
						found_connection = dfa_list_sps->operator[](ii)->getParentNodesWithLabels(lbls, temp_par_container[ii]);
						parent_node_list_size *= temp_par_container[ii].size();
						node_list_sizes[ii] = temp_par_container[ii].size();
						if (!found_connection) {
							parent_node_list.clear();
							break;
						}
					}
					if (found_connection){
						parent_node_list.resize(parent_node_list_size);
						for (int ii=0; ii<parent_node_list_size; ++ii) {
							std::vector<int> node(num_dfa_sps + 1); // Temp unique node
							std::vector<int> ret_list_inds;
							Graph<float>::augmentedStatePreImage(node_list_sizes, ii, ret_list_inds);
							node[0] = TS_sps->getCurrNode();
							for (int iii=0; iii<num_dfa_sps; ++iii) {
								node[iii+1] = temp_par_container[iii][ret_list_inds[iii]];
							}
							parent_node_list[ii] = node;
						}
					}
				}
				bool unique = true;
				int connected_node_ind = -1;
				std::vector<int> unique_dfa_parent_set;
				float weight = curr_leaf_weight + temp_weight;
				unsigned int depth = curr_leaf_depth + 1;
				bool all_included = true;
				if (round == 0) {
					std::vector<int> node_inds(num_dfa_sps + 1);
					node_inds[0] = TS_sps->getCurrNode();
					for (int i=0; i<num_dfa_sps; ++i) {
						node_inds[i+1] = dfa_list_sps->operator[](i)->getCurrNode();
					}
					connected_node_ind = Graph<float>::augmentedStateImage(node_inds, graph_sizes);
					if (visited[connected_node_ind]) {
						continue;
					} 
					if (!min_w.is_inf[connected_node_ind]) { // Candidate node has been found
						if (min_w.min_weight[connected_node_ind] > weight) { // Candidate node is more optimal than previous found node
							min_w.min_weight[connected_node_ind] = weight;
						} 
						continue;
					} else {
						min_w.is_inf[connected_node_ind] = false;
						min_w.min_weight[connected_node_ind] = weight;
					}
					std::pair<int, float> new_leaf = {connected_node_ind, weight};
					pq.push(new_leaf); // add to prio queue
					//tree.connect(curr_leaf_ind, {connected_node_ind, &min_w.min_weight[connected_node_ind]});
				} else {
					int temp_connected_node_ind = -1;
					for (auto par_node : parent_node_list) {
						temp_connected_node_ind = Graph<float>::augmentedStateImage(par_node, graph_sizes);
						if (visited[temp_connected_node_ind]) {
							continue;
						} else {
							all_included = false;
						}
						if (!min_w.is_inf[temp_connected_node_ind]) {
							if (min_w.min_weight[temp_connected_node_ind] > weight) {
								min_w.min_weight[temp_connected_node_ind] = weight;
							} 
							continue;
						} else {
							min_w.is_inf[temp_connected_node_ind] = false;
							min_w.min_weight[temp_connected_node_ind] = weight;
							spw.reachability[temp_connected_node_ind] = true; // mark the new new as reachable
						}
						if (depth_limiting) {
							depth_map[temp_connected_node_ind] = depth;
							cout<<"state_weights set: "<<spwFunc(weight, depth)<<endl;
                            spw.state_weights[temp_connected_node_ind] = spwFunc(weight, depth);
						} else {
                            spw.state_weights[temp_connected_node_ind] = spwFunc(weight, 0); 
						}
						std::pair<int, float> new_leaf = {temp_connected_node_ind, weight};
						pq.push(new_leaf); // add to prio queue
						//tree.connect(curr_leaf_ind, {temp_connected_node_ind, &min_w.min_weight[temp_connected_node_ind]});
					}
				}

				if (round == 0) {
					bool all_accepting = true;
					for (int i=0; i<num_dfa_sps; ++i) {
						if (!dfa_list_sps->operator[](i)->isCurrAccepting()) {
							all_accepting = false;
							break;
						}
					}
					if (all_accepting) {
						accepting.first = true;
						accepting.second.push_back(connected_node_ind);
					}
				}
			}
			prev_leaf_ind = curr_leaf_ind;
		}
	}
	if (!exit_failure) {
		return true;
	} else {
		cout<<"Failed space search."<<endl;
		return false;
	}
}


typename SymbSearch::StrategyResult SymbSearch::synthesizeRiskStrategy(TS_EVAL<State>* TS_sps, DFA_EVAL* cosafe_dfa, DFA_EVAL* live_dfa) {

	/* Arguments:

	- TS_sps: Transition system
	- cosafe_dfa: DFA for negation of safety formula
	- live_dfa: DFA for liveness formula

	*/

	const int num_dfa_sps = 2;
	const int num_objectives = 2;

	std::vector<const std::vector<std::string>*> total_alphabet(num_dfa_sps);
	std::vector<int> graph_sizes;
	graph_sizes.push_back(TS_sps->size());
	int p_space_size = TS_sps->size();
	total_alphabet[0] = cosafe_dfa->getAlphabetEVAL();
	graph_sizes.push_back(cosafe_dfa->getDFA()->size());
	total_alphabet[1] = live_dfa->getAlphabetEVAL();
	graph_sizes.push_back(live_dfa->getDFA()->size());
	p_space_size *= graph_sizes[1] * graph_sizes[2];

	StrategyResult ret_result(p_space_size);

	if (!TS_sps->isReversible()) {
		cout<<"Error: Cannot perform space search on irreversible graphs\n";
		return ret_result;
	}
	if (!cosafe_dfa->getDFA()->isReversible()) {
		cout<<"Error: Cannot perform space search on irreversible graph cosafe_dfa\n";
		return ret_result;
	}

	TS_sps->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet

	spaceWeight risk_spw;
	bool success = generateRisk(TS_sps, cosafe_dfa, risk_spw);

	minWeightLS min_w(2, p_space_size);

	auto compare = [](const std::pair<int, std::weak_ptr<LexSet>>& pair1, const std::pair<int, std::weak_ptr<LexSet>>& pair2) {
		return *(pair1.second.lock()) > *(pair2.second.lock());
	};

	bool exit_failure = false;
	std::pair<bool, std::vector<int>> accepting;
	for (int round=0; round<2; ++round) {
		std::string search_type = (round == 0) ? "forward" : "reverse";
		std::priority_queue<std::pair<int, std::weak_ptr<LexSet>>, std::vector<std::pair<int, std::weak_ptr<LexSet>>>, decltype(compare)> pq(compare);

		bool found_target_state = false;
		std::vector<bool> visited(p_space_size, false);

		//clearSetsLS();
		setContainer_t<LexSet> set_container;
		
		std::pair<int, std::weak_ptr<LexSet>> curr_leaf;
		if (round == 0) {
			TS_sps->reset();
			cosafe_dfa->reset();
			live_dfa->reset();
			std::vector<int> init_node_inds(num_dfa_sps + 1);
			init_node_inds[0] = TS_sps->getCurrNode();
			init_node_inds[1] = cosafe_dfa->getCurrNode();
			init_node_inds[2] = live_dfa->getCurrNode();
			int init_node_prod_ind = Graph<float>::augmentedStateImage(init_node_inds, graph_sizes);
			min_w.is_inf[init_node_prod_ind] = false;
			min_w.min_weight[init_node_prod_ind].fill(0.0f);
			visited[init_node_prod_ind] = true;
			curr_leaf.first = init_node_prod_ind;
			auto new_set_ptr = newSet<LexSet>(num_objectives, set_container);
			curr_leaf.second = new_set_ptr;
			pq.push(curr_leaf);
		} else {
			int ROOT_STATE = p_space_size; // this is the root state ind, guaranteed to be larger than any prod state ind
			min_w.reset();
			for (auto acc_prod_state : accepting.second) {
				min_w.is_inf[acc_prod_state] = false;
				min_w.min_weight[acc_prod_state].fill(0.0f);
				visited[acc_prod_state] = true;
				std::vector<int> sol_inds;
				Graph<float>::augmentedStatePreImage(graph_sizes, acc_prod_state, sol_inds);
				curr_leaf.first = acc_prod_state;
				auto new_set_ptr = newSet<LexSet>(num_objectives, set_container);
				curr_leaf.second = new_set_ptr;
				pq.push(curr_leaf);
			}
		}

		float min_accepting_cost = -1;
		int prev_leaf_ind = -1;
		int prod_solution_ind;
		int iterations = 0;
		while (pq.size() > 0) {
			iterations++;
			curr_leaf = pq.top();
			pq.pop();
			int curr_leaf_ind = curr_leaf.first;
			std::weak_ptr<LexSet> curr_leaf_weight = curr_leaf.second;
			if (!min_w.is_inf[curr_leaf_ind]) {
				if ((*curr_leaf_weight.lock()) > min_w.min_weight[curr_leaf_ind]) {
					continue;
				}
			}

			visited[curr_leaf_ind] = true;
			std::vector<int> ret_inds;
			Graph<float>::augmentedStatePreImage(graph_sizes, curr_leaf_ind, ret_inds);
			TS_sps->set(ret_inds[0]);
			cosafe_dfa->set(ret_inds[1]);
			live_dfa->set(ret_inds[2]);
			std::vector<int> con_nodes;
			std::vector<WL*> con_data;
			con_data.clear();
			if (round == 0) {
				TS_sps->getConnectedDataEVAL(con_data);
				TS_sps->getConnectedNodesEVAL(con_nodes);
			} else {
				TS_sps->getParentDataEVAL(con_data);
				TS_sps->getParentNodesEVAL(con_nodes);
			}
			for (int j=0; j<con_data.size(); ++j) {
				TS_sps->set(ret_inds[0]);
                cosafe_dfa->set(ret_inds[1]);
                live_dfa->set(ret_inds[2]);
				std::string temp_str = con_data[j]->label;
				float temp_weight = con_data[j]->weight;
				bool found_connection = true;
				int prev_ts_ind;
				if (round == 0) {
					found_connection = TS_sps->eval(temp_str, true); // second arg tells wether or not to evolve on graph
				} else {
					prev_ts_ind = TS_sps->getCurrNode();
					found_connection = TS_sps->evalReverse(temp_str, true); // second arg tells wether or not to evolve on graph
				}
				if (!found_connection) {
					cout<<"Error ("<<search_type<<"): Did not find connectivity in TS. Current node: "<<TS_sps->getCurrNode()<<", Action: "<<temp_str<<endl;
					return ret_result;
				}
				const std::vector<std::string>* lbls;
				if (round == 0){
					lbls = TS_sps->returnStateLabels(TS_sps->getCurrNode());
				} else {
					lbls = TS_sps->returnStateLabels(prev_ts_ind);
				}
				found_connection = false;
				bool found_true = true;
				std::vector<std::vector<int>> parent_node_list(num_dfa_sps); // Array of nodes arrays [(TS, DFA1, DFA2, ...), (), (), ...]
				if (round == 0) {
					found_connection = false;
					for (int ii=0; ii<lbls->size(); ++ii) {
						if (cosafe_dfa->eval(lbls->operator[](ii), true)) {
							found_connection = true;
							break;
						}
					}
					if (!found_connection) {
						cout<<"Error ("<<search_type<<"): Did not find connectivity in cosafe DFA. Current node: "<<TS_sps->getCurrNode()<<", Action: "<<temp_str<<endl;
						return ret_result;
					}
					found_connection = false;
					for (int ii=0; ii<lbls->size(); ++ii) {
						if (live_dfa->eval(lbls->operator[](ii), true)) {
							found_connection = true;
							break;
						}
					}
					if (!found_connection) {
						cout<<"Error ("<<search_type<<"): Did not find connectivity in liveness DFA. Current node: "<<TS_sps->getCurrNode()<<", Action: "<<temp_str<<endl;
						return ret_result;
					}
				} else {
					std::vector<std::vector<int>> temp_par_container(num_dfa_sps); // Array of lists of parent nodes for each DFA
					std::vector<int> node_list_sizes(num_dfa_sps);
					int parent_node_list_size = 1;
					for (int ii=0; ii<num_dfa_sps; ++ii) {
						if (ii == 0) {
							found_connection = cosafe_dfa->getParentNodesWithLabels(lbls, temp_par_container[ii]);
						} else if (ii == 1) {
							found_connection = live_dfa->getParentNodesWithLabels(lbls, temp_par_container[ii]);
						} else {
							cout<<"Error num_dfa_sps is greater than 2\n";
						}
						parent_node_list_size *= temp_par_container[ii].size();
						node_list_sizes[ii] = temp_par_container[ii].size();
						if (!found_connection) {
							parent_node_list.clear();
							break;
						}
					}
					if (found_connection){
						parent_node_list.resize(parent_node_list_size);
						for (int ii=0; ii<parent_node_list_size; ++ii) {
							std::vector<int> node(num_dfa_sps + 1); // Temp unique node
							std::vector<int> ret_list_inds;
							// Use aug state pre image in the space of node list sizes (not product graph)
							Graph<float>::augmentedStatePreImage(node_list_sizes, ii, ret_list_inds);
							node[0] = TS_sps->getCurrNode();
							for (int iii=0; iii<num_dfa_sps; ++iii) {
								node[iii+1] = temp_par_container[iii][ret_list_inds[iii]];
							}
							parent_node_list[ii] = node;
						}
					}
				}
				bool unique = true;
				int connected_node_ind = -1;
				std::vector<int> unique_dfa_parent_set;
				bool all_included = true;
				if (round == 0) {
					std::vector<int> node_inds(num_dfa_sps + 1);
					node_inds[0] = TS_sps->getCurrNode();
					node_inds[1] = cosafe_dfa->getCurrNode();
					node_inds[2] = live_dfa->getCurrNode();
					LexSet weight(2);
					weight = *curr_leaf_weight.lock();

					// Get risk values:
					std::vector<float> risk_vals(2);
					risk_vals[0] = 0.0;
					risk_vals[1] = temp_weight;
					weight += risk_vals;

					connected_node_ind = Graph<float>::augmentedStateImage(node_inds, graph_sizes);
					if (visited[connected_node_ind]) {
						continue;
					} 
					if (!min_w.is_inf[connected_node_ind]) { // Candidate node has been found
						if (min_w.min_weight[connected_node_ind] > weight) { // Candidate node is more optimal than previous found node
							min_w.min_weight[connected_node_ind] = weight;
						}
						continue;
					} else {
						min_w.is_inf[connected_node_ind] = false;
						min_w.min_weight[connected_node_ind] = weight;
					}
					auto tmp_new_set_ptr = newSet<LexSet>(2, set_container);
					*tmp_new_set_ptr.lock() = weight;
					std::pair<int, std::weak_ptr<LexSet>> new_leaf;
					new_leaf.first = connected_node_ind;
					new_leaf.second = tmp_new_set_ptr;
					pq.push(new_leaf); // add to prio queue
				} else {
					int temp_connected_node_ind = -1;
					for (auto par_node : parent_node_list) {
						temp_connected_node_ind = Graph<float>::augmentedStateImage(par_node, graph_sizes);
						int curr_risk_ind = Graph<float>::augmentedStateImage({par_node[0], par_node[1]}, {graph_sizes[0], graph_sizes[1]});
						float curr_risk;
						bool curr_risk_is_inf = false;
						if (risk_spw.reachability[curr_risk_ind]) {
							curr_risk = risk_spw.state_weights[curr_risk_ind];
							if (curr_risk == 2.0f) {
								continue;
							}
						} else {
							curr_risk = 0.0f; // Obtain no cost if there is no risk
						}
						// Assign the weights
						LexSet weight(2);
						weight = *curr_leaf_weight.lock();
						// Get risk values:
						std::vector<float> risk_vals(2);
						risk_vals[0] = curr_risk;
						risk_vals[1] = temp_weight;
						weight += risk_vals;
						if (visited[temp_connected_node_ind]) {
							continue;
						} else {
							all_included = false;
						}
						if (!min_w.is_inf[temp_connected_node_ind]) {
							if (min_w.min_weight[temp_connected_node_ind] > weight) {
								min_w.min_weight[temp_connected_node_ind] = weight;
								ret_result.action_map[temp_connected_node_ind] = temp_str; 
							}
							continue;
						} else {
							min_w.is_inf[temp_connected_node_ind] = false;
							min_w.min_weight[temp_connected_node_ind] = weight;
							ret_result.reachability[temp_connected_node_ind] = true; // mark the new new as reachable
							ret_result.action_map[temp_connected_node_ind] = temp_str; 
						}
						auto tmp_new_set_ptr = newSet<LexSet>(2, set_container);
						*tmp_new_set_ptr.lock() = weight;
						std::pair<int, std::weak_ptr<LexSet>> new_leaf;
						new_leaf.first = temp_connected_node_ind;
						new_leaf.second = tmp_new_set_ptr;
						pq.push(new_leaf); // add to prio queue
					}
				}

				if (round == 0) {
					if (live_dfa->isCurrAccepting()) {
						accepting.first = true;
						accepting.second.push_back(connected_node_ind);
					}
				}
			}
			prev_leaf_ind = curr_leaf_ind;
		}
	}
	if (!exit_failure) {
		for (int bb=0; bb<min_w.min_weight.size(); ++bb) {
			cout<<"prod_state: "<<bb<<" min cost to goal: "<<endl;
			min_w.min_weight[bb].print();
		}
		ret_result.success = true;
		return ret_result;
	} else {
		cout<<"Failed space search."<<endl;
		ret_result.success = false;
		return ret_result;
	}
}

bool SymbSearch::generateRisk(TS_EVAL<State>* TS_sps, DFA_EVAL* cosafe_dfa, spaceWeight& spw) {
	std::vector<DFA_EVAL*> co_safe_dfa_vec = {cosafe_dfa};
	auto spwFunc = [](float ts_cost_to_goal, unsigned depth_to_goal) {
		if (depth_to_goal == 0) {
			return 2.0f;
		} else {
			return 1.0f; // Return a value of 1.0 for any state with imminent risk
		}
	};
	bool success = spaceSearch(TS_sps, &co_safe_dfa_vec, spw, spwFunc, 1); // max_depth of 1 determines risk
	if (!success) {
		cout<<"Error: spaceSearch failed.\n";
		return false;
	}
	return true;
}

bool SymbSearch::generateHeuristic() {
	heuristic.resize(num_dfas);
	for (int i=0; i<num_dfas; ++i) {
		heuristic[i].dfa_ind = i;
		std::vector<DFA_EVAL*> dfa_list_single = {dfa_list_ordered->operator[](i)};
		auto spwFunc = [](float ts_cost_to_goal, unsigned depth_to_goal) {
			return ts_cost_to_goal;
		};
		bool success = spaceSearch(TS, &dfa_list_single, heuristic[i], spwFunc);
		if (!success) {
			cout<<"Error: spaceSearch failed on dfa: "<<i<<"\n";
			return false;
		}
	}
	return true;
}

float SymbSearch::pullStateWeight(unsigned ts_ind, unsigned dfa_ind, unsigned dfa_list_ind, bool& reachable) const {	
	int n = TS->size();
	int m = dfa_list_ordered->operator[](dfa_list_ind)->getDFA()->size();
	std::vector<int> inds(2);
	inds[0] = ts_ind;
	inds[1] = dfa_ind;
	int p_node_ind = Graph<float>::augmentedStateImage(inds, {n, m});
	if (heuristic[dfa_list_ind].reachability[p_node_ind]) {
		reachable = true;
		return heuristic[dfa_list_ind].state_weights[p_node_ind];
	} else {
		reachable = false;
		return -1;
	}
}

void SymbSearch::resetSearchParameters() {
	pathlength = 0.0;
	TS_action_sequence.clear();
	TS_state_sequence.clear();
}

std::pair<bool, float> SymbSearch::search(bool use_heuristic) {
	resetSearchParameters();
	std::pair<bool, float> ret_vals;
	if (!TS_set || !dfas_set || !mu_set) {
		cout<<"Error: Forgot to set TS, dfas, or mu.\n";
		ret_vals.second = false;
		return ret_vals;
	}
	auto compareLEX = [](const std::pair<int, DetourLex*>& pair1, const std::pair<int, DetourLex*>& pair2) {
		return *(pair1.second) > *(pair2.second);
	};
	auto accCompareLEX = [](const DetourLex& curr_sol, const DetourLex& min_acc_sol){
		return curr_sol < min_acc_sol;
	};
	auto ___ = [](const DetourLex&) {return true;};

	PlanResult result_LEX(num_dfas, mu);
	if (use_benchmark) {benchmark.pushStartPoint("first_search");}
	result_LEX = BFS(compareLEX, accCompareLEX, ___, false, false, true); // No pruning criteria, no need for path

	if (use_benchmark) {benchmark.measureMilli("first_search");}
	if (!result_LEX.success) {
		ret_vals.second = false;
		cout<<"Info: First search failed."<<endl; //TODO move this to verbose
		return ret_vals;
	}
	if (verbose) {
		cout<<"Printing prune bound: "<<endl;
		result_LEX.pathcost.print();
	}
	auto pruneCriterionMAX = [&prune_bound=result_LEX.pathcost](const DetourLex& arg_set) {
		return (!prune_bound.withinBounds(arg_set));
	};
	auto compareMAX = [](const std::pair<int, DetourLex*>& pair1, const std::pair<int, DetourLex*>& pair2) {
		return pair1.second->getMaxVal() > pair2.second->getMaxVal();
	};
	auto accCompareMAX = [](const DetourLex& curr_sol, const DetourLex& min_acc_sol){
		if (min_acc_sol.isInf()) {
			return true;
		}
		return curr_sol.getMaxVal() < min_acc_sol.getMaxVal();
	};

	PlanResult result_solution(num_dfas, mu);
	if (use_benchmark) {benchmark.pushStartPoint("second_search");}
	result_solution = BFS(compareMAX, accCompareMAX, pruneCriterionMAX, true, true, use_heuristic); 
	if (!result_solution.success) {
		ret_vals.second = false;
		cout<<"Info: Second search failed."<<endl; //TODO move this to verbose
		return ret_vals;
	}
	if (use_benchmark) {
		benchmark.measureMilli("second_search");
		benchmark.pushAttributesToFile();
	}
	
	if (verbose) {
		cout<<"Pathlength: "<<pathlength<<", Max val in solution cost: "<<result_solution.pathcost.getMaxVal()<<"\n";
	}
	ret_vals.first = pathlength;
	ret_vals.second = true;
	return ret_vals;
}

typename SymbSearch::PlanResult SymbSearch::BFS(std::function<bool(const std::pair<int, DetourLex*>&, const std::pair<int, DetourLex*>&)> compare, std::function<bool(const DetourLex&, const DetourLex&)> acceptanceCompare, std::function<bool(const DetourLex&)> pruneCriterion, bool prune, bool extract_path, bool use_heuristic) {	
	PlanResult ret_result(num_dfas, mu);
	ret_result.success = false;
	std::vector<const std::vector<std::string>*> total_alphabet(num_dfas);
	std::vector<int> graph_sizes = {static_cast<int>(TS->size())};
	int p_space_size = TS->size();
	for (int i=0; i<num_dfas; ++i) {
		total_alphabet[i] = dfa_list_ordered->operator[](i)->getAlphabetEVAL();
		graph_sizes.push_back(dfa_list_ordered->operator[](i)->getDFA()->size());
		p_space_size *= dfa_list_ordered->operator[](i)->getDFA()->size();
	}
	// ATTENTION: "pq" is indicies are in the product space
	// ATTENTION: Tree is indicies are indexed by tree size
	// ATTENTION: "visited" is indexed in the product space
	std::vector<bool> visited(p_space_size, false);
	// ATTENTION: "parents" is indexed by the tree size for memory space efficiency
	std::vector<int> parents(p_space_size, -1);
	// ATTENTION: Members of min_w are indexed in the product space
	minLS min_w(p_space_size);

	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet

	DetourLex min_accepting_cost(num_dfas, mu); // Declare here so we can have a return value
	float min_accepting_h_cost;

	if (use_heuristic) {
		bool h_success = generateHeuristic();
		if (!h_success) {
			return ret_result;
		}
	}
	int solution_ind;
	min_w.reset();
	std::priority_queue<std::pair<int, DetourLex*>, std::vector<std::pair<int, DetourLex*>>, decltype(compare)> pq(compare);
	//Graph<IVFlexLex<DetourLex>> tree;

	//clearNodes();
	nodeContainer_t<DetourLex> node_container;
	setContainer_t<DetourLex> set_container;

	TS->reset();
	bool init = true;
	std::weak_ptr<IVFlexLex<DetourLex>> temp_nodeptr = newNode<DetourLex>(num_dfas, mu, node_container);
	//cout<<"use count: "<<temp_nodeptr.lock().use_count()<<endl;
	std::vector<float> temp_lex_set_fill(num_dfas);
	temp_nodeptr.lock()->i = TS->getCurrNode();
	std::vector<int> init_node_inds(num_dfas + 1);
	init_node_inds[0] = TS->getCurrNode();
	for (int i=0; i<num_dfas; ++i) {
		dfa_list_ordered->operator[](i)->reset();
		temp_nodeptr.lock()->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();
		init_node_inds[i+1] = dfa_list_ordered->operator[](i)->getCurrNode();
		// Weights are zero for the root node:
		temp_lex_set_fill[i] = 0;
	}
	int init_node_prod_ind = Graph<float>::augmentedStateImage(init_node_inds, graph_sizes);
	min_w.prod2node_list[init_node_prod_ind] = node_container.size() - 1; //map the prod node ind to the node in node list
	visited[init_node_prod_ind] = true;
	min_w.is_inf[init_node_prod_ind] = false;
	temp_nodeptr.lock()->lex_set = temp_lex_set_fill;
	std::pair<int, DetourLex*> curr_leaf;
	//std::weak_ptr<DetourLex> temp_setptr = newSet<DetourLex>(num_dfas, set_container);
	//*temp_setptr.lock() = (temp_nodeptr.lock()->lex_set);
	curr_leaf.first = init_node_prod_ind;
	curr_leaf.second = &(temp_nodeptr.lock()->lex_set);
	pq.push(curr_leaf);
	std::vector<WL*> con_data;
	std::pair<bool, std::vector<int>> accepting;
	min_accepting_cost.setInf();
	int iterations = 0;
	int prev_leaf_ind = -1;
	bool finished = false;
	bool sol_found = false;

	while (!finished) {
		if (pq.empty() || sol_found) {
			finished = true;
			continue;
		}
		iterations++;
		curr_leaf = pq.top();
		pq.pop();
		int curr_leaf_prod_ind = curr_leaf.first;
		int curr_leaf_ind = min_w.prod2node_list.at(curr_leaf_prod_ind);
		DetourLex* curr_path_weight = &node_container[curr_leaf_ind]->lex_set;
		visited[curr_leaf_prod_ind] = true;
		TS->set(node_container[curr_leaf_ind]->i);
		for (int i=0; i<num_dfas; ++i) {
			dfa_list_ordered->operator[](i)->set(node_container[curr_leaf_ind]->v[i]);
		}
		con_data.clear();
		TS->getConnectedDataEVAL(con_data);
		std::vector<int> con_nodes;
		con_nodes.clear();
		TS->getConnectedNodesEVAL(con_nodes);
		for (int j=0; j<con_data.size(); ++j) {
			TS->set(node_container[curr_leaf_ind]->i);
			for (int i=0; i<num_dfas; ++i) {
				dfa_list_ordered->operator[](i)->set(node_container[curr_leaf_ind]->v[i]);
			}
			std::string temp_str = con_data[j]->label;
			float temp_weight = con_data[j]->weight;
			bool found_connection = true;

			// Evaluate each graph under the current action
			found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
			if (!found_connection) {
				cout<<"Error: Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<endl;
				return ret_result;
			}
			for (int i=0; i<num_dfas; ++i) {
				// There could be multiple logical statements the correspond to the same
				// underlying label, therefore we must test all of the statements until
				// one is found. Only one is needed due to determinism.
				const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
				found_connection = false;
				for (int ii=0; ii<lbls->size(); ++ii) {
					if (dfa_list_ordered->operator[](i)->eval(lbls->operator[](ii), true)) {
						found_connection = true;
						break;
					}
				}
				if (!found_connection) {
					cout<<"Error: Connectivity was not found for either the TS or a DFA. \n";
					return ret_result;
				}
			}
			// Collect the nodes that each graph has transitioned to to get the product state
			std::vector<int> node_inds(num_dfas + 1);
			node_inds[0] = TS->getCurrNode();
			for (int i=0; i<num_dfas; ++i) {
				node_inds[i+1] = dfa_list_ordered->operator[](i)->getCurrNode();
			}
			int con_node_prod_ind = Graph<float>::augmentedStateImage(node_inds, graph_sizes);
			if (!use_heuristic) {
				if (visited[con_node_prod_ind]) { // Node was visited
					continue;
				}
			}
			// Now check if the node is accepting, if not, append the action weight for that formula
			bool all_accepting = true;
			for (int i=0; i<num_dfas; ++i) {
				if (dfa_list_ordered->operator[](i)->isCurrAccepting()) {
					temp_lex_set_fill[i] = 0;
				} else {
					all_accepting = false;
					temp_lex_set_fill[i] = temp_weight;
				}
			}
			// Only consider non pruned nodes
			if (prune) {
				DetourLex temp_prune_check(num_dfas, mu); 
				temp_prune_check = *(curr_path_weight);
				temp_prune_check += temp_lex_set_fill;
				if (pruneCriterion(temp_prune_check)) {
					continue;
				}
			}
			// Check if node was seen before to see if a shorter path has been found
			if (!min_w.is_inf[con_node_prod_ind]) { // Node was seen before, non inf weight, but not visited
				int seen_node_ind = min_w.prod2node_list.at(con_node_prod_ind); // This value will be mapped if weve seen the node before
				std::weak_ptr<IVFlexLex<DetourLex>> seen_node = node_container[seen_node_ind];
				DetourLex temp_lex_set(num_dfas, mu, &temp_lex_set_fill);
				temp_lex_set += *curr_path_weight;
				bool updated = false;
				if (prune) {
					if (seen_node.lock()->lex_set.getMaxVal() > temp_lex_set.getMaxVal()) {
						seen_node.lock()->lex_set = temp_lex_set;
						// UPDATE PARENT HERE:
						parents[seen_node_ind] = curr_leaf_ind;
						updated = true;
					}
				} else {
					if (seen_node.lock()->lex_set > temp_lex_set) {
						seen_node.lock()->lex_set = temp_lex_set;
						// UPDATE PARENT HERE:
						parents[seen_node_ind] = curr_leaf_ind;
						updated = true;
					}
				}
				if (!updated || !use_heuristic) {
					continue;
				}
			} 
			std::weak_ptr<IVFlexLex<DetourLex>> new_temp_nodeptr = newNode<DetourLex>(num_dfas, mu, node_container);
			min_w.prod2node_list[con_node_prod_ind] = node_container.size() - 1; // Make new node, must map the tree and prod indices
			int con_node_ind = node_container.size() - 1;
			new_temp_nodeptr.lock()->i = TS->getCurrNode();
			for (int i=0; i<num_dfas; ++i) {
				new_temp_nodeptr.lock()->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();
			}
			new_temp_nodeptr.lock()->lex_set = *(curr_path_weight);
			new_temp_nodeptr.lock()->lex_set += temp_lex_set_fill;
			// The priority queue includes the heuristic cost, however the tree does not:
			std::pair<int, DetourLex*> new_leaf;
			if (use_heuristic) {
				float max_h_val = 0.0;
				std::vector<float> lex_h_vals(num_dfas, 0.0);
				for (int i=0; i<num_dfas; ++i) {
					bool reachable;
					if (!dfa_list_ordered->operator[](i)->isCurrAccepting()) {
						float h_val = pullStateWeight(TS->getCurrNode(), dfa_list_ordered->operator[](i)->getCurrNode(), i, reachable);
						if (!reachable) {
							cout<<"Error: Attempted to find heuristic distance for unreachable product state (ts: "<<TS->getCurrNode()<<" dfa "<<i<<": "<<dfa_list_ordered->operator[](i)->getCurrNode()<<") \n";
							return ret_result;
						}
						if (prune) {
							if (i == 0 || h_val > max_h_val) {
								max_h_val = h_val;
							}
						} else {
							if (h_val > 0.0) {
								lex_h_vals[i] = h_val;
								break;
							}
						}
					}
				}
				std::weak_ptr<DetourLex> new_temp_setptr = newSet(num_dfas, set_container);
				new_temp_setptr.lock()->operator=(new_temp_nodeptr.lock()->lex_set);
				if (prune) {
					new_temp_setptr.lock()->addToMax(max_h_val);
				} else {
					*new_temp_setptr.lock() += lex_h_vals;
				}
				new_leaf = {con_node_prod_ind, new_temp_setptr.lock().get()};
			} else {
				new_leaf = {con_node_prod_ind, &(new_temp_nodeptr.lock()->lex_set)};
			}
			pq.push(new_leaf); // add to prio queue
			//tree.connect(curr_leaf_ind, {con_node_ind, new_temp_nodeptr});
			min_w.is_inf[con_node_prod_ind] = false; // mark node as seen
			parents[con_node_ind] = curr_leaf_ind;
			int solution_ind_prod;

			if (all_accepting) {
				accepting.first = true;
				accepting.second.push_back(con_node_prod_ind);
				solution_ind_prod = con_node_prod_ind;
				solution_ind = con_node_ind;
				if (acceptanceCompare(*(new_leaf.second), min_accepting_cost)) { // new_leaf.second < min_accepting_cost
					min_accepting_cost = *(new_leaf.second);
				}
			}
			if (accepting.first) {
				if (!acceptanceCompare(*(pq.top().second), min_accepting_cost)) {
					finished = true;
					sol_found = true;
					break;
				}
			}
		}
		prev_leaf_ind = curr_leaf_ind;
	}
	if (finished && sol_found) {
		if (extract_path) {
			extractPath(parents, solution_ind,graph_sizes, node_container);
			int p_space_size = 1;
		}
		if (use_benchmark && prune) {
			benchmark.addCustomTimeAttr("iterations", static_cast<double>(iterations), ""); // no units
		}
		ret_result.pathcost = min_accepting_cost;
		ret_result.success = true;
		return ret_result;
	} else {
		cout<<"Failed (no plan)."<<endl;
		return ret_result;
	}
}

void SymbSearch::extractPath(const std::vector<int>& parents, int accepting_state, const std::vector<int>& graph_sizes, const nodeContainer_t<DetourLex>& node_container) {
	int curr_node = 0;
	curr_node = accepting_state;
	std::vector<int> reverse_TS_state_sequence;
	std::vector<int> reverse_prod_state_sequence;
	reverse_TS_state_sequence.clear();
	reverse_prod_state_sequence.clear();
	while (curr_node != 0) {
		reverse_TS_state_sequence.push_back(node_container[curr_node]->i);
		std::vector<int> prod_inds(num_dfas + 1);
		prod_inds[0] = node_container[curr_node]->i;
		for (int i=0; i<num_dfas; ++i) {
			prod_inds[i+1] = node_container[curr_node]->v[i];
		}
		int prod_ind_out = Graph<float>::augmentedStateImage(prod_inds, graph_sizes);
		reverse_prod_state_sequence.push_back(prod_ind_out);
		curr_node = parents[curr_node];
	}
	reverse_TS_state_sequence.push_back(node_container[0]->i); // finally add the init state
	TS_state_sequence.resize(reverse_TS_state_sequence.size());
	TS_action_sequence.resize(reverse_TS_state_sequence.size()-1);
	std::vector<int> prod_state_sequence(reverse_prod_state_sequence.size());
	if (verbose) {
		cout<<"Info: Successfully extracted plan!\n";
		cout<<"State sequence: ";
	}
	for (int i=0; i<reverse_TS_state_sequence.size(); ++i) {
		TS_state_sequence[i] = reverse_TS_state_sequence[reverse_TS_state_sequence.size()-1-i];
		if (verbose) {
			cout<<" -> "<<TS_state_sequence[i];
		}
	}
	if (verbose) {
		cout<<"\n";
	}

	if (verbose) {
		cout<<"\nProduct State sequence: ";
	}
	for (int i=0; i<reverse_prod_state_sequence.size(); ++i) {
		prod_state_sequence[i] = reverse_prod_state_sequence[reverse_prod_state_sequence.size()-1-i];
		if (verbose) {
			cout<<" -> "<<prod_state_sequence[i];
		}
	}
	if (verbose) {
		cout<<"\n";
	}
	std::vector<WL*> con_data;
	std::vector<int> con_nodes;
	pathlength = 0.0;
	for (int i=0; i<TS_action_sequence.size(); ++i) {
		TS->set(TS_state_sequence[i]);
		TS->getConnectedDataEVAL(con_data);
		TS->getConnectedNodesEVAL(con_nodes);
		for (int ii=0; ii<con_nodes.size(); ++ii){
			if (con_nodes[ii] == TS_state_sequence[i+1]) {
				TS_action_sequence[i] = con_data[ii]->label;
				pathlength += con_data[ii]->weight;
			}
		}
	}
	if (verbose) {
		cout<<"Info: Number of actions: "<<TS_action_sequence.size()<<", pathlength: "<<pathlength<<"\n";
	}
}

void SymbSearch::writePlanToFile(std::string filename, const std::vector<std::string>& xtra_info) {
	std::string line;
	std::ofstream plan_file;
	plan_file.open(filename);
	for (int i=0; i<TS_action_sequence.size(); ++i) {
			plan_file <<TS_action_sequence[i]<<"\n";
	}
	for (int i=0; i<xtra_info.size(); ++i) {
			plan_file <<xtra_info[i]<<"\n";
	}
	plan_file.close();
}

const std::vector<const State*> SymbSearch::getStateSequence() {
	std::vector<const State*> ret_state_seq(TS_state_sequence.size());
	for (int i=0; i<TS_state_sequence.size(); ++i) {
		ret_state_seq[i] = TS->getState(TS_state_sequence[i]);
	}
	return ret_state_seq;
}


const std::vector<std::string>& SymbSearch::getActionSequence() {
	return TS_action_sequence;
}

