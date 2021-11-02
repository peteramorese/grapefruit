#include "symbSearch.h"
#include<fstream>
#include<chrono>
#include<ctime>

template<class T>
SymbSearch<T>::SymbSearch() {}

template<class T>
void SymbSearch<T>::setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_) {
	dfa_list_ordered = dfa_list_ordered_;
	node_size = dfa_list_ordered->size();
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
	IVFlexLex<T>* node_i = new IVFlexLex<T>(mu, node_size);
	//node_i->v.resize(node_size);
	node_list.push_back(node_i);
	return node_i;
}

//template<class T>
//IVFlexLex<T>* SymbSearch<T>::newNode_ss() {
//	IVFlexLex<T>* node_i = new IVFlexLex<T>(mu, node_size);
//	//node_i->v.resize(node_size);
//	node_list.push_back(node_i);
//	return node_i;
//}

template<class T>
template<typename Q>
void SymbSearch<T>::printQueue(Q queue) {
	if (queue.top().first == 99){
		std::cout<<" HELLO WE LOOK AT 99----------------------"<<std::endl;
	while (!queue.empty()) {
		std::cout<<"Ind: "<<queue.top().first<<std::endl;
		std::cout<<"LexSet: ";
		queue.top().second->print();
		queue.pop();
	}
	}
}

template<class T>
bool SymbSearch<T>::spaceSearch(TS_EVAL<State>* TS_sps, DFA_EVAL* dfa_sps, spaceWeight& spw) {
	if (!TS_sps->isReversible() || !dfa_sps->getDFA()->isReversible()) {
		std::cout<<"Error: Cannot perform space search on irreversible graphs\n";
	}
	int n = TS_sps->size();
	int m = dfa_sps->getDFA()->size();
	int p_space_size = n * m;

	// These indices are determined after the first search round:
	int TS_accepting_state = -1;
	int best_dfa_accepting_state = -1;

	std::vector<float> first_search_weights(p_space_size, -1.0);
	//std::vector<float> second_search_weights(p_space_size, -1.0);
	spw.state_weights.resize(p_space_size);
	
	// Search: 

	//std::chrono::time_point<std::chrono::system_clock> end_time;
	//std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
	std::vector<const std::vector<std::string>*> total_alphabet(1);
	for (int i=0; i<node_size; ++i) {
		total_alphabet[i] = dfa_sps->getAlphabetEVAL();
	}
	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet
	auto compare = [](std::pair<int, float> pair1, std::pair<int, float> pair2) {
		return pair1.second > pair2.second;
	};

	for (int round=0; round<2; ++round) {
		std::string search_type = (round == 0) ? "forward" : "reverse";
		std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float>>, decltype(compare)> pq(compare);
		pq.clear();

		// Tree to keep history as well as parent node list:
		bool found_target_state = false;
		std::vector<bool> included(p_space_size, false);
		Graph<float> tree;
		//std::vector<int> parents;


		// Fill the root tree node (init node):
		//WIV<T>* temp_nodeptr = newNode();
		//parents.push_back(0);
		//std::vector<float> temp_lex_set_fill(node_size);
		//temp_nodeptr->i = 0;
		//for (int i=0; i<node_size; ++i) {
		//	// dfa eval inits to currnode = init node
		//	temp_nodeptr->v[i] = dfa_sps->getCurrNode();
		//	// Weights are zero for the root node:
		//	temp_lex_set_fill[i] = 0;
		//}
		//temp_nodeptr->lex_set = temp_lex_set_fill;

		// Root tree node has index zero with weight zero
		if (round == 0) {
			TS_sps->reset();
			dfa_sps->reset();
		} else {
			TS_sps->set(TS_accepting_state);
			dfa_sps->set(best_dfa_accepting_state);
		}
		int init_node_ind = Graph<float>::augmentedStateFunc(TS_sps->getCurrNode(), dfa_sps->getCurrNode(), n, m);
		//spw.state_weights[init_node_ind] = 0.0;
		first_search_weights[init_node_ind] = 0.0;
		included[init_node_ind] = true;
		std::pair<int, float> curr_leaf = {init_node_ind, 0.0f};
		pq.push(curr_leaf);

		std::pair<bool, std::vector<int>> accepting;
		float min_accepting_cost = -1;
		//int tree_end_node = 0;
		int prev_leaf_ind, solution_ind;
		bool exit_failure = false;
		while (!pq.size() > 0) {
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
			int curr_leaf_ind = curr_leaf.first;
			std::pair<unsigned int, unsigned int> ret_inds;
			Graph<float>::augmentedStateMap(curr_leaf_ind, n, m, ret_inds);

			if (curr_leaf_ind == prev_leaf_ind) {
				std::cout<<"no sol"<<std::endl;
				exit_failure = true;
				break;
			}
			//std::cout<<" ------ CURRENT LEAF: "<<curr_leaf_ind<<std::endl;
			float curr_leaf_weight = curr_leaf.second;
			//curr_leaf.second->setInf();
			//pq.push(curr_leaf);
			//printQueue(pq);

			//if (tree_end_node != 0) { // Use temp_nodeptr from outside the loop (init) when no nodes are in tree (=0)
			//	temp_nodeptr = tree.getNodeDataptr(curr_leaf_ind);
			//}

			// SET:
			//TS_sps->set(ret_inds.first);
			//dfa_sps->set(ret_inds.second);

			//std::cout<<" SET NODE: "<<node_list[curr_leaf_ind]->i;
			//for (int i=0; i<node_size; ++i) {
			//	//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
			//	dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
			//}
			//std::cout<<" ---\n";
			con_data.clear();
			std::vector<int> con_nodes;
			std::vector<WL*> con_data;
			if (round == 0) {
				TS_sps->getConnectedDataEVAL(con_data);
				TS_sps->getConnectedNodesEVAL(con_nodes);
			} else {
				TS_sps->getParentDataEVAL(con_data);
				TS_sps->getParentNodesEVAL(con_nodes);
			
			}
			//std::cout<<"printing con nodes and data"<<std::endl;
			////std::cout<<"con nodes size:"<<con_nodes.size()<<std::endl;
			////std::cout<<"con data size:"<<con_data.size()<<std::endl;
			//for (int i=0; i<con_data.size(); ++i) {
			//	std::cout<<con_nodes[i]<<std::endl;
			//	std::cout<<con_data[i]->label<<std::endl;
			//}
			std::pair<unsigned int, float*> src;
			src.first = curr_leaf_ind;
			src.second = &curr_leaf_weight;
			//for (auto con_data_ptr : con_data) {
			for (int j=0; j<con_data.size(); ++j) {
				// Reset after checking connected nodes
				TS->set(ret_inds.first);
				dfa_sps->set(ret_inds.second);

				//for (int i=0; i<node_size; ++i) {
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

				if (round == 0) {
					found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
				} else {
					found_connection = TS->evalReverse(temp_str, true); // second arg tells wether or not to evolve on graph
				
				}
				//std::cout<<"ts evolved state: "<<TS->getCurrNode()<<" under action: "<<temp_str<<std::endl;
				//

				if (!found_connection) {
					std::cout<<"Error ("<<search_type<<"): Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
					return false;
				}
				const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
				found_connection = false;
				//std::cout<<"\n before eval"<<std::endl;
				for (int ii=0; ii<lbls->size(); ++ii) {
					//std::cout<<"       labels: --- "<<lbls->operator[](ii)<<std::endl;
					//std::cout<<"DFA::: "<<(i-1)<<" curr node; "<<(dfa_list_ordered->operator[](i-1)->getCurrNode())<<std::endl;
					if (round == 0) {
						if (dfa_sps->eval(lbls->operator[](ii), true)) {
							found_connection = true;
							break;
						}
					} else {
						if (dfa_sps->evalReverse(lbls->operator[](ii), true)) {
							found_connection = true;
							break;
						}
					}
				}
				if (!found_connection) {
					std::cout<<"Error ("<<search_type<<"): Did not find connectivity in DFA. \n";
					return false;
				}
				//for (int i=0; i<node_size + 1; ++i) {
				//	if (i == 0) { // eval TS first!!! (so that getCurrNode spits out the adjacent node, not the current node)
				//		found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
				//		//std::cout<<"ts evolved state: "<<TS->getCurrNode()<<" under action: "<<temp_str<<std::endl;
				//		if (!found_connection) {
				//			std::cout<<"Error: Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
				//			return false;
				//		}
				//	} else { // eval DFAi
				//		// There could be multiple logical statements the correspond to the same
				//		// underlying label, therefore we must test all of the statements until
				//		// one is found. Only one is needed due to determinism.
				//		const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
				//		found_connection = false;
				//		//std::cout<<"\n before eval"<<std::endl;
				//		for (int ii=0; ii<lbls->size(); ++ii) {
				//			//std::cout<<"       labels: --- "<<lbls->operator[](ii)<<std::endl;
				//			//std::cout<<"DFA::: "<<(i-1)<<" curr node; "<<(dfa_list_ordered->operator[](i-1)->getCurrNode())<<std::endl;
				//			if (dfa_list_ordered->operator[](i-1)->eval(lbls->operator[](ii), true)) {
				//				found_connection = true;
				//				break;
				//			}
				//		}
				//	}
				//	if (!found_connection) {
				//		std::cout<<"Error: Connectivity was not found for either the TS or a DFA. \n";
				//		return false;
				//	}
				//}
				// Check if the node has already been seen:

				bool unique = true;
				int connected_node_ind = Graph<float>::augmentedStateFunc(TS_sps->getCurrNode(), dfa_sps->getCurrNode(), n, m);
				if (included[connected_node_ind]) {
					continue;
				} else {
					included[connected_node_ind] = true;
				}

				//for (int i=0; i<node_list.size(); ++i) {
				//	//std::cout<<"i: "<<i<<std::endl;

				//	//std::cout<<"Check node: "<<TS->getCurrNode();
				//	//for (int ii=0; ii<node_size; ++ii) {
				//	//	std::cout<<", "<<dfa_list_ordered->operator[](ii)->getCurrNode();
				//	//	
				//	//}
				//	//std::cout<<"\n";
				//	//std::cout<<"--Nodelist node: "<<node_list[i]->i;
				//	//for (int ii=0; ii<node_size; ++ii) {
				//	//	std::cout<<", "<<node_list[i]->v[ii];
				//	//	
				//	//}
				//	//std::cout<<"\n";
				//	for (int ii=0; ii<node_size + 1; ++ii) {
				//		unique = false;
				//		//std::cout<<"Check node: ";
				//		if (ii==0) {
				//			//std::cout<<TS->getCurrNode();
				//			if (TS->getCurrNode() != node_list[i]->i) {
				//				//std::cout<<"\n";
				//				unique = true;
				//				break;
				//			}
				//		} else {
				//			//std::cout<<", "<<dfa_list_ordered->operator[](ii)->getCurrNode();
				//			if (dfa_list_ordered->operator[](ii-1)->getCurrNode() != node_list[i]->v[ii-1]) {
				//				unique = true;
				//				break;
				//			}
				//		}
				//	}
				//	if (!unique) {
				//		//std::cout<<"   -NOT UNIQUE"<<std::endl;
				//		break;
				//	}
				//}
				//if (!unique) {
				//	continue;
				//}

				// Made it through connectivity check, therefore we can append the state to the tree:
				// GET:
				//IVFlexLex<T>* new_temp_nodeptr = newNode();
				//parents.push_back(src.first);
				//new_temp_nodeptr->i = TS->getCurrNode();
				//bool all_accepting = true;
				//std::cout<<"printing temp lex set fill: "<<std::endl;
				if (round == 0) {
					if (dfa_sps->isCurrAccepting()) {
						accepting.first = true;
						accepting.second.push_back(connected_node_ind);
					}
				}
				//for (int i=0; i<node_size; ++i) {
				//	new_temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();

				//	// If the dfa is accepting at the evolved ind, append no cost, otherwise append
				//	// the cost of taking the action:
				//	//std::cout<<"-DFA: "<<i<<", curr becore accepting: "<<dfa_list_ordered->operator[](i)->getCurrNode()<<std::endl;
				//	//std::cout<<" ->DFA: ";
				//	if (dfa_list_ordered->operator[](i)->isCurrAccepting()) {
				//		//std::cout<<i<<" is accepting";
				//		temp_lex_set_fill[i] = 0;
				//	} else {
				//		all_accepting = false;
				//		temp_lex_set_fill[i] = temp_weight;
				//	}
				//	//std::cout<<"\n";
				//	//std::cout<<temp_lex_set_fill[i]<<std::endl;
				//}
				//new_temp_nodeptr->lex_set = *(curr_leaf_weight);
				//std::cout<<"before: ";
				//new_temp_nodeptr->lex_set.print();
				//new_temp_nodeptr->lex_set += temp_lex_set_fill;
				//std::cout<<"printing flex set: \n";
				//new_temp_nodeptr->lex_set.print();

				// BUILD CONNECTION:
				//tree_end_node++;
				//std::cout<<"tree_end_node: "<<tree_end_node<<"node_list size(): "<<node_list.size()<<std::endl;
				if (round == 0) {
					first_search_weights[connected_node_ind] = curr_leaf_weight + temp_weight;
				} else {
					spw.state_weights[connected_node_ind] = curr_leaf_weight + temp_weight;
				}
				std::pair<int, float> new_leaf = {connected_node_ind, (curr_leaf_weight + temp_weight)};
				pq.push(new_leaf); // add to prio queue
				//if (new_leaf.first == 66) {
				//	std::cout<<"ADDING NODE 66!!!!!!!!!!!! "; 
				//	new_leaf.second->print();
				//}
				tree.connect(src, {connected_node_ind, &first_search_weights[connected_node_ind]});
				//if (all_accepting) {
				//	accepting.first = true;
				//	accepting.second.push_back(tree_end_node);
				//}
			}

			// If accepting node is found, check if it is the smallest cost candidate on the next iteration.
			// If so, then the algorithm is finished because all other candidate nodes have a longer path.
			if (accepting.first && round == 0) {
				int top_node = pq.top().first;
				for (int i=0; i<accepting.second.size(); ++i) {
					if (top_node == accepting.second[i]) {
						found_target_state = true;
						solution_ind = top_node;
						std::pair<unsigned int, unsigned int> sol_inds;
						Graph<float>::augmentedStateMap(curr_leaf_ind, n, m, sol_inds);
						TS_accepting_state = sol_inds.first;
						best_dfa_accepting_state = sol_inds.second;
						std::cout<<"Found a solution!"<<std::endl;
						break;
					}
				}
			}
			prev_leaf_ind = curr_leaf_ind;
		}
		if ((round == 0) && !found_target_state) {
			std::cout<<"Error: Target state not found\n";
			exit_failure = true; // currently not using this
			return false;
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
bool SymbSearch<T>::search() {	
	//std::chrono::time_point<std::chrono::system_clock> end_time;
	//std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
	std::vector<const std::vector<std::string>*> total_alphabet(node_size);
	for (int i=0; i<node_size; ++i) {
		total_alphabet[i] = dfa_list_ordered->operator[](i)->getAlphabetEVAL();
	}
	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet
	auto compare = [](std::pair<int, T*> pair1, std::pair<int, T*> pair2) {
		return *(pair1.second) > *(pair2.second);
	};
	std::priority_queue<std::pair<int, T*>, std::vector<std::pair<int, T*>>, decltype(compare)> pq(compare);

	// Tree to keep history as well as parent node list:
	Graph<IVFlexLex<T>> tree;
	std::vector<int> parents;

	
	// Fill the root tree node (init node):
	IVFlexLex<T>* temp_nodeptr = newNode();
	parents.push_back(0);
	std::vector<float> temp_lex_set_fill(node_size);
	temp_nodeptr->i = 0;
	for (int i=0; i<node_size; ++i) {
		temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();
		// Weights are zero for the root node:
		temp_lex_set_fill[i] = 0;
	}
	temp_nodeptr->lex_set = temp_lex_set_fill;

	std::pair<int, T*> curr_leaf = {0, &(temp_nodeptr->lex_set)};
	pq.push(curr_leaf);

	std::vector<WL*> con_data;
	std::pair<bool, std::vector<int>> accepting;
	int tree_end_node = 0;
	int prev_leaf_ind, solution_ind;
	bool finished = false;
	while (!finished) {
		pq.top();
		//std::cout<<"\nprior queue: "<<std::endl;
		printQueue(pq);
		
		//int pause;
		//std::cin >> pause;
		curr_leaf = pq.top();
		//std::cout<<" TOP --- Ind: "<<pq.top().first<<std::endl;
		//std::cout<<" TOP --- LexSet: ";
		//pq.top().second->print();

		pq.pop();
		int curr_leaf_ind = curr_leaf.first;
		if (curr_leaf_ind == prev_leaf_ind) {
			std::cout<<"no sol"<<std::endl;
			break;
		}
		//std::cout<<" ------ CURRENT LEAF: "<<curr_leaf_ind<<std::endl;
		T* curr_leaf_weight = curr_leaf.second;
		//curr_leaf.second->setInf();
		//pq.push(curr_leaf);
		//printQueue(pq);
		if (tree_end_node != 0) { // Use temp_nodeptr from outside the loop (init) when no nodes are in tree (=0)
			temp_nodeptr = tree.getNodeDataptr(curr_leaf_ind);
		}
		// SET:
		//TS->set(node_list[curr_leaf_ind]->i);
		////std::cout<<" SET NODE: "<<node_list[curr_leaf_ind]->i;
		//for (int i=0; i<node_size; ++i) {
		//	//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
		//	dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
		//}

		//std::cout<<" ---\n";
		con_data.clear();
		TS->getConnectedDataEVAL(con_data);
		std::vector<int> con_nodes;
		TS->getConnectedNodesEVAL(con_nodes);
		//std::cout<<"printing con nodes and data"<<std::endl;
		////std::cout<<"con nodes size:"<<con_nodes.size()<<std::endl;
		////std::cout<<"con data size:"<<con_data.size()<<std::endl;
		//for (int i=0; i<con_data.size(); ++i) {
		//	std::cout<<con_nodes[i]<<std::endl;
		//	std::cout<<con_data[i]->label<<std::endl;
		//}
		std::pair<unsigned int, IVFlexLex<T>*> src;
		src.first = curr_leaf_ind;
		src.second = temp_nodeptr;
		//for (auto con_data_ptr : con_data) {
		for (int j=0; j<con_data.size(); ++j) {
			//std::cout<<"j = "<<j<<std::endl;
			TS->set(node_list[curr_leaf_ind]->i);
			for (int i=0; i<node_size; ++i) {
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

			for (int i=0; i<node_size + 1; ++i) {
				if (i == 0) { // eval TS first!!! (so that getCurrNode spits out the adjacent node, not the current node)
					found_connection = TS->eval(temp_str, true); // second arg tells wether or not to evolve on graph
					//std::cout<<"ts evolved state: "<<TS->getCurrNode()<<" under action: "<<temp_str<<std::endl;
					if (!found_connection) {
						std::cout<<"Error: Did not find connectivity in TS. Current node: "<<TS->getCurrNode()<<", Action: "<<temp_str<<std::endl;
						return false;
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
					return false;
				}
			}
			// Check if the node has already been seen:
			bool unique = true;
			//std::cout<<"\n Uniqueness check: \n"<<std::endl;
			//std::cout<<"printing nodelist: "<<std::endl;
			for (int i=0; i<node_list.size(); ++i) {
				//std::cout<<"i: "<<i<<std::endl;

				//std::cout<<"Check node: "<<TS->getCurrNode();
				//for (int ii=0; ii<node_size; ++ii) {
				//	std::cout<<", "<<dfa_list_ordered->operator[](ii)->getCurrNode();
				//	
				//}
				//std::cout<<"\n";
				//std::cout<<"--Nodelist node: "<<node_list[i]->i;
				//for (int ii=0; ii<node_size; ++ii) {
				//	std::cout<<", "<<node_list[i]->v[ii];
				//	
				//}
				//std::cout<<"\n";
				for (int ii=0; ii<node_size + 1; ++ii) {
					unique = false;
					//std::cout<<"Check node: ";
					if (ii==0) {
						//std::cout<<TS->getCurrNode();
						if (TS->getCurrNode() != node_list[i]->i) {
							//std::cout<<"\n";
							unique = true;
							break;
						}
					} else {
						//std::cout<<", "<<dfa_list_ordered->operator[](ii)->getCurrNode();
						if (dfa_list_ordered->operator[](ii-1)->getCurrNode() != node_list[i]->v[ii-1]) {
							unique = true;
							break;
						}
					}
				}
				if (!unique) {
					//std::cout<<"   -NOT UNIQUE"<<std::endl;
					break;
				}
			}
			if (!unique) {
				continue;
			}

			// Made it through connectivity check, therefore we can append the state to the tree:
			// GET:
			IVFlexLex<T>* new_temp_nodeptr = newNode();
			parents.push_back(src.first);
			new_temp_nodeptr->i = TS->getCurrNode();
			bool all_accepting = true;
			//std::cout<<"printing temp lex set fill: "<<std::endl;
			for (int i=0; i<node_size; ++i) {
				new_temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();

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
				//std::cout<<"\n";
				//std::cout<<temp_lex_set_fill[i]<<std::endl;
			}
			new_temp_nodeptr->lex_set = *(curr_leaf_weight);
			//std::cout<<"before: ";
			//new_temp_nodeptr->lex_set.print();
			new_temp_nodeptr->lex_set += temp_lex_set_fill;
			//std::cout<<"printing flex set: \n";
			//new_temp_nodeptr->lex_set.print();

			// BUILD CONNECTION:
			tree_end_node++;
			//std::cout<<"tree_end_node: "<<tree_end_node<<"node_list size(): "<<node_list.size()<<std::endl;
			std::pair<int, T*> new_leaf = {tree_end_node, &(new_temp_nodeptr->lex_set)};
			pq.push(new_leaf); // add to prio queue
			if (new_leaf.first == 66) {
				std::cout<<"ADDING NODE 66!!!!!!!!!!!! "; 
				new_leaf.second->print();
			}
			tree.connect(src, {tree_end_node, new_temp_nodeptr});
			if (all_accepting) {
				accepting.first = true;
				accepting.second.push_back(tree_end_node);
			}
		}

		// If accepting node is found, check if it is the smallest cost candidate on the next iteration.
		// If so, then the algorithm is finished because all other candidate nodes have a longer path.
		if (accepting.first) {
			int top_node = pq.top().first;
			for (int i=0; i<accepting.second.size(); ++i) {
				if (top_node == accepting.second[i]) {
					finished = true;
					solution_ind = top_node;
					std::cout<<"Found a solution!"<<std::endl;
					break;
				}
			}
		}
		prev_leaf_ind = curr_leaf_ind;
	}
	if (finished) {
		extractPath(parents, solution_ind);
		int p_space_size = 1;
		//p_space_size *=
		//for 

		//end_time = std::chrono::system_clock::now();
		//double search_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
		//std::cout<<"Search time (milliseconds): "<<search_time<<std::endl;
		std::cout<<"Finished. Tree size: "<<tree.size()<<std::endl; //<<", Maximum product graph (no pruning) size: "<<
		//std::cout<<"\n\n ...Finished with plan."<<std::endl;
		return true;
	} else {
		std::cout<<"Failed (no plan)."<<std::endl;
		return false;
	}
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
		std::cout<<"node list node: "<<curr_node<<", ";
		std::cout<<"ts node: "<<node_list[curr_node]->i<<", ";
		std::cout<<"lexset: ";
		node_list[curr_node]->lex_set.print();
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
SymbSearch<T>::~SymbSearch() {
	for (int i=0; i<node_list.size(); ++i) {
		delete node_list[i];
	}
}

//template class SymbSearch<LexSet>; CANNOT USE BECAUSE CTOR REQUIRES MU
template class SymbSearch<FlexLexSetS>;
template class SymbSearch<REQLex>;


