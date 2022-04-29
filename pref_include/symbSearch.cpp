#include<fstream>
#include<chrono>
#include<ctime>
#include<unordered_map>
#include "symbSearch.h"
#include "benchmark.h"

template<class T>
SymbSearch<T>::PlanResult::PlanResult(float mu, int num_dfas) : pathcost(mu, num_dfas) {}

template<class T>
SymbSearch<T>::StrategyResult::StrategyResult(int graph_size) : reachability(graph_size, false), action_map(graph_size), success(false) {}

template<class T>
SymbSearch<T>::SymbSearch() : 
	benchmark("none"), 
	use_benchmark(false),
	verbose(false), 
	dfas_set(false), 
	TS_set(false), 
	mu_set(false) {}

template<class T>
SymbSearch<T>::SymbSearch(const std::string& bench_mark_session_, bool verbose_) : 
	benchmark(bench_mark_session_), 
	verbose(verbose_), 
	dfas_set(false), 
	TS_set(false), 
	mu_set(false) {

	if (bench_mark_session_.empty() || bench_mark_session_ == "none") {
		use_benchmark = false;
	} else {
		use_benchmark = true;
	}
}

template<class T>
void SymbSearch<T>::setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_) {
	node_list.clear();
	set_list.clear();
	dfa_list_ordered = dfa_list_ordered_;
	num_dfas = dfa_list_ordered->size();
	dfas_set = true;
}

template<class T>
void SymbSearch<T>::setTransitionSystem(TS_EVAL<State>* TS_) {
	TS = TS_;	
	TS_set = true;
}

template<class T>
void SymbSearch<T>::setFlexibilityParam(float mu_) {
	mu = mu_;
	mu_set = true;
}

template<class T>
IVFlexLex<T>* SymbSearch<T>::newNode() {
	IVFlexLex<T>* node_i = new IVFlexLex<T>(mu, num_dfas);
	//std::cout<<"new node: "<<node_i<<std::endl;
	//node_i->v.resize(num_dfas);
	node_list.push_back(node_i);
	return node_i;
}

template<class T>
IVLex* SymbSearch<T>::newNodeLS(unsigned node_size, unsigned set_size) {
	IVLex* node_i = new IVLex(node_size, set_size);
	//std::cout<<"new node: "<<node_i<<std::endl;
	//node_i->v.resize(num_dfas);
	node_list_ls.push_back(node_i);
	return node_i;
}

template<class T>
T* SymbSearch<T>::newSet() {
	T* set_i = new T(mu, num_dfas);
	//node_i->v.resize(num_dfas);
	set_list.push_back(set_i);
	return set_i;
}

template<class T>
LexSet* SymbSearch<T>::newSetLS(unsigned set_size) {
	LexSet* set_i = new LexSet(set_size);
	//node_i->v.resize(num_dfas);
	set_list_ls.push_back(set_i);
	return set_i;
}

//template<class T>
//IVFlexLex<T>* SymbSearch<T>::newNode_ss() {
//	IVFlexLex<T>* node_i = new IVFlexLex<T>(mu, num_dfas);
//	//node_i->v.resize(num_dfas);
//	node_list.push_back(node_i);
//	return node_i;
//}

template<class T>
template<typename Q>
void SymbSearch<T>::printQueue(Q queue) {
	int toomuch = 0;
	while (!queue.empty()) {
		std::cout<<"Ind: "<<queue.top().first<<std::endl;
		std::cout<<"LexSet: ";
		//std::cout<<" CHANGE weight: "<< queue.top().second->getMaxVal()<<std::endl;
		queue.top().second->print();
		queue.pop();
		toomuch++;
		if (toomuch > 50) {
			break;
		}
	}
}

template<class T>
template<typename Q_f>
void SymbSearch<T>::printQueueFloat(Q_f queue) {
	while (!queue.empty()) {
		std::cout<<"Ind: "<<queue.top().first<<", Weight: "<<queue.top().second<<std::endl;
		queue.pop();
	}
}

template<class T>
bool SymbSearch<T>::spaceSearch(TS_EVAL<State>* TS_sps, std::vector<DFA_EVAL*>* dfa_list_sps, spaceWeight& spw, std::function<float(float, unsigned int)> spwFunc, int max_depth) {

	/* Arguments:

	- TS_sps: Transition system
	- dfa_list_sps: List of dfas in product
	- spw: Space weight (product state weights)
	- spwFunc: Space weight function: spwFunc(float ts_cost_to_goal, unsigned depth_to_goal)

	*/

	bool depth_limiting = (max_depth > 0) ? true : false;
	const int num_dfa_sps = dfa_list_sps->size();
	if (!TS_sps->isReversible()) {
		std::cout<<"Error: Cannot perform space search on irreversible graphs\n";
		return false;
	}
	for (int i=0; i<num_dfa_sps; ++i) {
		if (!dfa_list_sps->operator[](i)->getDFA()->isReversible()) {
			std::cout<<"Error: Cannot perform space search on irreversible graphs\n";
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

	//int n = TS_sps->size();
	//int m = dfa_sps->getDFA()->size();
	//int p_space_size = n * m;
	//std::vector<int> graph_sizes = {n, m};

	// These indices are determined after the first search round:
	//int TS_accepting_state = -1;
	//std::vector<int> dfa_accepting_states;

	//std::vector<float> first_search_weights(p_space_size, -1.0);
	//std::vector<float> second_search_weights(p_space_size, -1.0);
	spw.state_weights.resize(p_space_size, 0.0);
	spw.reachability.resize(p_space_size, false);
	//spw.is_inf.resize(p_space_size, false);
	
	minWeight min_w(p_space_size);

	// Search: 

	//std::chrono::time_point<std::chrono::system_clock> end_time;
	//std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
	//for (int i=0; i<num_dfas; ++i) {
	//}
	//TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet
	auto compare = [](std::pair<int, float> pair1, std::pair<int, float> pair2) {
		return pair1.second > pair2.second;
	};

	bool exit_failure = false;
	std::pair<bool, std::vector<int>> accepting;
	for (int round=0; round<2; ++round) {
		//std::cout<<"\n -- spaceSearch: STARTING ROUND: "<<round<<std::endl;
		std::string search_type = (round == 0) ? "forward" : "reverse";
		std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float>>, decltype(compare)> pq(compare);
		std::unordered_map<int, int> depth_map;
		//pq.clear();

		// Tree to keep history as well as parent node list:
		bool found_target_state = false;
		std::vector<bool> visited(p_space_size, false);
		Graph<float> tree;
		//std::vector<int> parents;

		
		// Root tree node has index zero with weight zero
		std::pair<int, float> curr_leaf;
		if (round == 0) {
			TS_sps->reset();
			for (auto dfa_ptr : *(dfa_list_sps)) {
				dfa_ptr->reset();
			}
			std::vector<int> init_node_inds(num_dfa_sps + 1);
			for (int i=0; i<num_dfa_sps; ++i) {
				init_node_inds[i+1] = dfa_list_ordered->operator[](i)->getCurrNode();
			}
			int init_node_prod_ind = Graph<float>::augmentedStateImage(init_node_inds, graph_sizes);
			//std::cout<<"ROUND 0 INIT NODE IND: "<<init_node_prod_ind<<std::endl;
			min_w.is_inf[init_node_prod_ind] = false;
			min_w.min_weight[init_node_prod_ind] = 0;
			visited[init_node_prod_ind] = true;
			curr_leaf.first = init_node_prod_ind;
			curr_leaf.second = 0.0f;
			pq.push(curr_leaf);
		} else {
			// Add the accepting states to the prio queue with weight zero, and add them to the tree so
			int ROOT_STATE = p_space_size; // this is the root state ind, guaranteed to be larger than any prod state ind
			min_w.reset();
			for (auto acc_prod_state : accepting.second) {
				//std::cout<<"Found accepting prod state: "<<acc_prod_state<<std::endl;
				min_w.is_inf[acc_prod_state] = false;
				min_w.min_weight[acc_prod_state] = 0;
				visited[acc_prod_state] = true;
				spw.state_weights[acc_prod_state] = 0.0f;
				spw.reachability[acc_prod_state] = true;
				std::vector<int> sol_inds;
				Graph<float>::augmentedStatePreImage(graph_sizes, acc_prod_state, sol_inds);
				curr_leaf.first = acc_prod_state;
				curr_leaf.second = 0.0f;
				pq.push(curr_leaf);
				if (depth_limiting) {
					depth_map[acc_prod_state] = 0; // Depth of pin states is defined as zero
				}
				tree.connect(ROOT_STATE, {acc_prod_state, nullptr}); // the root state is the merged accepting state
			}
			//std::cout<<"round 1 set ts init: "<<TS_accepting_state<<std::endl;
			//std::cout<<"round 1 set dfa init: "<<best_dfa_accepting_state<<std::endl;
			//TS_sps->set(TS_accepting_state);
			//dfa_sps->set(best_dfa_accepting_state);
		}
		//std::cout<<"b4 asf"<<std::endl;
		//spw.state_weights[init_node_ind] = 0.0;

		float min_accepting_cost = -1;
		int prev_leaf_ind = -1;
		int prod_solution_ind;
		int iterations = 0;
		while (pq.size() > 0) {
			iterations++;

			//printQueueFloat(pq);

			curr_leaf = pq.top();
			pq.pop();
			int curr_leaf_ind = curr_leaf.first;
			int curr_leaf_depth = depth_map.at(curr_leaf_ind);
			if (depth_limiting && curr_leaf_depth >= max_depth) {
				continue;
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

			//if (curr_leaf_ind == prev_leaf_ind) {
			//	std::cout<<"no sol"<<std::endl;
			//	exit_failure = true;
			//	break;
			//}

			//std::cout<<" ------ CURRENT LEAF: "<<curr_leaf_ind<<std::endl;
			//std::cout<<" CURR LEAF WEIGHT: "<<curr_leaf_weight<<std::endl;
			//curr_leaf.second->setInf();
			//pq.push(curr_leaf);
			//printQueue(pq);

			//if (tree_end_node != 0) { // Use temp_nodeptr from outside the loop (init) when no nodes are in tree (=0)
			//	temp_nodeptr = tree.getNodeDataptr(curr_leaf_ind);
			//}

			// SET:
			TS_sps->set(ret_inds[0]);
			for (int i=0; i<num_dfa_sps; ++i) {
				dfa_list_sps->operator[](i)->set(ret_inds[i+1]);
			}

			//std::cout<<" SET NODE: "<<node_list[curr_leaf_ind]->i;
			//for (int i=0; i<num_dfas; ++i) {
			//	//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
			//	dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
			//}
			//std::cout<<" ---\n";
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
			//if (round == 1) {
			//std::cout<<"ROOT:  ts curr node: "<<TS_sps->getCurrNode()<<std::endl;
			//std::cout<<"    :  ts labels:";
			//const std::vector<std::string>* temp_lbls;
			//temp_lbls = TS->returnStateLabels(TS_sps->getCurrNode());
			//for (int i=0; i<temp_lbls->size(); ++i) {
			//	std::cout<<" "<<temp_lbls->operator[](i);
			////	std::cout<<"  label: "<<con_data[i]->label<<std::endl;
			////	std::cout<<"  weight: "<<con_data[i]->weight<<std::endl;
			//}
			//std::cout<<"\n";
			//std::cout<<"ROOT:  dfa curr node: "<<dfa_sps->getCurrNode()<<std::endl;
			////std::cout<<"printing con nodes and data for current node: "<<TS_sps->getCurrNode()<<std::endl;
			//////std::cout<<"con nodes size:"<<con_nodes.size()<<std::endl;
			//////std::cout<<"con data size:"<<con_data.size()<<std::endl;
			//std::cout<<"STAR: con ts nodes:";
			//for (int i=0; i<con_data.size(); ++i) {
			//	std::cout<<" "<<con_nodes[i];
			////	std::cout<<"  label: "<<con_data[i]->label<<std::endl;
			////	std::cout<<"  weight: "<<con_data[i]->weight<<std::endl;
			//}
			//std::cout<<"\n";
		
			//int pause;
			//std::cin >> pause;
			//}
			//std::pair<unsigned int, float*> src;
			//src.first = curr_leaf_ind;
			//src.second = &curr_leaf_weight;
			//for (auto con_data_ptr : con_data) {
			for (int j=0; j<con_data.size(); ++j) {
				// Reset after checking connected nodes
				TS->set(ret_inds[0]);
				//dfa_sps->set(ret_inds[1]);
				for (int i=0; i<num_dfa_sps; ++i) {
					dfa_list_sps->operator[](i)->set(ret_inds[i+1]);
				}

				//for (int i=0; i<num_dfas; ++i) {
				//	//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
				//	dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
				//}
				//printQueue(pq);
				//std::string temp_str = con_data_ptr->label;
				//float temp_weight = con_data_ptr->weight;
				std::string temp_str = con_data[j]->label;
				float temp_weight = con_data[j]->weight;
				//std::cout<<"temp label: "<<temp_str<<std::endl;
				//std::cout<<"temp weight: "<<temp_weight<<std::endl;
				bool found_connection = true;
				int prev_ts_ind;

				if (round == 0) {
					//std::cout<<"forward ts eval"<<std::endl;
					found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
				} else {
					//std::cout<<"reverse ts eval"<<std::endl;
					prev_ts_ind = TS->getCurrNode();
					found_connection = TS->evalReverse(temp_str, true); // second arg tells wether or not to evolve on graph
				
				}
				//std::cout<<"ts evolved state: "<<TS->getCurrNode()<<" under action: "<<temp_str<<std::endl;
				//

				if (!found_connection) {
					std::cout<<"Error ("<<search_type<<"): Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
					return false;
				}
				const std::vector<std::string>* lbls;
				if (round == 0){
					//std::cout<<"\nts node b4 return state labels: "<<TS->getCurrNode()<<std::endl;
					lbls = TS->returnStateLabels(TS->getCurrNode());
				} else {
					//std::cout<<"\nts node b4 return state labels: "<<prev_ts_ind<<std::endl;
					lbls = TS->returnStateLabels(prev_ts_ind);
				}
				found_connection = false;
				//std::cout<<"\n before eval"<<std::endl;
				bool found_true = true;
				std::vector<std::vector<int>> parent_node_list(num_dfa_sps); // Array of nodes arrays [(TS, DFA1, DFA2, ...), (), (), ...]
				if (round == 0) {
					for (int i=0; i<num_dfa_sps; ++i) {
						found_connection = false;
						for (int ii=0; ii<lbls->size(); ++ii) {
							//std::cout<<"       label: --- "<<lbls->operator[](ii)<<std::endl;
							//std::cout<<"DFA::: "<<(i-1)<<" curr node; "<<(dfa_list_ordered->operator[](i-1)->getCurrNode())<<std::endl;
							//if (round == 0) {
							if (dfa_list_sps->operator[](i)->eval(lbls->operator[](ii), true)) {
								found_connection = true;
								break;
							}
						}
						if (!found_connection) {
							std::cout<<"Error ("<<search_type<<"): Did not find connectivity in DFA. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
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


					//std::cout<<"found connection? : "<<found_connection<<" parent_node_list size: "<<parent_node_list.size()<<std::endl;

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
						} //else if (min_w.min_weight[connected_node_ind] < weight) { // Candidate node is less optimal than previous found node (don't care about it)
						//}
						continue;
					} else {
						min_w.is_inf[connected_node_ind] = false;
						min_w.min_weight[connected_node_ind] = weight;
					}

					//first_search_weights[connected_node_ind] = curr_leaf_weight + temp_weight;
					std::pair<int, float> new_leaf = {connected_node_ind, weight};
					pq.push(new_leaf); // add to prio queue
					tree.connect(curr_leaf_ind, {connected_node_ind, &min_w.min_weight[connected_node_ind]});
					//included[connected_node_ind] = true;
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
							} //else if (min_w.min_weight[temp_connected_node_ind] < weight) {
							//}
							continue;
						} else {
							min_w.is_inf[temp_connected_node_ind] = false;
							min_w.min_weight[temp_connected_node_ind] = weight;
							spw.reachability[temp_connected_node_ind] = true; // mark the new new as reachable
						}
						if (depth_limiting) {
							depth_map[temp_connected_node_ind] = depth;
                            spw.state_weights[temp_connected_node_ind] = spwFunc(weight, depth);
						} else {
                            spw.state_weights[temp_connected_node_ind] = spwFunc(weight, 0); 
						}
						std::pair<int, float> new_leaf = {temp_connected_node_ind, weight};
						pq.push(new_leaf); // add to prio queue
						tree.connect(curr_leaf_ind, {temp_connected_node_ind, &min_w.min_weight[temp_connected_node_ind]});
						//included[temp_connected_node_ind] = true;
					}
				}

				if (round == 0) {
					bool all_accepting = true;
					for (int i=0; i<num_dfa_sps; ++i) {
						if (!dfa_list_sps->operator[](i)->isCurrAccepting()) {
							//std::cout<<"found accepting node: "<<std::endl;
							//std::cout<<" ts node: "<<TS_sps->getCurrNode()<<std::endl;
							//std::cout<<" ts node: "<<dfa_sps->getCurrNode()<<std::endl;
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
		std::cout<<"Failed space search."<<std::endl;
		return false;
	}
}


template<class T>
SymbSearch<T>::StrategyResult SymbSearch<T>::synthesizeRiskStrategy(TS_EVAL<State>* TS_sps, DFA_EVAL* cosafe_dfa, DFA_EVAL* live_dfa) {

	/* Arguments:

	- TS_sps: Transition system
	- dfa_list_sps: List of dfas in product
	- spw: Space weight (product state weights)
	- spwFunc: Space weight function: spwFunc(float ts_cost_to_goal, unsigned depth_to_goal)

	*/

	const int num_dfa_sps = 2;


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
		std::cout<<"Error: Cannot perform space search on irreversible graphs\n";
		return ret_result;
	}
	if (!cosafe_dfa->getDFA()->isReversible()) {
		std::cout<<"Error: Cannot perform space search on irreversible graph cosafe_dfa\n";
		return ret_result;
	}

	minWeight min_w(p_space_size);

	//auto compare = [](std::pair<int, LexSet*> pair1, std::pair<int, float> pair2) {
	//	return pair1.second > pair2.second;
	//};
	auto compare = [](const std::pair<int, LexSet*>& pair1, const std::pair<int, LexSet*>& pair2) {
		return *(pair1.second) > *(pair2.second);
	};

	bool exit_failure = false;
	std::pair<bool, std::vector<int>> accepting;
	for (int round=0; round<2; ++round) {
		//std::cout<<"\n -- spaceSearch: STARTING ROUND: "<<round<<std::endl;
		std::string search_type = (round == 0) ? "forward" : "reverse";
		std::priority_queue<std::pair<int, LexSet*>, std::vector<std::pair<int, LexSet*>>, decltype(compare)> pq(compare);
		//pq.clear();

		// Tree to keep history as well as parent node list:
		bool found_target_state = false;
		std::vector<bool> visited(p_space_size, false);
		Graph<IVLex> tree;
		//std::vector<int> parents;

		
		// Root tree node has index zero with weight zero
		std::pair<int, LexSet*> curr_leaf;
		if (round == 0) {
			TS_sps->reset();
			for (auto dfa_ptr : *(dfa_list_sps)) {
				dfa_ptr->reset();
			}
			std::vector<int> init_node_inds(num_dfa_sps + 1);
			for (int i=0; i<num_dfa_sps; ++i) {
				init_node_inds[i+1] = dfa_list_ordered->operator[](i)->getCurrNode();
			}
			int init_node_prod_ind = Graph<float>::augmentedStateImage(init_node_inds, graph_sizes);
			//std::cout<<"ROUND 0 INIT NODE IND: "<<init_node_prod_ind<<std::endl;
			min_w.is_inf[init_node_prod_ind] = false;
			min_w.min_weight[init_node_prod_ind] = 0;
			visited[init_node_prod_ind] = true;
			curr_leaf.first = init_node_prod_ind;
			curr_leaf.second = 0.0f;
			pq.push(curr_leaf);
		} else {
			// Add the accepting states to the prio queue with weight zero, and add them to the tree so
			int ROOT_STATE = p_space_size; // this is the root state ind, guaranteed to be larger than any prod state ind
			min_w.reset();
			for (auto acc_prod_state : accepting.second) {
				//std::cout<<"Found accepting prod state: "<<acc_prod_state<<std::endl;
				min_w.is_inf[acc_prod_state] = false;
				min_w.min_weight[acc_prod_state] = 0;
				visited[acc_prod_state] = true;
				spw.state_weights[acc_prod_state] = 0.0f;
				spw.reachability[acc_prod_state] = true;
				std::vector<int> sol_inds;
				Graph<float>::augmentedStatePreImage(graph_sizes, acc_prod_state, sol_inds);
				curr_leaf.first = acc_prod_state;
				curr_leaf.second = 0.0f;
				pq.push(curr_leaf);
				if (depth_limiting) {
					depth_map[acc_prod_state] = 0; // Depth of pin states is defined as zero
				}
				tree.connect(ROOT_STATE, {acc_prod_state, nullptr}); // the root state is the merged accepting state
			}
		}

		float min_accepting_cost = -1;
		int prev_leaf_ind = -1;
		int prod_solution_ind;
		int iterations = 0;
		while (pq.size() > 0) {
			iterations++;

			//printQueueFloat(pq);

			curr_leaf = pq.top();
			pq.pop();
			int curr_leaf_ind = curr_leaf.first;
			int curr_leaf_depth = depth_map.at(curr_leaf_ind);
			if (depth_limiting && curr_leaf_depth >= max_depth) {
				continue;
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

			// SET:
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
				// Reset after checking connected nodes
				TS->set(ret_inds[0]);
				//dfa_sps->set(ret_inds[1]);
				for (int i=0; i<num_dfa_sps; ++i) {
					dfa_list_sps->operator[](i)->set(ret_inds[i+1]);
				}
				std::string temp_str = con_data[j]->label;
				float temp_weight = con_data[j]->weight;
				bool found_connection = true;
				int prev_ts_ind;

				if (round == 0) {
					found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
				} else {
					prev_ts_ind = TS->getCurrNode();
					found_connection = TS->evalReverse(temp_str, true); // second arg tells wether or not to evolve on graph
				
				}
				if (!found_connection) {
					std::cout<<"Error ("<<search_type<<"): Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
					return false;
				}
				const std::vector<std::string>* lbls;
				if (round == 0){
					lbls = TS->returnStateLabels(TS->getCurrNode());
				} else {
					lbls = TS->returnStateLabels(prev_ts_ind);
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
							std::cout<<"Error ("<<search_type<<"): Did not find connectivity in DFA. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
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
						} //else if (min_w.min_weight[connected_node_ind] < weight) { // Candidate node is less optimal than previous found node (don't care about it)
						//}
						continue;
					} else {
						min_w.is_inf[connected_node_ind] = false;
						min_w.min_weight[connected_node_ind] = weight;
					}

					//first_search_weights[connected_node_ind] = curr_leaf_weight + temp_weight;
					std::pair<int, float> new_leaf = {connected_node_ind, weight};
					pq.push(new_leaf); // add to prio queue
					tree.connect(curr_leaf_ind, {connected_node_ind, &min_w.min_weight[connected_node_ind]});
					//included[connected_node_ind] = true;
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
							} //else if (min_w.min_weight[temp_connected_node_ind] < weight) {
							//}
							continue;
						} else {
							min_w.is_inf[temp_connected_node_ind] = false;
							min_w.min_weight[temp_connected_node_ind] = weight;
							spw.reachability[temp_connected_node_ind] = true; // mark the new new as reachable
						}
						if (depth_limiting) {
							depth_map[temp_connected_node_ind] = depth;
                            spw.state_weights[temp_connected_node_ind] = spwFunc(weight, depth);
						} else {
                            spw.state_weights[temp_connected_node_ind] = spwFunc(weight, 0); 
						}
						std::pair<int, float> new_leaf = {temp_connected_node_ind, weight};
						pq.push(new_leaf); // add to prio queue
						tree.connect(curr_leaf_ind, {temp_connected_node_ind, &min_w.min_weight[temp_connected_node_ind]});
						//included[temp_connected_node_ind] = true;
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
		return true;
	} else {
		std::cout<<"Failed space search."<<std::endl;
		return false;
	}
}

template<class T>
bool SymbSearch<T>::generateRisk(DFA_EVAL* cosafe_dfa, spaceWeight& spw) {
	std::vector<DFA_EVAL*> co_safe_dfa_vec = {cosafe_dfa};
	auto spwFunc = [](float ts_cost_to_goal, unsigned depth_to_goal) {
		//return static_cast<float>(depth_to_goal);
		return 1.0f // Return a value of 1.0 for any state with imminent risk
	}
	bool success = spaceSearch(TS, &co_safe_dfa_vec, spw, spwFunc, 1); // max_depth of 1 determines risk
	if (!success) {
		std::cout<<"Error: spaceSearch failed.\n";
		return false;
	}
	return true;
}

template<class T>
bool SymbSearch<T>::generateHeuristic() {
	heuristic.resize(num_dfas);
	for (int i=0; i<num_dfas; ++i) {
		heuristic[i].dfa_ind = i;
		std::vector<DFA_EVAL*> dfa_list_single = {dfa_list_ordered->operator[](i)};
		auto spwFunc = [](float ts_cost_to_goal, unsigned depth_to_goal) {
			return ts_cost_to_goal;
		}
		bool success = spaceSearch(TS, &dfa_list_single, spwFunc);
		if (!success) {
			std::cout<<"Error: spaceSearch failed on dfa: "<<i<<"\n";
			return false;
		}
		// Print the state weights:
		//std::cout<<"\nPRINTING HEURISTIC:"<<std::endl;
		//for (int ii=0; ii<heuristic[i].state_weights.size(); ++ii) {
		//	if (heuristic[i].reachability[ii]) {
		//		std::cout<<"Prod ind:"<<ii<<"  Weight: "<<heuristic[i].state_weights[ii]<<std::endl;
		//	}
		//}
	}
	return true;
}

template<class T>
float SymbSearch<T>::pullStateWeight(unsigned ts_ind, unsigned dfa_ind, unsigned dfa_list_ind, bool& reachable) const {	
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

template<class T>
void SymbSearch<T>::resetSearchParameters() {
	pathlength = 0.0;
	TS_action_sequence.clear();
	TS_state_sequence.clear();
}

template<class T>
std::pair<bool, float> SymbSearch<T>::search(bool use_heuristic) {
	resetSearchParameters();
	std::pair<bool, float> ret_vals;
	if (!TS_set || !dfas_set || !mu_set) {
		std::cout<<"Error: Forgot to set TS, dfas, or mu.\n";
		ret_vals.second = false;
		return ret_vals;
	}
	auto compareLEX = [](const std::pair<int, T*>& pair1, const std::pair<int, T*>& pair2) {
		return *(pair1.second) > *(pair2.second);
	};
	auto accCompareLEX = [](const T& curr_sol, const T& min_acc_sol){
		return curr_sol < min_acc_sol;
	};
	auto ___ = [](const T&) {return true;};

	PlanResult result_LEX(mu, num_dfas);
	//T prune_bound(mu, num_dfas);
	//prune_bound = BFS(compareLEX, accCompareLEX, ___, false, false, use_heuristic); // No pruning criteria, no need for path
	if (use_benchmark) {benchmark.pushStartPoint("first_search");}
	result_LEX = BFS(compareLEX, accCompareLEX, ___, false, false, true); // No pruning criteria, no need for path
	if (use_benchmark) {benchmark.measureMilli("first_search");}
	if (!result_LEX.success) {
		ret_vals.second = false;
		std::cout<<"Info: First search failed."<<std::endl; //TODO move this to verbose
		return ret_vals;
	}
	if (verbose) {
		std::cout<<"Printing prune bound: "<<std::endl;
		result_LEX.pathcost.print();
	}
	auto pruneCriterionMAX = [&prune_bound=result_LEX.pathcost](const T& arg_set) {

		//std::cout<<"   prune bound set:"<<std::endl;
		//prune_bound.print();
		//int in;
		//std::cin>>in;
		return (!prune_bound.withinBounds(arg_set));
	};
	auto compareMAX = [](const std::pair<int, T*>& pair1, const std::pair<int, T*>& pair2) {
		return pair1.second->getMaxVal() > pair2.second->getMaxVal();
	};
	auto accCompareMAX = [](const T& curr_sol, const T& min_acc_sol){
		if (min_acc_sol.isInf()) {
			return true;
		}
		return curr_sol.getMaxVal() < min_acc_sol.getMaxVal();
	};

	PlanResult result_solution(mu, num_dfas);
	if (use_benchmark) {benchmark.pushStartPoint("second_search");}
	result_solution = BFS(compareMAX, accCompareMAX, pruneCriterionMAX, true, true, use_heuristic); 
	if (!result_solution.success) {
		ret_vals.second = false;
		std::cout<<"Info: Second search failed."<<std::endl; //TODO move this to verbose
		return ret_vals;
	}
	if (use_benchmark) {
		benchmark.measureMilli("second_search");
		benchmark.pushAttributesToFile();
	}
	
	if (verbose) {
		std::cout<<"Pathlength: "<<pathlength<<", Max val in solution cost: "<<result_solution.pathcost.getMaxVal()<<"\n";
	}
	ret_vals.first = pathlength;
	ret_vals.second = true;
	return ret_vals;
	//std::cout<<"\nPRINTING SOLUTION COST: "<<std::endl;
	//solution_cost.print();
	//std::cout<<"\n";
	
}

template<class T>
typename SymbSearch<T>::PlanResult SymbSearch<T>::BFS(std::function<bool(const std::pair<int, T*>&, const std::pair<int, T*>&)> compare, std::function<bool(const T&, const T&)> acceptanceCompare, std::function<bool(const T&)> pruneCriterion, bool prune, bool extract_path, bool use_heuristic) {	
	PlanResult ret_result(mu, num_dfas);
	ret_result.success = false;
	//clearNodesAndSets();
	//std::chrono::time_point<std::chrono::system_clock> end_time;
	//std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
	std::vector<const std::vector<std::string>*> total_alphabet(num_dfas);
	std::vector<int> graph_sizes = {static_cast<int>(TS->size())};
	int p_space_size = TS->size();
	for (int i=0; i<num_dfas; ++i) {
		total_alphabet[i] = dfa_list_ordered->operator[](i)->getAlphabetEVAL();
		graph_sizes.push_back(dfa_list_ordered->operator[](i)->getDFA()->size());
		p_space_size *= dfa_list_ordered->operator[](i)->getDFA()->size();
	}
	//std::vector<bool> included(p_space_size, false); // Check if a node has been seen
	
	// ATTENTION: "pq" is indicies are in the product space
	
	// ATTENTION: Tree is indicies are indexed by tree size
	
	// ATTENTION: "visited" is indexed in the product space
	std::vector<bool> visited(p_space_size, false);
	
	// ATTENTION: "parents" is indexed by the tree size for memory space efficiency
	std::vector<int> parents(p_space_size, -1);

	// ATTENTION: Members of min_w are indexed in the product space
	minLS min_w(p_space_size);
	//std::cout<<"PSPACE SIZE: "<<p_space_size<<std::endl;


	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet

	T min_accepting_cost(mu, num_dfas); // Declare here so we can have a return value
	float min_accepting_h_cost;

	//spaceWeight spw;
	//std::cout<<"b4 space search"<<std::endl;
	if (use_heuristic) {
		bool h_success = generateHeuristic();
		if (!h_success) {
			return ret_result;
		}
	}
	//spaceSearch(TS, dfa_list_ordered->operator[](0), spw);
	//std::cout<<"\nPRINTING HEURISTIC\n"<<std::endl;
	//
	//for (int j=0; j<heuristic.size(); ++j) {
	//std::cout<<"\nHEURISTIC: "<<j<<"\n"<<std::endl;
	//	for (int i=0; i<heuristic[j].state_weights.size(); ++i) {
	//		int n = TS->size();
	//		int m = dfa_list_ordered->operator[](0)->getDFA()->size();
	//		std::cout<<"Prod state ind: "<<i;
	//		std::vector<int> temp_inds;
	//		Graph<float>::augmentedStatePreImage({n, m}, i, temp_inds);
	//		std::cout<<", TS state ind: "<<temp_inds[0];
	//		std::cout<<", DFA state ind: "<<temp_inds[1];
	//		std::cout<<",   Weight: "<<heuristic[j].state_weights[i]<<std::endl;
	//	}
	//}

	
	//T prev_sol_weight(mu, num_dfas);
	//prev_sol_weight.setInf();
	//int dfs_iterations = 0;
	//std::vector<int> prev_parents;
	//std::vector<int> parents;
	int solution_ind;
	//while (sol_found) {
	min_w.reset();
	//std::cout<<"sol_found: "<<sol_found<<std::endl;
	//std::cout<<"prev_sol_weight: ";
	//prev_sol_weight.print();
	//dfs_iterations++;
	std::priority_queue<std::pair<int, T*>, std::vector<std::pair<int, T*>>, decltype(compare)> pq(compare);

	// Tree to keep history as well as parent node list:
	Graph<IVFlexLex<T>> tree;
	//std::unordered_map<int, T*> path_length_map;
	//path_length_map.clear();
	//std::vector<T*> path_length_map_ptrs;
	//prev_parents = parents;
	//parents.clear();

	clearNodes();

	// Fill the root tree node (init node):
	TS->reset();

	bool init = true;
	IVFlexLex<T>* temp_nodeptr = newNode();
	//std::cout<<"tmep node ptr: "<<temp_nodeptr<<std::endl;
	//parents.push_back(TS->getCurrNode());
	std::vector<float> temp_lex_set_fill(num_dfas);
	temp_nodeptr->i = TS->getCurrNode();
	std::vector<int> init_node_inds(num_dfas + 1);
	init_node_inds[0] = TS->getCurrNode();
	for (int i=0; i<num_dfas; ++i) {
		dfa_list_ordered->operator[](i)->reset();
		temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();
		init_node_inds[i+1] = dfa_list_ordered->operator[](i)->getCurrNode();
		// Weights are zero for the root node:
		temp_lex_set_fill[i] = 0;
	}
	int init_node_prod_ind = Graph<float>::augmentedStateImage(init_node_inds, graph_sizes);
	//std::cout<<"  MAPPING: "<<init_node_prod_ind<<" TO: "<<node_list.size() - 1;
	min_w.prod2node_list[init_node_prod_ind] = node_list.size() - 1; //map the prod node ind to the node in node list
	visited[init_node_prod_ind] = true;
	min_w.is_inf[init_node_prod_ind] = false;
	temp_nodeptr->lex_set = temp_lex_set_fill;

	//std::cout<<"tmep node ptr: "<<temp_nodeptr<<std::endl;
	//temp_nodeptr->lex_set.print();
	//std::cout<<"tmep node ptr: "<<temp_nodeptr<<std::endl;
	//std::pair<int, T*> curr_leaf = {0, &(temp_nodeptr->lex_set)};
	std::pair<int, T*> curr_leaf;
	//std::cout<<"tmep node ptr: "<<temp_nodeptr<<std::endl;
	curr_leaf.first = init_node_prod_ind;
	//std::cout<<"tmep node ptr: "<<temp_nodeptr<<std::endl;
	curr_leaf.second = &(temp_nodeptr->lex_set);
	pq.push(curr_leaf);
	//std::cout<<"made it past the thing "<<std::endl;
	//if (ITERATE) {
	//	//std::vector<int> fill_vec(num_dfas, 0);
	//	T* temp_weight_set_ptr = new T(mu, num_dfas);
	//	path_length_map_ptrs.push_back(temp_weight_set_ptr);
	//	path_length_map[0] = temp_weight_set_ptr;
	//}

	std::vector<WL*> con_data;
	std::pair<bool, std::vector<int>> accepting;
	min_accepting_cost.setInf();
	//int tree_end_node = 0;

	int iterations = 0;
	int prev_leaf_ind = -1;
	bool finished = false;
	bool sol_found = false;
	//std::cout<<"entering the loop"<<std::endl;
	while (!finished) {
		//std::cout<<"pq size: "<<pq.size()<<std::endl;
		if (pq.empty() || sol_found) {
			finished = true;
			continue;
		}
		iterations++;
		//std::cout<<"iter: "<<iterations<<std::endl;
		//if (extract_path) {
		//std::cout<<"\nprior queue: "<<std::endl;
		//printQueue(pq);
		//std::cout<<"- "<<std::endl;
		//}

		//int pause;
		//std::cin >> pause;
		curr_leaf = pq.top();
		//std::cout<<" TOP --- Ind: "<<pq.top().first<<std::endl;
		//std::cout<<" TOP --- LexSet: ";
		//pq.top().second->print();

		pq.pop();
		//std::cout<<"b4 get leaf"<<std::endl;
		int curr_leaf_prod_ind = curr_leaf.first;
		//std::cout<<"looking for map at key: "<<curr_leaf_prod_ind<<std::endl;
		int curr_leaf_ind = min_w.prod2node_list.at(curr_leaf_prod_ind);
		T* curr_path_weight = &node_list[curr_leaf_ind]->lex_set;
		
		// DEBUGGING vvvv
		//std::vector<int> test_inds = {95, 0, 0, 0, 0, 3};
		//int test_prod = Graph<float>::augmentedStateImage(test_inds, graph_sizes);
		//int test_prod = 226581;
		//int test_prod = 133058;
		//int test_prod = 178925;
		//int test_target = 178926;
		//bool test48 = false;
		//if (prune && curr_leaf_prod_ind == test_prod) {
		//	std::cout<<"------VISITING TEST PROD: "<<test_prod<<std::endl;
		//	std::cout<<"------Curr path weight: "<<curr_path_weight->getMaxVal()<<std::endl;
		//	std::cout<<"------Curr leaf weight: "<<curr_leaf.second->getMaxVal()<<std::endl;
		//	test48 = true;
		//}

		//if (curr_path_weight->getMaxVal() == 47 && prune) {
		//	std::cout<<"LOOKING: curr_path_weight: "<<curr_path_weight->getMaxVal()<<std::endl;
		//	test48 = true;
		//} else {
		//	test48 = false;
		//}
		//T* curr_leaf_weight = curr_leaf.second;

		visited[curr_leaf_prod_ind] = true;

		//bool test_print = false;
		//std::cout<<" ck: curr_leaf_prod_ind: "<<curr_leaf_prod_ind<<std::endl;	
		//if (pq.empty()) {
		//	test_print = true;
		//	std::cout<<"pq empty!"<<std::endl;	
		//	std::cout<<"curr_leaf_prod_ind: "<<curr_leaf_prod_ind<<std::endl;	
		//	std::cout<<"curr_leaf_ind: "<<curr_leaf_ind<<std::endl;	
		//}

		//std::cout<<"af get leaf"<<std::endl;
		//std::cout<<"\n ----- CURR LEAF PROD IND: "<<curr_leaf_prod_ind<<std::endl;
		//std::cout<<" ----- CURR LEAF IND: "<<curr_leaf_ind<<std::endl;
		//std::cout<<" ----- CURR LEAF WEIGHT: "<<std::endl;
		//curr_leaf_weight->print();

		//if (!min_w.is_inf[curr_leaf_ind]
		//if (curr_leaf_ind == prev_leaf_ind) {
		//	std::cout<<"no sol"<<std::endl;
		//	break;
		//}

		//std::cout<<" ------ CURRENT LEAF: "<<curr_leaf_ind<<std::endl;
		//curr_leaf.second->setInf();
		//pq.push(curr_leaf);
		//printQueue(pq);
		//if (!init) { // Use temp_nodeptr from outside the loop (init) when no nodes are in tree
		//	temp_nodeptr = tree.getNodeDataptr(curr_leaf_ind);
		//} else {
		//	init = false;
		//}
		//std::cout<<"af temp node ptr"<<std::endl;
		//std::cout<<"node list size: "<<node_list.size()<<std::endl;
		// SET:
		//std::cout<<"b4 set"<<std::endl;
		TS->set(node_list[curr_leaf_ind]->i);
		//std::cout<<" SET NODE: "<<node_list[curr_leaf_ind]->i;
		for (int i=0; i<num_dfas; ++i) {
			//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
			dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
		}
		//std::cout<<"af set"<<std::endl;

		//std::cout<<" ---\n";
		//if (TS->getCurrNode() == 45) {
		//	std::cout<<"^^^^^^^^^^VISITED NODE 45"<<std::endl;
		//	for (int i=0; i<num_dfas; ++i) {
		//		std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
		//	}
		//	std::cout<<"\n";
		//	std::cout<<"^^^^^^^^^^CURR PATH WEIGHT: "<<curr_path_weight->getMaxVal()<<std::endl;
		//}
		//if (test48) {
		//	std::cout<<"CURRENT NODE: "<<TS->getCurrNode();
		//	for (int i=0; i<num_dfas; ++i) {
		//		std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
		//	}
		//	std::cout<<"\n";
		//}
		//std::cout<<"b4 get con"<<std::endl;
		con_data.clear();
		TS->getConnectedDataEVAL(con_data);
		std::vector<int> con_nodes;
		con_nodes.clear();
		TS->getConnectedNodesEVAL(con_nodes);

		//std::cout<<"af get con node data"<<std::endl;
		//std::cout<<"af get con"<<std::endl;
		//std::cout<<"printing con nodes and data"<<std::endl;
		////std::cout<<"con nodes size:"<<con_nodes.size()<<std::endl;
		////std::cout<<"con data size:"<<con_data.size()<<std::endl;
		//for (int i=0; i<con_data.size(); ++i) {
		//	std::cout<<con_nodes[i]<<std::endl;
		//	std::cout<<con_data[i]->label<<std::endl;
		//}
		//std::pair<unsigned int, IVFlexLex<T>*> src;
		//src.first = curr_leaf_ind;
		//src.second = temp_nodeptr;
		//for (auto con_data_ptr : con_data) {
		//std::cout<<"entering da loop 2"<<std::endl;
		for (int j=0; j<con_data.size(); ++j) {
			//std::cout<<"\n";
			//std::cout<<"j = "<<j<<std::endl;
			//if (test48) {
			//	std::cout<<"  in loop: curr_leaf_ind: "<<curr_leaf_ind<<" ts set state: "<<node_list[curr_leaf_ind]->i;
			//}
			TS->set(node_list[curr_leaf_ind]->i);
			for (int i=0; i<num_dfas; ++i) {
				//if (test48) {
				//	std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
				//	if (i == num_dfas - 1) {
				//		std::cout<<"\n";
				//	}
				//}
				dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
			}
			//printQueue(pq);
			//std::string temp_str = con_data_ptr->label;
			//float temp_weight = con_data_ptr->weight;
			std::string temp_str = con_data[j]->label;
			float temp_weight = con_data[j]->weight;
			//std::cout<<"temp label: "<<temp_str<<std::endl;
			//std::cout<<"temp weight: "<<temp_weight<<std::endl;
			bool found_connection = true;

			//std::cout<<"b4 da eval"<<std::endl;
			//std::cout<<"b4 eval"<<std::endl;
			
			// Evaluate each graph under the current action
			found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
			if (!found_connection) {
				std::cout<<"Error: Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
				return ret_result;
			}
			for (int i=0; i<num_dfas; ++i) {
				// There could be multiple logical statements the correspond to the same
				// underlying label, therefore we must test all of the statements until
				// one is found. Only one is needed due to determinism.
				const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
				found_connection = false;
				//std::cout<<"\n before eval"<<std::endl;
				for (int ii=0; ii<lbls->size(); ++ii) {
					//std::cout<<"       labels: --- "<<lbls->operator[](ii)<<std::endl;
					//std::cout<<"DFA::: "<<(i-1)<<" curr node; "<<(dfa_list_ordered->operator[](i-1)->getCurrNode())<<std::endl;
					if (dfa_list_ordered->operator[](i)->eval(lbls->operator[](ii), true)) {
						found_connection = true;
						break;
					}
				}
				if (!found_connection) {
					std::cout<<"Error: Connectivity was not found for either the TS or a DFA. \n";
					return ret_result;
				}
			}
			//std::cout<<"af eval"<<std::endl;
			// Check if the node has already been seen:
			//std::cout<<"\n Uniqueness check: \n"<<std::endl;
			//std::cout<<"printing nodelist: "<<std::endl;

			// Collect the nodes that each graph has transitioned to to get the product state
			std::vector<int> node_inds(num_dfas + 1);
			node_inds[0] = TS->getCurrNode();
			for (int i=0; i<num_dfas; ++i) {
				node_inds[i+1] = dfa_list_ordered->operator[](i)->getCurrNode();
			}
			int con_node_prod_ind = Graph<float>::augmentedStateImage(node_inds, graph_sizes);

			//if (test48) {
			//	std::cout<<"  con node:";
			//	for (int i=0; i<node_inds.size(); ++i) {
			//		std::cout<<" "<<node_inds[i];
			//	}

			//int testi = 559;
			//bool test74 = false;
			//if (con_node_prod_ind == testi || con_node_prod_ind == 1049) {
			//	std::cout<<"  con node prod: "<< con_node_prod_ind<<std::endl;
			//	test74 = true;
			//}
			//}

			//// 2028 <- 2038

			//bool test96 = false;
			//if (con_node_prod_ind == test_target) {
			//	std::cout<<"FOUND NODE!!!!"<<std::endl;
			//	test96 = true;
			//}

			// Only consider unvisited connected nodes
			//if (use_heuristic) {
			//	//if (pruned[con_node_prod_ind]) { // Node was pruned
			//	//	if (test48 && test96) {
			//	//		std::cout<<"    -pruned before."<<std::endl;
			//	//	}
			//	//	continue;
			//	//}
			//} else {
			if (!use_heuristic) {
				if (visited[con_node_prod_ind]) { // Node was visited
					//if (test48 && test96) {
					//	if (visited[con_node_prod_ind]) {
					//		std::cout<<"    -visited."<<std::endl;
					//	} else {
					//		std::cout<<"    -pruned before."<<std::endl;
					//	}
					//}
					continue;
				}
			}
			//std::cout<<"af visited"<<std::endl;

			// Now check if the node is accepting, if not, append the action weight for that formula
			bool all_accepting = true;
			for (int i=0; i<num_dfas; ++i) {

				// If the dfa is accepting at the evolved ind, append no cost, otherwise append
				// the cost of taking the action:
				//std::cout<<"-DFA: "<<i<<", curr becore accepting: "<<dfa_list_ordered->operator[](i)->getCurrNode()<<std::endl;
				//std::cout<<" ->DFA: ";
				if (dfa_list_ordered->operator[](i)->isCurrAccepting()) {
					//std::cout<<i<<" is accepting"<<std::endl;
					temp_lex_set_fill[i] = 0;
				} else {
					//std::cout<<i<<" NOT ACCEPTING"<<std::endl;
					all_accepting = false;
					temp_lex_set_fill[i] = temp_weight;
				}
				//std::cout<<temp_lex_set_fill[i]<<" ";
			}
			//std::cout<<"\n";

			// Only consider non pruned nodes
			if (prune) {
				T temp_prune_check(mu, num_dfas); 
				temp_prune_check = *(curr_path_weight);
				temp_prune_check += temp_lex_set_fill;
				//if (temp_prune_check.getMaxVal() > 26) {
				//	std::cout<<"		ERROR FOUND BAD NODE: "<<temp_prune_check.getMaxVal();
				//	std::cout<<", curr_path_weight: "<<curr_path_weight->getMaxVal()<<std::endl;
				//}
				if (pruneCriterion(temp_prune_check)) {
					//min_w.is_inf[con_node_prod_ind] = false; // mark node as seen
					//visited[con_node_prod_ind] = false;
					//pruned[con_node_prod_ind] = true;
					//if (test48 && test96) {std::cout<<"    -pruned."<<std::endl;}
					continue;
				}
			}
			
			// Check if node was seen before to see if a shorter path has been found
			if (!min_w.is_inf[con_node_prod_ind]) { // Node was seen before, non inf weight, but not visited
				int seen_node_ind = min_w.prod2node_list.at(con_node_prod_ind); // This value will be mapped if weve seen the node before
				IVFlexLex<T>* seen_node = node_list[seen_node_ind];
				T temp_lex_set(mu, &temp_lex_set_fill, num_dfas);
				temp_lex_set += *curr_path_weight;
				//std::cout<<"printing seen node lex set: "<<std::endl;
				//temp_node->lex_set.print();
				//std::cout<<"printing temp lex set: "<<std::endl;
				//temp_lex_set.print();
				bool updated = false;
				if (prune) {
					if (seen_node->lex_set.getMaxVal() > temp_lex_set.getMaxVal()) {
						//std::cout<<"UPDATING NODE: "<<seen_node_ind<<" to: "<<curr_leaf_ind<<std::endl;
						seen_node->lex_set = temp_lex_set;
						// UPDATE PARENT HERE vvvvvv
						parents[seen_node_ind] = curr_leaf_ind;
						updated = true;
						//if (test48) {std::cout<<"     -updated."<<std::endl;}
						//if (con_node_prod_ind == test_target) {
						//	std::cout<<"### upd the target, parent node: "<<curr_leaf_prod_ind<<" parent node cost: "<<curr_path_weight->getMaxVal()<< std::endl;
						//}
					}
					//if (test48) {
					//	std::cout<<"     -already seen not updated."<<std::endl;
					//	//seen_node->lex_set.print();
					//}

				} else {
					if (seen_node->lex_set > temp_lex_set) {
						//std::cout<<"UPDATING NODE: "<<seen_node_ind<<" to: "<<curr_leaf_ind<<std::endl;
						seen_node->lex_set = temp_lex_set;
						// UPDATE PARENT HERE vvvvvv
						parents[seen_node_ind] = curr_leaf_ind;
						updated = true;
						//if (test48) {std::cout<<"     -updated."<<std::endl;}
					}
					//if (test48 && test96) {
					//	std::cout<<"     -already seen not updated."<<std::endl;
					//	//seen_node->lex_set.print();
					//}
				}
				if (!updated || !use_heuristic) {
					continue;
				}
			} 
			IVFlexLex<T>* new_temp_nodeptr = newNode();
			//std::cout<<"  MAPPING: "<<con_node_prod_ind<<" TO: "<<node_list.size() - 1<<std::endl;
			min_w.prod2node_list[con_node_prod_ind] = node_list.size() - 1; // Make new node, must map the tree and prod indices
			//std::cout<<" mapping prod node: "<<con_node_prod_ind<<" to tr node: "<<node_list.size() -1<<std::endl;
			int con_node_ind = node_list.size() - 1;
			//std::cout<<"ADDING NODE: "<<con_node_ind<<std::endl;
			//std::cout<<"ADDING PROD NODE: "<<con_node_prod_ind<<std::endl;
			//std::cout<<"   COMMITTED con_node_ind: "<<con_node_ind<<std::endl;
			//parents.push_back(src.first);
			new_temp_nodeptr->i = TS->getCurrNode();
			for (int i=0; i<num_dfas; ++i) {
				new_temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();
			}
			//std::cout<<"af update"<<std::endl;

			new_temp_nodeptr->lex_set = *(curr_path_weight);
			new_temp_nodeptr->lex_set += temp_lex_set_fill;

			//std::cout<<"printing add node set: \n";
			//new_temp_nodeptr->lex_set.print();

			// BUILD CONNECTION:
			//tree_end_node++;
			//tree_end_node = con_node_prod_ind;

			// The priority queue includes the heuristic cost, however the tree does not:
			std::pair<int, T*> new_leaf;
			//T temp_weight_set(mu, num_dfas);
			//T* temp_weight_set_ptr;
			//if (ITERATE) {
			//	temp_weight_set_ptr = new T(mu, num_dfas);
			//	path_length_map_ptrs.push_back(temp_weight_set_ptr);
			//}
			if (use_heuristic) {
				float max_h_val = 0.0;
				std::vector<float> lex_h_vals(num_dfas, 0.0);
				for (int i=0; i<num_dfas; ++i) {
					bool reachable;
					if (!dfa_list_ordered->operator[](i)->isCurrAccepting()) {
						//h_vals[i] = 0;
						//h_vals[i] = pullStateWeight(TS->getCurrNode(), dfa_list_ordered->operator[](i)->getCurrNode(), i, reachable);
						float h_val = pullStateWeight(TS->getCurrNode(), dfa_list_ordered->operator[](i)->getCurrNode(), i, reachable);
						if (!reachable) {
							std::cout<<"Error: Attempted to find heuristic distance for unreachable product state (ts: "<<TS->getCurrNode()<<" dfa "<<i<<": "<<dfa_list_ordered->operator[](i)->getCurrNode()<<") \n";
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
					// else {
						//std::cout<<"dfa curr node: "<<dfa_list_ordered->operator[](i)->getCurrNode()<<" is it accepting? "<<dfa_list_ordered->operator[](i)->isCurrAccepting()<<std::endl;
					//}
				}
				T* new_temp_setptr = newSet();
				//if (!ITERATE) { // ITERATE determine whether or not to use A* or DFS
				//new_temp_setptr->operator=(*curr_leaf_weight);
				new_temp_setptr->operator=(new_temp_nodeptr->lex_set);
				
				//if (test74) {
				//	std::cout<<"b4 add h"<<std::endl;
				//	new_temp_setptr->print();
				//	std::cout<<"max h val: "<<max_h_val<<std::endl;
				//}
				//new_temp_setptr->operator+=(fill_set);
				if (prune) {
					new_temp_setptr->addToMax(max_h_val);
					//if (test48) {
					//	std::cout<<"     hval: "<<max_h_val<<std::endl;
					//}
				} else {
					*new_temp_setptr += lex_h_vals;
				}
				//new_temp_setptr->addHeuristic(h_vals);
				//} else {
				//	temp_weight_set_ptr->operator=(new_temp_nodeptr->lex_set);
				//	new_temp_setptr->operator=(fill_set);
				//}
				//if (prune) {
				//	if (pruneCriterion(*(new_temp_setptr))) {
				//		//min_w.is_inf[con_node_prod_ind] = false; // mark node as seen
				//		//visited[con_node_prod_ind] = false;
				//		//pruned[con_node_prod_ind] = true;
				//		//if (test48 && test96) {std::cout<<"    -pruned."<<std::endl;}
				//		continue;
				//	}
				//}
				new_leaf = {con_node_prod_ind, new_temp_setptr};
				//if (test74) {
				//	std::cout<<"af add h (f)"<<std::endl;
				//	new_temp_setptr->print();
				//}
			} else {
				new_leaf = {con_node_prod_ind, &(new_temp_nodeptr->lex_set)};
			}
			//if (ITERATE) {
			//	//std::cout<<"adding lex set: ";
			//	//path_length_map[curr_leaf_ind]->print();
			//	////new_temp_nodeptr->lex_set.print();
			//	//std::cout<<"prev sol lex set: ";
			//	//prev_sol_weight.print();
			//	temp_weight_set_ptr->operator+=(*(path_length_map[curr_leaf_ind]));
			//	if ( *(temp_weight_set_ptr) >= prev_sol_weight) {
			//		//std::cout<<"WE CONTINUIING..."<<std::endl;
			//		continue;
			//	}
			//}
			//std::cout<<"b4 connect"<<std::endl;

			//std::vector<float> tmc = {245, 0, 0, 0, 0};
			//T test_min_cost(mu, num_dfas);
			//test_min_cost = tmc;
			//if (*(new_leaf.second) > test_min_cost) {
			//	std::cout<<" \nNOT ADMISSIBLE!!!!!"<<std::endl;
			//}
			//if (con_node_prod_ind == test_target) {
			//	std::cout<<"### found the target, parent node: "<<curr_leaf_prod_ind<<" parent node cost: "<<curr_path_weight->getMaxVal()<< std::endl;
			//}

			// Only consider non pruned nodes

			pq.push(new_leaf); // add to prio queue
			tree.connect(curr_leaf_ind, {con_node_ind, new_temp_nodeptr});
			min_w.is_inf[con_node_prod_ind] = false; // mark node as seen
			parents[con_node_ind] = curr_leaf_ind;
			//std::cout<<"af connect"<<std::endl;
			//if (ITERATE) {
			//	path_length_map[tree_end_node] = temp_weight_set_ptr;
			//}
			int solution_ind_prod;

			if (all_accepting) {
				//std::cout<<"ALL ACCEPTING!"<<std::endl;
				accepting.first = true;
				//std::cout<<">>>found accepting state: "<<con_node_prod_ind<<std::endl;
				//std::vector<int> ret_inds;
				//Graph<float>::augmentedStatePreImage(graph_sizes, con_node_prod_ind, ret_inds);
				//std::cout<<"   acc TS: "<<ret_inds[0]<<std::endl;
				//for (int i=0; i<ret_inds.size() -1; ++i) {
				//	std::cout<<"   acc dfa "<<i<<": "<<ret_inds[i+1]<<std::endl;
				//}
				accepting.second.push_back(con_node_prod_ind);
				solution_ind_prod = con_node_prod_ind;
				//std::cout<<"  --acc prod ind: "<<solution_ind_prod<<std::endl;
				solution_ind = con_node_ind;
				//std::cout<<"  --con node ind: "<<solution_ind<<std::endl;
				//std::cout<<"min accepting cost: "<<std::endl;
				//min_accepting_cost.print();
				//std::cout<<"found cost: "<<std::endl;
				//new_leaf.second->print();
				if (acceptanceCompare(*(new_leaf.second), min_accepting_cost)) { // new_leaf.second < min_accepting_cost
					min_accepting_cost = *(new_leaf.second);
					//min_accepting_cost.print();
				}
				//if (*(new_leaf.second) < min_accepting_cost) {
				//	min_accepting_cost = *(new_leaf.second);
				//}
			}
			if (accepting.first) {
				//if (*(pq.top().second) >= min_accepting_cost) {
				if (!acceptanceCompare(*(pq.top().second), min_accepting_cost)) {
					finished = true;
					//std::cout<<"Found a solution!"<<"\n";
					//std::cout<<"SOLUTION IND: "<<solution_ind<<std::endl;
					std::cout<<"SOLUTION IND PROD: "<<solution_ind_prod<<std::endl;
					//std::cout<<"   -Iterations: "<<iterations<<"\n";
					//solution_ind = min_w.prod2node_list.at(pq.top().first);
					//solution_ind = *(pq.top().first);
					sol_found = true;
					break;
				}
			}
		}

		// If accepting node is found, check if it is the smallest cost candidate on the next iteration.
		// If so, then the algorithm is finished because all other candidate nodes have a longer path.
		//if (accepting.first) {
		//	int top_node = pq.top().first;

		//	for (int i=0; i<accepting.second.size(); ++i) {
		//		if (top_node == accepting.second[i]) {
		//			//if (ITERATE) {
		//			//	std::cout<<"printing top weight: ";
		//			//	path_length_map[top_node]->print();
		//			//	prev_sol_weight = *(path_length_map[top_node]);
		//			//}
		//			finished = true;
		//			solution_ind = top_node;
		//			std::cout<<"Found a solution!"<<std::endl;
		//			std::cout<<" Iterations: "<<iterations<<std::endl; 
		//			sol_found = true;
		//			break;
		//		}
		//	}
		//}
		prev_leaf_ind = curr_leaf_ind;
	}
	//for (int i=0; i<path_length_map_ptrs.size(); ++i) {
	//	delete path_length_map_ptrs[i];
	//}
	//	if (!ITERATE) {
	if (finished && sol_found) {
		if (extract_path) {
			extractPath(node_list, parents, solution_ind,graph_sizes);
			int p_space_size = 1;
		}
		if (use_benchmark && prune) {
			benchmark.addCustomTimeAttr("iterations", static_cast<double>(iterations), ""); // no units
		}
		ret_result.pathcost = min_accepting_cost;
		ret_result.success = true;
		return ret_result;
	} else {
		std::cout<<"Failed (no plan)."<<std::endl;
		return ret_result;
	}
}

template<class T>
void SymbSearch<T>::extractPath(const std::vector<int>& parents, int accepting_state, const std::vector<int>& graph_sizes) {
	int curr_node = 0;
	curr_node = accepting_state;
	std::vector<int> reverse_TS_state_sequence;
	std::vector<int> reverse_prod_state_sequence;
	reverse_TS_state_sequence.clear();
	reverse_prod_state_sequence.clear();
	while (curr_node != 0) {
		reverse_TS_state_sequence.push_back(node_list[curr_node]->i);
		std::vector<int> prod_inds(num_dfas + 1);
		prod_inds[0] = node_list[curr_node]->i;
		for (int i=0; i<num_dfas; ++i) {
			prod_inds[i+1] = node_list[curr_node]->v[i];
		}
		int prod_ind_out = Graph<float>::augmentedStateImage(prod_inds, graph_sizes);
		reverse_prod_state_sequence.push_back(prod_ind_out);
		curr_node = parents[curr_node];
	}
	reverse_TS_state_sequence.push_back(node_list[0]->i); // finally add the init state
	TS_state_sequence.resize(reverse_TS_state_sequence.size());
	TS_action_sequence.resize(reverse_TS_state_sequence.size()-1);
	std::vector<int> prod_state_sequence(reverse_prod_state_sequence.size());
	if (verbose) {
		std::cout<<"Info: Successfully extracted plan!\n";
		std::cout<<"State sequence: ";
	}
	//std::cout<<"first state: "<<reverse_TS_state_sequence[0]<<std::endl;
	for (int i=0; i<reverse_TS_state_sequence.size(); ++i) {
		//std::cout<<"i: "<<i<<" size(): "<<TS_state_sequence.size()<<std::endl;
		TS_state_sequence[i] = reverse_TS_state_sequence[reverse_TS_state_sequence.size()-1-i];
		if (verbose) {
			std::cout<<" -> "<<TS_state_sequence[i];
		}
	}
	if (verbose) {
		std::cout<<"\n";
	}

	if (verbose) {
		std::cout<<"\nProduct State sequence: ";
	}
	//std::cout<<"first state: "<<reverse_TS_state_sequence[0]<<std::endl;
	for (int i=0; i<reverse_prod_state_sequence.size(); ++i) {
		//std::cout<<"i: "<<i<<" size(): "<<TS_state_sequence.size()<<std::endl;
		prod_state_sequence[i] = reverse_prod_state_sequence[reverse_prod_state_sequence.size()-1-i];
		if (verbose) {
			std::cout<<" -> "<<prod_state_sequence[i];
		}
	}
	if (verbose) {
		std::cout<<"\n";
	}

	//TS->reset();
	std::vector<WL*> con_data;
	std::vector<int> con_nodes;
	pathlength = 0.0;
	for (int i=0; i<TS_action_sequence.size(); ++i) {
		//std::cout<<"b4 set"<<std::endl;
		TS->set(TS_state_sequence[i]);
		//std::cout<<"af set b4 con data"<<std::endl;
		//std::cout<<"curr node: "<<TS->getCurrNode()<<std::endl;
		TS->getConnectedDataEVAL(con_data);
		TS->getConnectedNodesEVAL(con_nodes);
		//std::cout<<"af con data and nodes  b4 search"<<std::endl;
		for (int ii=0; ii<con_nodes.size(); ++ii){
			if (con_nodes[ii] == TS_state_sequence[i+1]) {
				TS_action_sequence[i] = con_data[ii]->label;
				pathlength += con_data[ii]->weight;
			}
		}
		//std::cout<<"af search"<<std::endl;
	}
	if (verbose) {
		std::cout<<"Info: Number of actions: "<<TS_action_sequence.size()<<", pathlength: "<<pathlength<<"\n";
	}
}

template<class T>
void SymbSearch<T>::writePlanToFile(std::string filename, const std::vector<std::string>& xtra_info) {
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

template<class T>
void SymbSearch<T>::clearNodes() {
	for (int i=0; i<node_list.size(); ++i) {
		delete node_list[i];
	}
	node_list.clear();
}

template<class T>
void SymbSearch<T>::clearNodesLS() {
	for (int i=0; i<node_list_ls.size(); ++i) {
		delete node_list_ls[i];
	}
	node_list_ls.clear();
}

template<class T>
void SymbSearch<T>::clearNodesAndSets() {
	for (int i=0; i<node_list.size(); ++i) {
		delete node_list[i];
	}
	node_list.clear();
	for (int i=0; i<set_list.size(); ++i) {
		delete set_list[i];
	}
	set_list.clear();
}

template<class T>
SymbSearch<T>::~SymbSearch() {
	for (int i=0; i<node_list.size(); ++i) {
		delete node_list[i];
	}
	for (int i=0; i<set_list.size(); ++i) {
		delete set_list[i];
	}
}

//template class SymbSearch<LexSet>; CANNOT USE BECAUSE CTOR REQUIRES MU
//template class SymbSearch<FlexLexSetS>;
//template class SymbSearch<REQLex>;
template class SymbSearch<DetourLex>;


