#include "symbSearch.h"
#include<fstream>
#include<chrono>
#include<ctime>

template<class T>
SymbSearch<T>::SymbSearch() {}

template<class T>
void SymbSearch<T>::setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_) {
	node_list.clear();
	set_list.clear();
	dfa_list_ordered = dfa_list_ordered_;
	num_dfas = dfa_list_ordered->size();
}

template<class T>
void SymbSearch<T>::setTransitionSystem(TS_EVAL<State>* TS_) {
	TS = TS_;	
}

template<class T>
void SymbSearch<T>::setFlexibilityParam(float mu_) {
	mu = mu_;
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
T* SymbSearch<T>::newSet() {
	T* set_i = new T(mu, num_dfas);
	//node_i->v.resize(num_dfas);
	set_list.push_back(set_i);
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
	while (!queue.empty()) {
		std::cout<<"Ind: "<<queue.top().first<<std::endl;
		std::cout<<"LexSet: ";
		queue.top().second->print();
		queue.pop();
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
bool SymbSearch<T>::spaceSearch(TS_EVAL<State>* TS_sps, std::vector<DFA_EVAL*>* dfa_list_sps, spaceWeight& spw) {
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
	spw.state_weights.resize(p_space_size);
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
			//std::cout<<iterations<<std::endl;
			//std::cout<<"in loop"<<std::endl;
			//pq.top();
			//std::cout<<"Prior queue size(): "<<pq.size()<<std::endl;

			//printQueueFloat(pq);
			//int pause;
			//std::cin >> pause;

			curr_leaf = pq.top();
			//std::cout<<" TOP --- Ind: "<<pq.top().first<<std::endl;
			//std::cout<<" TOP --- LexSet: ";
			//pq.top().second->print();

			pq.pop();
			int curr_leaf_ind = curr_leaf.first;
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
							//std::cout<<"DID NOT FIND CONEC OIOI"<<std::endl;
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
						spw.state_weights[temp_connected_node_ind] = weight;
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
		//std::cout<<"made it out of space search"<<std::endl;
		//if ((round == 0) && !found_target_state) {
		//	std::cout<<"Error: Target state not found\n";
		//	exit_failure = true; // currently not using this
		//	return false;
		//}
		if (round == 1) {
			for (int i=0; i<spw.reachability.size(); ++i) {
				if (!spw.reachability[i]) {
					std::cout<<"Warning: Not all states were searched (round 2). Failed prod state ind: "<<i<<std::endl;
					std::vector<int> temp_inds;
					Graph<float>::augmentedStatePreImage(graph_sizes, i, temp_inds);
					std::cout<<"   TS ind: "<<temp_inds[0]<<std::endl;
					std::cout<<"   DFA ind: "<<temp_inds[1]<<std::endl;
				}
			}
		}
	}
	if (!exit_failure) {
		//extractPath(parents, solution_ind);
		//int p_space_size = 1;
		//p_space_size *=
		//for 

		//end_time = std::chrono::system_clock::now();
		//double search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		//std::cout<<"Search time (milliseconds): "<<search_time<<std::endl;
		//std::cout<<"Finished. Tree size: "<<tree.size()<<std::endl; //<<", Maximum product graph (no pruning) size: "<<
		//std::cout<<"\n\n ...Finished with plan."<<std::endl;
		return true;
	} else {
		std::cout<<"Failed space search."<<std::endl;
		return false;
	}
}

template<class T>
bool SymbSearch<T>::riskSearch(TS_EVAL<State>* TS_sps, DFA_EVAL* dfa_sps, spaceWeight& spw, std::function<float(unsigned int)> cFunc) {
	if (!TS_sps->isReversible() || !dfa_sps->getDFA()->isReversible()) {
		std::cout<<"Error: Cannot perform space search on irreversible graphs\n";
		return false;
	}
	int n = TS_sps->size();
	int m = dfa_sps->getDFA()->size();
	int p_space_size = n * m;
	std::vector<int> graph_sizes = {n, m};

	// These indices are determined after the first search round:
	//int TS_accepting_state = -1;

	//std::vector<int> first_search_weights(p_space_size, -1); 
	//std::vector<int> second_search_weights(p_space_size, -1);
	spw.state_weights.resize(p_space_size);
	spw.reachability.resize(p_space_size, false);
	spw.is_inf.resize(p_space_size, false);

	minWeight min_w(p_space_size);
	
	// Search: 

	//std::chrono::time_point<std::chrono::system_clock> end_time;
	//std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
	std::vector<const std::vector<std::string>*> total_alphabet(1);
	total_alphabet[0] = dfa_sps->getAlphabetEVAL();
	//for (int i=0; i<num_dfas; ++i) {
	//}
	//TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet
	auto compare = [](std::pair<int, int> pair1, std::pair<int, int> pair2) {
		return pair1.second > pair2.second;
	};

	bool exit_failure = false;
	std::pair<bool, std::vector<int>> accepting;
	for (int round=0; round<2; ++round) {
		std::cout<<"\n -- riskSearch: STARTING ROUND: "<<round<<std::endl;
		std::string search_type = (round == 0) ? "forward" : "reverse";
		std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(compare)> pq(compare);
		//pq.clear();

		// Tree to keep history as well as parent node list:
		bool found_target_state = false;
		std::vector<bool> included(p_space_size, false);
		Graph<float> tree; // values represent the path distance AKA the tree depth

		// Root tree node has index zero with weight zero
		std::pair<int, int> curr_leaf;
		if (round == 0) {
			//std::cout<<"b4 reset"<<std::endl;
			TS_sps->reset();
			dfa_sps->reset();
			int init_node_ind = Graph<float>::augmentedStateImage({TS_sps->getCurrNode(), dfa_sps->getCurrNode()}, graph_sizes);
			std::cout<<"ROUND 0 INIT NODE IND: "<<init_node_ind<<std::endl;
			min_w.is_inf[init_node_ind] = false;
			min_w.min_weight[init_node_ind] = 0;

			//first_search_weights[init_node_ind] = -1;
			included[init_node_ind] = true;
			curr_leaf.first = init_node_ind;
			curr_leaf.second = 0;
			pq.push(curr_leaf);
			//std::cout<<"af reset"<<std::endl;
		} else {
			// Add the accepting states to the prio queue with weight zero, and add them to the tree so
			int ROOT_STATE = n * m; // this is the root state ind, guaranteed to be larger than any prod state ind
			min_w.reset();
			for (auto acc_prod_state : accepting.second) {
				min_w.is_inf[acc_prod_state] = false;
				min_w.min_weight[acc_prod_state] = 0;
				included[acc_prod_state] = true;
				spw.state_weights[acc_prod_state] = -1.0f;
				spw.is_inf[acc_prod_state] = true;
				spw.reachability[acc_prod_state] = true;
				std::vector<int> sol_inds;
				Graph<float>::augmentedStatePreImage(graph_sizes, acc_prod_state, sol_inds);
				std::cout<<" ---- Found (TS): "<< sol_inds[0]<<std::endl;
				curr_leaf.first = acc_prod_state;
				curr_leaf.second = 0.0f;
				pq.push(curr_leaf);
				tree.connect(ROOT_STATE, {acc_prod_state, nullptr}); // the root state is the merged accepting state
			}
			std::cout<<"found "<<accepting.second.size()<<" accepting states"<<std::endl;
		}

		float min_accepting_cost = -1;
		int prev_leaf_ind = -1;
		int prod_solution_ind;
		while (pq.size() > 0) {
			//int pause;
			//std::cin >> pause;
			curr_leaf = pq.top();
			//std::cout<<" TOP --- Ind: "<<pq.top().first<<std::endl;
			//std::cout<<" TOP --- LexSet: ";
			//pq.top().second->print();

			pq.pop();
			int curr_leaf_ind = curr_leaf.first;
			int curr_leaf_depth = curr_leaf.second;
			if (!min_w.is_inf[curr_leaf_ind]) {
				int min_leaf_depth = static_cast<int>(min_w.min_weight[curr_leaf_ind]);
				if (curr_leaf_depth > min_leaf_depth ) {
					continue;
				}
			}
			std::vector<int> ret_inds;
			Graph<float>::augmentedStatePreImage(graph_sizes, curr_leaf_ind, ret_inds);

			//if (curr_leaf_ind == prev_leaf_ind) {
			//	std::cout<<"Info: No solution found. "<<std::endl;
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
			dfa_sps->set(ret_inds[1]);

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
			//std::pair<int, int*> src;
			//src.first = curr_leaf_ind;
			//src.second = &curr_leaf_depth;
			//for (auto con_data_ptr : con_data) {
			for (int j=0; j<con_data.size(); ++j) {
				// Reset after checking connected nodes
				TS_sps->set(ret_inds[0]);
				dfa_sps->set(ret_inds[1]);

				//for (int i=0; i<num_dfas; ++i) {
				//	//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
				//	dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
				//}
				//printQueue(pq);
				//std::string temp_str = con_data_ptr->label;
				//float temp_weight = con_data_ptr->weight;
				std::string temp_str = con_data[j]->label;
				//float temp_weight = con_data[j]->weight;
				int temp_depth = 1;
				//std::cout<<"curr_leaf_depth = "<<curr_leaf_depth<<std::endl;
				//float temp_weight = cFunc(curr_leaf_depth + temp_depth);
				//std::cout<<" CFUNC INPUT: "<<curr_leaf_depth + temp_depth<<" OUTPUT: "<<temp_weight<<std::endl;
				//std::cout<<"temp label: "<<temp_str<<std::endl;
				//std::cout<<"temp weight: "<<temp_weight<<std::endl;
				bool found_connection = true;
				int prev_ts_ind;

				if (round == 0) {
					//std::cout<<"forward ts eval"<<std::endl;
					found_connection = TS_sps->eval(temp_str, true); // second arg tells wether or not to evolve on graph
				} else {
					//std::cout<<"reverse ts eval"<<std::endl;
					prev_ts_ind = TS_sps->getCurrNode();
					found_connection = TS_sps->evalReverse(temp_str, true); // second arg tells wether or not to evolve on graph
				
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
					//std::cout<<"ts curr node (4 lblsl) : "<<TS->getCurrNode()<<std::endl;
					lbls = TS->returnStateLabels(TS->getCurrNode());
				} else {
					//std::cout<<"\nts node b4 return state labels: "<<prev_ts_ind<<std::endl;
					lbls = TS->returnStateLabels(prev_ts_ind);
				}
				found_connection = false;
				//std::cout<<"\n before eval"<<std::endl;
				bool found_true = true;
				std::vector<int> parent_node_list;
				//std::cout<<"LBLS SIZE::::: "<<lbls->size()<<std::endl;
				if (round == 0) {
					for (int ii=0; ii<lbls->size(); ++ii) {
						//std::cout<<" INPUT, lbl: "<<lbls->operator[](ii)<<std::endl;
						//std::cout<<"       label: --- "<<lbls->operator[](ii)<<std::endl;
						//std::cout<<"DFA::: "<<(i-1)<<" curr node; "<<(dfa_list_ordered->operator[](i-1)->getCurrNode())<<std::endl;
						//if (round == 0) {
						if (dfa_sps->eval(lbls->operator[](ii), true)) {
							//std::cout<<" OUTPUT EVAL TRUE"<<std::endl;
							found_connection = true;
							break;
						} 
					}
				} else {
					found_connection = dfa_sps->getParentNodesWithLabels(lbls, parent_node_list);
					//std::cout<<"found connection? : "<<found_connection<<" parent_node_list size: "<<parent_node_list.size()<<std::endl;

				}
			
				bool unique = true;
				int connected_node_ind = -1;
				std::vector<int> unique_dfa_parent_set;

				// CHECK IF CANDIDATE NODE WAS ALREADY INCLUDED, IF SO, ADD IT TO THE QUEUE AND TREE:
				int depth = curr_leaf_depth + temp_depth;
				float f_depth = static_cast<float>(depth);
				bool all_included = true;
				if (round == 0) {
					//std::cout<<"Checking acceptance: TS: "<<TS_sps->getCurrNode()<<" DFA: "<<dfa_sps->getCurrNode()<<std::endl;
					connected_node_ind = Graph<float>::augmentedStateImage({TS_sps->getCurrNode(), dfa_sps->getCurrNode()}, graph_sizes);
					if (included[connected_node_ind]) {
						continue;
					}
					if (!min_w.is_inf[connected_node_ind]) { // Candidate node has been found
						if (min_w.min_weight[connected_node_ind] > f_depth) { // Candidate node is more optimal than previous found node
							min_w.min_weight[connected_node_ind] = f_depth;
						} else if (min_w.min_weight[connected_node_ind] < f_depth) { // Candidate node is less optimal than previous found node (don't care about it)
							continue;
						}
					} else {
						min_w.is_inf[connected_node_ind] = false;
						min_w.min_weight[connected_node_ind] = f_depth;
					}
					//first_search_weights[connected_node_ind] = depth;
					std::pair<int, int> new_leaf = {connected_node_ind, depth};
					pq.push(new_leaf); // add to prio queue
					//tree.connect(src, {connected_node_ind, &first_search_weights[connected_node_ind]});
					tree.connect(curr_leaf_ind, {connected_node_ind, &min_w.min_weight[connected_node_ind]});
					included[connected_node_ind] = true;
				} else {
					int temp_connected_node_ind = -1;
					for (auto par_node : parent_node_list) {
						temp_connected_node_ind = Graph<float>::augmentedStateImage({TS_sps->getCurrNode(), par_node}, graph_sizes);
						//std::cout<<"b4 incl"<<std::endl;
						if (included[temp_connected_node_ind]) {
							continue;
						} else {
							all_included = false;
						}
						//std::cout<<"af incl"<<std::endl;
						if (!min_w.is_inf[temp_connected_node_ind]) {
							if (min_w.min_weight[temp_connected_node_ind] > f_depth) { // Candidate node is more optimal than previous found node
								min_w.min_weight[temp_connected_node_ind] = f_depth;
							} else if (min_w.min_weight[temp_connected_node_ind] < f_depth) { // Candidate node is less optimal than previous found node (don't care about it)
								continue;
							}
						} else {
							min_w.is_inf[temp_connected_node_ind] = false;
							min_w.min_weight[temp_connected_node_ind] = f_depth;
							spw.reachability[temp_connected_node_ind] = true; // mark the new new as reachable
							//unique_dfa_parent_set.push_back(temp_connected_node_ind); // add the prod ind to the set of nodes to add
						}
						spw.state_weights[temp_connected_node_ind] = cFunc(depth); // Only a fcn of depth
						//second_search_weights[unique_con_node] = depth;
						std::pair<int, int> new_leaf = {temp_connected_node_ind, depth};
						pq.push(new_leaf); // add to prio queue
						//tree.connect(src, {temp_connected_node_ind, &second_search_weights[temp_connected_node_ind]});
						tree.connect(curr_leaf_ind, {temp_connected_node_ind, &min_w.min_weight[temp_connected_node_ind]});
						included[temp_connected_node_ind] = true;
					}
						//std::cout<<"af for loop"<<std::endl;
					if (all_included) {
						continue;
					}
				}
				
				// CHECK IF CANDIDATE NODE IS ACCEPTING:
				if (round == 0) {
					if (dfa_sps->isCurrAccepting()) {
						//std::cout<<"found accepting node: "<<std::endl;
						//std::cout<<" ts node: "<<TS_sps->getCurrNode()<<std::endl;
						//std::cout<<" ts node: "<<dfa_sps->getCurrNode()<<std::endl;
						accepting.first = true;
						//std::cout<<"Pushing node to accepting set: "<<connected_node_ind<<std::endl;
						accepting.second.push_back(connected_node_ind);
					}
				}
			}
			prev_leaf_ind = curr_leaf_ind;
		}
		std::cout<<"made it out"<<std::endl;
		//if ((round == 0) && !found_target_state) {
		//	std::cout<<"Error: Target state not found\n";
		//	exit_failure = true; // currently not using this
		//	return false;
		//}
		if (round == 1) {
			for (int i=0; i<spw.reachability.size(); ++i) {
				if (!spw.reachability[i]) {
					std::cout<<"Warning: Not all states were searched (round 2). Failed prod state ind: "<<i<<std::endl;
					std::vector<int> temp_inds;
					Graph<float>::augmentedStatePreImage(graph_sizes, i, temp_inds);
					std::cout<<"   TS ind: "<<temp_inds[0]<<std::endl;
					std::cout<<"   DFA ind: "<<temp_inds[1]<<std::endl;
				}
			}
		}
	}
	if (!exit_failure) {
		//extractPath(parents, solution_ind);
		//int p_space_size = 1;
		//p_space_size *=
		//for 

		//end_time = std::chrono::system_clock::now();
		//double search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		//std::cout<<"Search time (milliseconds): "<<search_time<<std::endl;
		//std::cout<<"Finished. Tree size: "<<tree.size()<<std::endl; //<<", Maximum product graph (no pruning) size: "<<
		//std::cout<<"\n\n ...Finished with plan."<<std::endl;
		return true;
	} else {
		std::cout<<"Failed space search."<<std::endl;
		return false;
	}
}

template<class T>
bool SymbSearch<T>::generateRiskStrategy(DFA_EVAL* cosafe_dfa, DFA_EVAL* live_dfa, std::function<float(unsigned int)> cFunc, Strategy& strat, bool use_cost) {
	if (!TS->isReversible() || !cosafe_dfa->getDFA()->isReversible() || !live_dfa->getDFA()->isReversible()) {
		std::cout<<"Error: Cannot perform space search on irreversible graphs\n";
		return false;
	}
	int n = TS->size();
	int m = cosafe_dfa->getDFA()->size();
	int l = live_dfa->getDFA()->size();
	int p_space_size = n * m * l;
	std::vector<int> graph_sizes = {n, m, l};

	std::vector<const std::vector<std::string>*> total_alphabet(2);
	total_alphabet[0] = cosafe_dfa->getAlphabetEVAL();
	total_alphabet[1] = live_dfa->getAlphabetEVAL();
	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet

	spaceWeight spw_rsk;
	bool rsk_success = riskSearch(TS, cosafe_dfa, spw_rsk, cFunc);
	if (!rsk_success) {
		std::cout<<"Error: Risk state weighting failed\n";
		return false;
	}
	std::cout<<"spw_rsk size: "<<spw_rsk.reachability.size()<<std::endl;
	for (int i=0; i<spw_rsk.reachability.size(); ++i) {
		if (spw_rsk.reachability[i]) {
			std::vector<int> ret_inds;
			Graph<float>::augmentedStatePreImage({n, m}, i, ret_inds);
			if (spw_rsk.is_inf[i]) {
				std::cout<<"spw_rsk: P:"<<i<<" TS State: "<<ret_inds[0]<<" (cosafe: "<<ret_inds[1]<<") is reachable with weight: INF"<<std::endl;
			} else {
				std::cout<<"spw_rsk: P:"<<i<<" TS State: "<<ret_inds[0]<<"(cosafe: "<<ret_inds[1]<<") is reachable with weight: "<<spw_rsk.state_weights[i]<<std::endl;
			}
		}
	}

	//std::vector<float> first_search_weights(p_space_size, -1.0f); 
	//std::vector<float> second_search_weights(p_space_size, -1);
	//std::cout<<" P SPACE SIZE: "<<p_space_size<<std::endl;
	strat.action_map.resize(p_space_size);
	strat.reachability.resize(p_space_size, false);
	
	minWeight min_w(p_space_size);
	
	// Search: 

	//std::chrono::time_point<std::chrono::system_clock> end_time;
	//std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
	
	// Priority queue lambda
	auto compare = [](std::pair<int, int> pair1, std::pair<int, int> pair2) {
		if (pair1.second == -1.0) {
			return true; // pair1 is infinite
		} else if (pair2.second == -1.0) {
			return false;
		} else {
			return pair1.second > pair2.second;
		}
	};

	bool exit_failure = false;
	std::pair<bool, std::vector<int>> accepting;
	for (int round=0; round<2; ++round) {
		std::cout<<"\n generateRiskStrategy: STARTING ROUND: "<<round<<std::endl;
		std::string search_type = (round == 0) ? "forward" : "reverse";
		std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float>>, decltype(compare)> pq(compare);
		//pq.clear();

		// Tree to keep history as well as parent node list:
		bool found_target_state = false;
		std::vector<bool> included(p_space_size, false);
		Graph<float> tree; 

		// Root tree node has index zero with weight zero
		std::pair<int, float> curr_leaf;
		if (round == 0) {
			//std::cout<<"b4 reset"<<std::endl;
			TS->reset();
			cosafe_dfa->reset();
			live_dfa->reset();
			//std::cout<<" \n\n WE RESET: "<<live_dfa->getCurrNode()<<std::endl;
			int init_node_ind = Graph<float>::augmentedStateImage({TS->getCurrNode(), cosafe_dfa->getCurrNode(), live_dfa->getCurrNode()}, graph_sizes);
			std::cout<<"ROUND 0 INIT NODE IND: "<<init_node_ind<<std::endl;
			//first_search_weights[init_node_ind] = -1.0f;
			min_w.is_inf[init_node_ind] = false;
			min_w.min_weight[init_node_ind] = 0;
			included[init_node_ind] = true;
			curr_leaf.first = init_node_ind;
			curr_leaf.second = 0.0f;
			pq.push(curr_leaf);
		} else {
			// Add the accepting states to the prio queue with weight zero, and add them to the tree so
			int ROOT_STATE = n*m*l; // this is the root state ind, guaranteed to be larger than any prod state ind
			min_w.reset();
			std::cout<<"num acc states rnd 2: "<<accepting.second.size()<<std::endl;
			for (auto acc_prod_state : accepting.second) {
				//std::cout<<"in the accepting states... acc prod state: "<<acc_prod_state<<std::endl;
				min_w.is_inf[acc_prod_state] = false;
				min_w.min_weight[acc_prod_state] = 0;
				included[acc_prod_state] = true;
				//spw.state_weights[acc_prod_state] = 0.0f;
				strat.reachability[acc_prod_state] = true;
				strat.action_map[acc_prod_state] = "ACCEPTING";
				//std::vector<int> sol_inds;
				//Graph<float>::augmentedStateMap(acc_prod_state, n, m, sol_inds);
				curr_leaf.first = acc_prod_state;
				curr_leaf.second = 0.0f;
				pq.push(curr_leaf);
				tree.connect(ROOT_STATE, {acc_prod_state, nullptr}); // the root state is the merged accepting state
			}
			//std::cout<<"out of the accepting states"<<std::endl;
		}

		float min_accepting_cost = -1;
		int prev_leaf_ind = -1;
		int prod_solution_ind;
		while (pq.size() > 0) {
			//int pause;
			//std::cin >> pause;
			curr_leaf = pq.top();
			//std::cout<<" TOP --- Ind: "<<pq.top().first<<std::endl;
			//std::cout<<" TOP --- LexSet: ";
			//pq.top().second->print();

			pq.pop();
			int curr_leaf_ind = curr_leaf.first;
			float curr_leaf_weight = curr_leaf.second;
			if (!min_w.is_inf[curr_leaf_ind]) {
				if (curr_leaf_weight > min_w.min_weight[curr_leaf_ind]) {
					continue;
				}
			}
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
			TS->set(ret_inds[0]);
			cosafe_dfa->set(ret_inds[1]);
			live_dfa->set(ret_inds[2]);

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
				TS->getConnectedDataEVAL(con_data);
				TS->getConnectedNodesEVAL(con_nodes);
			} else {
				TS->getParentDataEVAL(con_data);
				TS->getParentNodesEVAL(con_nodes);
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
			//std::pair<int, float*> src;
			//src.first = curr_leaf_ind;
			//src.second = nullptr;
			//for (auto con_data_ptr : con_data) {
			for (int j=0; j<con_data.size(); ++j) {
				// Reset after checking connected nodes
				TS->set(ret_inds[0]);
				cosafe_dfa->set(ret_inds[1]);
				live_dfa->set(ret_inds[2]);
				//TS->set(ret_inds.first);
				//dfa_sps->set(ret_inds.second);

				//for (int i=0; i<num_dfas; ++i) {
				//	//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
				//	dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
				//}
				//printQueue(pq);
				//std::string temp_str = con_data_ptr->label;
				//float temp_weight = con_data_ptr->weight;
				std::string temp_str = con_data[j]->label;
				float temp_weight = con_data[j]->weight;
				//float temp_weight = cFunc(curr_leaf_depth + temp_depth);
				//std::cout<<"temp label: "<<temp_str<<std::endl;
				//std::cout<<"temp weight: "<<temp_weight<<std::endl;
				bool found_connection = true;
				int prev_ts_ind;

				if (round == 0) {
					//std::cout<<"forward ts eval"<<std::endl;
					//std::cout<<"ACTION: "<<temp_str<<std::endl;
					//std::cout<<"pre ts ind: "<<TS->getCurrNode()<<std::endl;
					found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
					//std::cout<<"post ts ind: "<<TS->getCurrNode()<<std::endl;
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
					//std::cout<<"ts curr node: "<<TS->getCurrNode()<<std::endl;
					lbls = TS->returnStateLabels(TS->getCurrNode());
					//std::cout<<"lbls size: "<<lbls->size()<<std::endl;
				} else {
					//std::cout<<"\nts node b4 return state labels: "<<prev_ts_ind<<std::endl;
					lbls = TS->returnStateLabels(prev_ts_ind);
				}
				found_connection = false;
				//std::cout<<"\n before eval"<<std::endl;
				bool found_true = true;
				std::vector<int> parent_node_list_cosafe;
				std::vector<int> parent_node_list_live;
				if (round == 0) {
					for (int ii=0; ii<lbls->size(); ++ii) {
						//std::cout<<"pre cosafe ind: "<<cosafe_dfa->getCurrNode()<<std::endl;
						if (cosafe_dfa->eval(lbls->operator[](ii), true)) {
							found_connection = true;
							//std::cout<<"post cosafe ind: "<<cosafe_dfa->getCurrNode()<<std::endl;
							break;
						}
					}
					if (!found_connection) {
						std::cout<<" DID NOT FIND CONNECTION IN COSAFE DFA "<<std::endl;
					}
					for (int ii=0; ii<lbls->size(); ++ii) {
						//std::cout<<" testing lbl: "<<lbls->operator[](ii)<<std::endl;
						if (live_dfa->eval(lbls->operator[](ii), true)) {
							found_connection = found_connection && true;
							break;
						}
					}
					if (!found_connection) {
						std::cout<<" DID NOT FIND CONNECTION IN LIVE DFA "<<std::endl;
					}
				} else {
					found_connection = cosafe_dfa->getParentNodesWithLabels(lbls, parent_node_list_cosafe);
					//if (!found_connection) {
					//	std::cout<<"Error: Did not find parent connection for cosafe DFA\n";
					//}
					//std::cout<<"\n HERE"<<std::endl;
					found_connection = live_dfa->getParentNodesWithLabels(lbls, parent_node_list_live);
					//if (!found_connection) {
					//	std::cout<<"Error: Did not find parent connection for liveness DFA\n";
					//}
					//std::cout<<"found connection? : "<<found_connection<<" parent_node_list size: "<<parent_node_list.size()<<std::endl;
				}

				bool unique = true;
				int connected_node_ind = -1;
				std::vector<int> unique_dfa_parent_set;
				float weight;
				bool all_included = true;
				if (round == 0) {
					weight = curr_leaf_weight + temp_weight;
					//connected_node_ind = Graph<float>::augmentedStateFunc(TS_sps->getCurrNode(), dfa_sps->getCurrNode(), n, m);
					connected_node_ind = Graph<float>::augmentedStateImage({TS->getCurrNode(), cosafe_dfa->getCurrNode(), live_dfa->getCurrNode()}, graph_sizes);
					if (included[connected_node_ind]) {
						//std::cout<<"included??? con node ind: "<<connected_node_ind<<std::endl;
						continue;
					} 
					if (!min_w.is_inf[connected_node_ind]) {
						if (min_w.min_weight[connected_node_ind] > weight) {
							min_w.min_weight[connected_node_ind] = weight;
						} else if (min_w.min_weight[connected_node_ind] < weight) {
							continue;
						} 
					}else {
						min_w.is_inf[connected_node_ind] = false;
						min_w.min_weight[connected_node_ind] = weight;
					}
					//first_search_weights[connected_node_ind] = curr_leaf_weight + temp_weight;
					std::pair<int, float> new_leaf = {connected_node_ind, weight};
					pq.push(new_leaf); // add to prio queue
					tree.connect(curr_leaf_ind, {connected_node_ind, &min_w.min_weight[connected_node_ind]});
					included[connected_node_ind] = true;
				} else {
					
					int temp_connected_node_ind = -1;
					for (auto par_node_cosafe : parent_node_list_cosafe) {
						for (auto par_node_live : parent_node_list_live) {
							// Convert from triple product to double product:
							int spw_ind = Graph<float>::augmentedStateImage({TS->getCurrNode(), par_node_cosafe}, {n, m});
							if (spw_rsk.is_inf[spw_ind]) {
								weight = -1.0; // Infinity
							} else {
								float spw_cost = spw_rsk.state_weights[spw_ind];
								if (use_cost) {
									float model_cost = curr_leaf_weight + temp_weight;
									weight = spw_cost + model_cost;
								} else {
									weight = spw_cost;
								}
							}
								//temp_connected_node_ind = Graph<float>::augmentedStateFunc(TS_sps->getCurrNode(), par_node, n, m);
							temp_connected_node_ind = Graph<float>::augmentedStateImage({TS->getCurrNode(), par_node_cosafe, par_node_live}, graph_sizes);
							if (included[temp_connected_node_ind]) {
								continue;
							} else {
								all_included = false;
							}
							if (!min_w.is_inf[temp_connected_node_ind]) {
								if (min_w.min_weight[temp_connected_node_ind] > weight) {
									min_w.min_weight[temp_connected_node_ind] = weight;
								} else if (min_w.min_weight[temp_connected_node_ind] < weight) {
									continue;
								}
							} else {
								min_w.is_inf[temp_connected_node_ind] = false;
								min_w.min_weight[temp_connected_node_ind] = weight;
								strat.reachability[temp_connected_node_ind] = true; // mark the new new as reachable
							}
							//second_search_weights[unique_con_node] = cost;
							std::pair<int, float> new_leaf = {temp_connected_node_ind, weight};
							pq.push(new_leaf); // add to prio queue
							tree.connect(curr_leaf_ind, {temp_connected_node_ind, &min_w.min_weight[temp_connected_node_ind]});
							strat.action_map[temp_connected_node_ind] = temp_str; // Set the optimal action
							included[temp_connected_node_ind] = true; // prevent node from being searched again in loop
						}
					}
				}
				if (round == 0) {
					std::cout<<"curr live node: "<<live_dfa->getCurrNode()<<std::endl;
					if (live_dfa->isCurrAccepting()) {
						accepting.first = true;
						accepting.second.push_back(connected_node_ind);
					}
				}
			}
			prev_leaf_ind = curr_leaf_ind;
		}
		std::cout<<"made it out"<<std::endl;
		//if ((round == 0) && !found_target_state) {
		//	std::cout<<"Error: Target state not found\n";
		//	exit_failure = true; // currently not using this
		//	return false;
		//}
		if (round == 1) {
			for (int i=0; i<strat.reachability.size(); ++i) {
				if (!strat.reachability[i]) {
					std::cout<<"Warning: Not all states were searched (round 2). Failed prod state ind: "<<i<<std::endl;
					std::vector<int> ret_inds;
					Graph<float>::augmentedStatePreImage(graph_sizes, i, ret_inds);

					std::cout<<"   TS ind: "<<ret_inds[0]<<std::endl;
					std::cout<<"   Cosafe DFA ind: "<<ret_inds[1]<<std::endl;
					std::cout<<"   Live DFA ind: "<<ret_inds[2]<<std::endl;
				}
			}
		}
	}
	if (!exit_failure) {
		//extractPath(parents, solution_ind);
		//int p_space_size = 1;
		//p_space_size *=
		//for 

		//end_time = std::chrono::system_clock::now();
		//double search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		//std::cout<<"Search time (milliseconds): "<<search_time<<std::endl;
		//std::cout<<"Finished. Tree size: "<<tree.size()<<std::endl; //<<", Maximum product graph (no pruning) size: "<<
		//std::cout<<"\n\n ...Finished with plan."<<std::endl;
		return true;
	} else {
		std::cout<<"Failed space search."<<std::endl;
		return false;
	}
}



template<class T>
bool SymbSearch<T>::generateHeuristic() {
	heuristic.resize(num_dfas);
	for (int i=0; i<num_dfas; ++i) {
		heuristic[i].dfa_ind = i;
		std::vector<DFA_EVAL*> dfa_list_single = {dfa_list_ordered->operator[](i)};
		bool success = spaceSearch(TS, &dfa_list_single, heuristic[i]);
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
	//std::cout<<"hello p_node_ind: "<<p_node_ind<<std::endl;
	//std::cout<<"reachability size: "<<heuristic[dfa_list_ind].reachability[p_node_ind] <<std::endl;
	if (heuristic[dfa_list_ind].reachability[p_node_ind]) {
		reachable = true;
		return heuristic[dfa_list_ind].state_weights[p_node_ind];
	} else {
		reachable = false;
		return -1;
	}
}

template<class T>
bool SymbSearch<T>::search(bool use_heuristic) {
	auto compareLEX = [](const std::pair<int, T*>& pair1, const std::pair<int, T*>& pair2) {
		return *(pair1.second) > *(pair2.second);
	};
	auto ___ = [](const T&) {return true;};

	T prune_bound(mu, num_dfas);
	prune_bound = BFS(compareLEX, ___, false, false, use_heuristic); // No pruning criteria, no need for path
	std::cout<<"\nPRINTING PRUNE BOUND: "<<std::endl;
	prune_bound.print();
	std::cout<<"\n";
	//int in;
	//std::cin>>in;
	auto pruneCriterionMAX = [&prune_bound](const T& arg_set) {

		//std::cout<<"   in prune criterion..."<<std::endl;
		//std::cout<<"   arg set:"<<std::endl;
		//arg_set.print();
		//std::cout<<"   prune bound set:"<<std::endl;
		//prune_bound.print();
		//int in;
		//std::cin>>in;

		return (!prune_bound.withinBounds(arg_set));
	};
	auto compareMAX = [](const std::pair<int, T*>& pair1, const std::pair<int, T*>& pair2) {
		return pair1.second->getMaxVal() > pair2.second->getMaxVal();
	};

	T solution_cost(mu, num_dfas);
	solution_cost = BFS(compareMAX, pruneCriterionMAX, true, true, use_heuristic); 
	std::cout<<"\nPRINTING SOLUTION COST: "<<std::endl;
	solution_cost.print();
	std::cout<<"\n";
	
}

template<class T>
T SymbSearch<T>::BFS(std::function<bool(const std::pair<int, T*>&, const std::pair<int, T*>&)> compare, std::function<bool(const T&)> pruneCriterion, bool prune, bool extract_path, bool use_heuristic) {	
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
	
	// ATTENTION: "pruned" is indexed in the product space
	std::vector<bool> pruned(p_space_size, false);
	
	// ATTENTION: "parents" is indexed by the tree size for memory space efficiency
	std::vector<int> parents(p_space_size, -1);

	// ATTENTION: Members of min_w are indexed in the product space
	minLS min_w(p_space_size);
	std::cout<<"PSPACE SIZE: "<<p_space_size<<std::endl;


	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet

	T min_accepting_cost(mu, num_dfas); // Declare here so we can have a return value

	spaceWeight spw;
	//std::cout<<"b4 space search"<<std::endl;
	if (use_heuristic) {
		bool h_success = generateHeuristic();
		if (!h_success) {
			return min_accepting_cost;
		}
	}
	//spaceSearch(TS, dfa_list_ordered->operator[](0), spw);
	//std::cout<<"af space search"<<std::endl;
	
	//for (int i=0; i<spw.state_weights.size(); ++i) {
	//	int n = TS->size();
	//	int m = dfa_list_ordered->operator[](0)->getDFA()->size();
	//	std::cout<<"Prod state ind: "<<i<<std::endl;
	//	std::vector<int> temp_inds;
	//	Graph<float>::augmentedStatePreImage({n, m}, i, temp_inds);
	//	std::cout<<"TS state ind: "<<temp_inds.first<<std::endl;
	//	std::cout<<"  Weight: "<<spw.state_weights[i]<<std::endl;
	//}
	
	//T prev_sol_weight(mu, num_dfas);
	//prev_sol_weight.setInf();
	bool sol_found = true;
	//int dfs_iterations = 0;
	bool finished;
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
	temp_nodeptr->lex_set.print();
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
	finished = false;
	sol_found = false;
	//std::cout<<"entering the loop"<<std::endl;
	while (!finished) {
		if (pq.empty()) {
			finished = true;
			continue;
		}
		iterations++;
		//std::cout<<"iter: "<<iterations<<std::endl;
		pq.top();
		//std::cout<<"\nprior queue: "<<std::endl;
		//printQueue(pq);

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
		T* curr_leaf_weight = curr_leaf.second;
		//int con_node_prod_ind = Graph<float>::augmentedStateImage(node_inds, graph_sizes);
		visited[curr_leaf_prod_ind] = true;
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
		if (!init) { // Use temp_nodeptr from outside the loop (init) when no nodes are in tree
			temp_nodeptr = tree.getNodeDataptr(curr_leaf_ind);
		} else {
			init = false;
		}
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
		//std::cout<<"CURRENT TS NODE: "<<TS->getCurrNode()<<std::endl;
		//std::cout<<"b4 get con"<<std::endl;
		con_data.clear();
		TS->getConnectedDataEVAL(con_data);
		std::vector<int> con_nodes;
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
			//std::cout<<"  in loop: curr_leaf_ind: "<<curr_leaf_ind<<" ts set state: "<<node_list[curr_leaf_ind]->i<<std::endl;
			TS->set(node_list[curr_leaf_ind]->i);
			for (int i=0; i<num_dfas; ++i) {
				//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
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
			for (int i=0; i<num_dfas + 1; ++i) {
				if (i == 0) { // eval TS first!!! (so that getCurrNode spits out the adjacent node, not the current node)
					//std::cout<<"current ts node: "<<TS->getCurrNode()<<std::endl;
					found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
					//std::cout<<"ts evolved state: "<<TS->getCurrNode()<<" under action: "<<temp_str<<std::endl;
					if (!found_connection) {
						std::cout<<"Error: Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
						return min_accepting_cost;
					}
				} else { // eval DFAi
					// There could be multiple logical statements the correspond to the same
					// underlying label, therefore we must test all of the statements until
					// one is found. Only one is needed due to determinism.
					const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
					found_connection = false;
					//std::cout<<"\n before eval"<<std::endl;
					for (int ii=0; ii<lbls->size(); ++ii) {
						//std::cout<<"       labels: --- "<<lbls->operator[](ii)<<std::endl;
						//std::cout<<"DFA::: "<<(i-1)<<" curr node; "<<(dfa_list_ordered->operator[](i-1)->getCurrNode())<<std::endl;
						if (dfa_list_ordered->operator[](i-1)->eval(lbls->operator[](ii), true)) {
							found_connection = true;
							break;
						}
					}
				}
				if (!found_connection) {
					std::cout<<"Error: Connectivity was not found for either the TS or a DFA. \n";
					return min_accepting_cost;
				}
			}
			//std::cout<<"af eval"<<std::endl;
			// Check if the node has already been seen:
			//std::cout<<"\n Uniqueness check: \n"<<std::endl;
			//std::cout<<"printing nodelist: "<<std::endl;

			//std::cout<<"b4 visited"<<std::endl;
			std::vector<int> node_inds(num_dfas + 1);
			node_inds[0] = TS->getCurrNode();
			for (int i=0; i<num_dfas; ++i) {
				node_inds[i+1] = dfa_list_ordered->operator[](i)->getCurrNode();
			}
			int con_node_prod_ind = Graph<float>::augmentedStateImage(node_inds, graph_sizes);
			if (visited[con_node_prod_ind] || pruned[con_node_prod_ind]) { // Node was visited
				continue;
			}
			//std::cout<<"af visited"<<std::endl;

			// Made it through connectivity check, therefore we can append the state to the tree:
			// GET:
			bool all_accepting = true;
			//std::cout<<"----printing temp lex set fill: ";
			for (int i=0; i<num_dfas; ++i) {

				// If the dfa is accepting at the evolved ind, append no cost, otherwise append
				// the cost of taking the action:
				//std::cout<<"-DFA: "<<i<<", curr becore accepting: "<<dfa_list_ordered->operator[](i)->getCurrNode()<<std::endl;
				//std::cout<<" ->DFA: ";
				if (dfa_list_ordered->operator[](i)->isCurrAccepting()) {
					//std::cout<<i<<" is accepting";
					temp_lex_set_fill[i] = 0;
				} else {
					all_accepting = false;
					temp_lex_set_fill[i] = temp_weight;
				}
				//std::cout<<temp_lex_set_fill[i]<<" ";
			}
			//std::cout<<"\n";

			if (prune) {
				T temp_prune_check(mu, num_dfas); 
				temp_prune_check = *(curr_leaf_weight);
				temp_prune_check += temp_lex_set_fill;
				if (pruneCriterion(temp_prune_check)) {
					min_w.is_inf[con_node_prod_ind] = false; // mark node as seen
					visited[con_node_prod_ind] = true;
					pruned[con_node_prod_ind] = true;
					continue;
				}
			}
			//std::cout<<"b4 update"<<std::endl;
			if (!min_w.is_inf[con_node_prod_ind]) { // Node was seen before, non inf weight, but not visited
				int seen_node_ind = min_w.prod2node_list.at(con_node_prod_ind); // This value will be mapped if weve seen the node before
				IVFlexLex<T>* seen_node = node_list[seen_node_ind];
				T temp_lex_set(mu, &temp_lex_set_fill, num_dfas);
				temp_lex_set += *curr_leaf_weight;
				//std::cout<<"printing seen node lex set: "<<std::endl;
				//temp_node->lex_set.print();
				//std::cout<<"printing temp lex set: "<<std::endl;
				//temp_lex_set.print();
				if (seen_node->lex_set > temp_lex_set) {
					//std::cout<<"UPDATING NODE: "<<seen_node_ind<<" to: "<<curr_leaf_ind<<std::endl;
					seen_node->lex_set = temp_lex_set;
					// UPDATE PARENT HERE vvvvvv
					parents[seen_node_ind] = curr_leaf_ind;
				}
				continue;
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

			new_temp_nodeptr->lex_set = *(curr_leaf_weight);
			new_temp_nodeptr->lex_set += temp_lex_set_fill;

			//std::cout<<"printing add node set: \n";
			//new_temp_nodeptr->lex_set.print();

			// BUILD CONNECTION:
			//tree_end_node++;
			//tree_end_node = con_node_prod_ind;

			// The priority queue includes the heuristic cost, however the tree does not:
			std::pair<int, T*> new_leaf;
			//T temp_weight_set(mu, num_dfas);
			T* temp_weight_set_ptr;
			//if (ITERATE) {
			//	temp_weight_set_ptr = new T(mu, num_dfas);
			//	path_length_map_ptrs.push_back(temp_weight_set_ptr);
			//}
			if (use_heuristic) {
				std::vector<float> h_vals(num_dfas);
				for (int i=0; i<num_dfas; ++i) {
					bool reachable;
					if (dfa_list_ordered->operator[](i)->isCurrAccepting()) {
						h_vals[i] = 0;
					} else {
						//std::cout<<"dfa curr node: "<<dfa_list_ordered->operator[](i)->getCurrNode()<<" is it accepting? "<<dfa_list_ordered->operator[](i)->isCurrAccepting()<<std::endl;
						h_vals[i] = pullStateWeight(TS->getCurrNode(), dfa_list_ordered->operator[](i)->getCurrNode(), i, reachable);
						if (!reachable) {
							std::cout<<"Error: Attempted to find heuristic distance for unreachable product state (ts: "<<TS->getCurrNode()<<" dfa "<<i<<": "<<dfa_list_ordered->operator[](i)->getCurrNode()<<") \n";
							return min_accepting_cost;
						}
					}
				}
				T* new_temp_setptr = newSet();
				//if (!ITERATE) { // ITERATE determine whether or not to use A* or DFS
				//new_temp_setptr->operator=(*curr_leaf_weight);
				new_temp_setptr->operator=(new_temp_nodeptr->lex_set);
				//new_temp_setptr->operator+=(fill_set);
				new_temp_setptr->addHeuristic(h_vals);
				//} else {
				//	temp_weight_set_ptr->operator=(new_temp_nodeptr->lex_set);
				//	new_temp_setptr->operator=(fill_set);
				//}
				new_leaf = {con_node_prod_ind, new_temp_setptr};
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
			pq.push(new_leaf); // add to prio queue
			tree.connect(curr_leaf_ind, {con_node_ind, new_temp_nodeptr});
			min_w.is_inf[con_node_prod_ind] = false; // mark node as seen
			parents[con_node_ind] = curr_leaf_ind;
			//std::cout<<"af connect"<<std::endl;
			//if (ITERATE) {
			//	path_length_map[tree_end_node] = temp_weight_set_ptr;
			//}
			if (all_accepting) {
				accepting.first = true;
				//std::cout<<">>>found accepting state: "<<con_node_prod_ind<<std::endl;
				//std::vector<int> ret_inds;
				//Graph<float>::augmentedStatePreImage(graph_sizes, con_node_prod_ind, ret_inds);
				//std::cout<<"   acc TS: "<<ret_inds[0]<<std::endl;
				//for (int i=0; i<ret_inds.size() -1; ++i) {
				//	std::cout<<"   acc dfa "<<i<<": "<<ret_inds[i+1]<<std::endl;
				//}
				accepting.second.push_back(con_node_prod_ind);
				solution_ind = con_node_ind;
				//std::cout<<"min accepting cost: "<<std::endl;
				//min_accepting_cost.print();
				//std::cout<<"found cost: "<<std::endl;
				//new_leaf.second->print();
				if (*(new_leaf.second) < min_accepting_cost) {
					min_accepting_cost = *(new_leaf.second);
				}
			}
			if (accepting.first) {
				if (*(pq.top().second) >= min_accepting_cost) {
					finished = true;
					//std::cout<<"Found a solution!"<<"\n";
					//std::cout<<"Printing MIN ACCEPTING COST: "<<std::endl;
					//min_accepting_cost.print();
					//std::cout<<"   -Iterations: "<<iterations<<"\n";
					sol_found = true;
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
	//std::cout<<"out of loop sol_found: "<<sol_found<<std::endl;
	//	if (!ITERATE) {
	if (finished) {
		if (extract_path) {
			extractPath(parents, solution_ind);
			int p_space_size = 1;
			//p_space_size *=
			//for 

			//end_time = std::chrono::system_clock::now();
			//double search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
			//std::cout<<"Search time (milliseconds): "<<search_time<<std::endl;
			std::cout<<p_space_size<<std::endl;
			std::cout<<"Finished. Tree size: "<<tree.size()<<" Iterations: "<<iterations<<" (P space size: "<<p_space_size<<")\n";//<<", Maximum product graph (no pruning) size: "<<
			//std::cout<<"\n\n ...Finished with plan."<<std::endl;
		}
		return min_accepting_cost;
	} else {
		std::cout<<"Failed (no plan)."<<std::endl;
		return min_accepting_cost;
	}
	//	} else {
	//		std::cout<<"in da check"<<std::endl;
	//		if (!finished) {
	//			std::cout<<"fixed point: "<<dfs_iterations<<" iterations!\n";
	//			break;
	//		}
	//	}
	//}
	//if (ITERATE) {
	//	std::cout<<"Made it out of dfs in: "<<dfs_iterations<<" iterations!\n";
	//}
	//return true;

}

template<class T>
void SymbSearch<T>::extractPath(const std::vector<int>& parents, int accepting_state) {
	int curr_node = accepting_state;
	std::vector<int> reverse_TS_state_sequence;
	//std::cout<<" given accepting state: "<<accepting_state<<std::endl;
	//reverse_TS_state_sequence.push_back(node_list[accepting_state);
	//std::cout<<"reverse plan: "<<std::endl;
	//for (int i=0; i<node_list.size(); ++i) {
	//	std::cout<<"node list ind: "<<i<<" ts ind: "<<node_list[i]->i<<std::endl;
	//	node_list[i]->lex_set.print();
	//}
	while (curr_node != 0) {
		//std::cout<<"node list node: "<<curr_node<<", ";
		//std::cout<<"ts node: "<<node_list[curr_node]->i<<", ";
		//std::cout<<"lexset: ";
		//node_list[curr_node]->lex_set.print();
		reverse_TS_state_sequence.push_back(node_list[curr_node]->i);
		curr_node = parents[curr_node];
	}
	reverse_TS_state_sequence.push_back(node_list[0]->i); // finally add the init state
	std::cout<<"Info: Successfully extracted plan!\n";
	TS_state_sequence.resize(reverse_TS_state_sequence.size());
	TS_action_sequence.resize(reverse_TS_state_sequence.size()-1);
	std::cout<<"State sequence: ";
	//std::cout<<"first state: "<<reverse_TS_state_sequence[0]<<std::endl;
	for (int i=0; i<reverse_TS_state_sequence.size(); ++i) {
		//std::cout<<"i: "<<i<<" size(): "<<TS_state_sequence.size()<<std::endl;
		TS_state_sequence[i] = reverse_TS_state_sequence[reverse_TS_state_sequence.size()-1-i];
		std::cout<<" -> "<<TS_state_sequence[i];
	}
	//std::cout<<"hey"<<std::endl;
	std::cout<<"\n ...";
	//std::cout<<"heyHEY"<<std::endl;

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
	std::cout<<"Info: Number of actions: "<<TS_action_sequence.size()<<", pathlength: "<<pathlength<<"\n";
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


