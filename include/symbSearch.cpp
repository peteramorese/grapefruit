#include "symbSearch.h"

SymbSearch::SymbSearch() {}

void SymbSearch::setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_) {
	dfa_list_ordered = dfa_list_ordered_;
	node_size = dfa_list_ordered->size();
}

void SymbSearch::setTransitionSystem(TS_EVAL<State>* TS_) {
	TS = TS_;	
}

void SymbSearch::setFlexibilityParam(float mu_) {
	mu = mu_;
}

IVFlexLexS* SymbSearch::newNode() {
	IVFlexLexS* node_i = new IVFlexLexS(mu, node_size);
	//node_i->v.resize(node_size);
	node_list.push_back(node_i);
	return node_i;
}

template<typename T>
void SymbSearch::printQueue(T queue) {
	while (!queue.empty()) {
		std::cout<<"Ind: "<<queue.top().first<<std::endl;
		std::cout<<"LexSet: ";
		queue.top().second->print();
		queue.pop();
	}
}

bool SymbSearch::search() {	
	std::vector<const std::vector<std::string>*> total_alphabet(node_size);
	for (int i=0; i<node_size; ++i) {
		total_alphabet[i] = dfa_list_ordered->operator[](i)->getAlphabetEVAL();
	}
	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet
	auto compare = [](std::pair<int, FlexLexSetS*> pair1, std::pair<int, FlexLexSetS*> pair2) {
		return *(pair1.second) > *(pair2.second);
	};
	std::priority_queue<std::pair<int, FlexLexSetS*>, std::vector<std::pair<int, FlexLexSetS*>>, decltype(compare)> pq(compare);

	// Tree to keep history as well as parent node list:
	Graph<IVFlexLexS> tree;
	std::vector<int> parents;

	
	// Fill the root tree node (init node):
	IVFlexLexS* temp_nodeptr = newNode();
	parents.push_back(0);
	std::vector<float> temp_lex_set_fill(node_size);
	temp_nodeptr->i = 0;
	for (int i=0; i<node_size; ++i) {
		temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();
		// Weights are zero for the root node:
		temp_lex_set_fill[i] = 0;
	}
	temp_nodeptr->lex_set = temp_lex_set_fill;

	std::pair<int, FlexLexSetS*> curr_leaf = {0, &(temp_nodeptr->lex_set)};
	pq.push(curr_leaf);

	std::vector<WL*> con_data;
	std::pair<bool, std::vector<int>> accepting;
	int tree_end_node = 0;
	int prev_leaf_ind, solution_ind;
	bool finished = false;
	while (!finished) {
		//std::cout<<"\nprior queue: "<<std::endl;
		//printQueue(pq);
		
		//int pause;
		//std::cin >> pause;
		curr_leaf = pq.top();
		std::cout<<"Ind: "<<pq.top().first<<std::endl;
		std::cout<<"LexSet: ";
		pq.top().second->print();

		pq.pop();
		int curr_leaf_ind = curr_leaf.first;
		if (curr_leaf_ind == prev_leaf_ind) {
			std::cout<<"no sol"<<std::endl;
			break;
		}
		//std::cout<<" ------ CURRENT LEAF: "<<curr_leaf_ind<<std::endl;
		FlexLexSetS* curr_leaf_weight = curr_leaf.second;
		//curr_leaf.second->setInf();
		//pq.push(curr_leaf);
		//printQueue(pq);
		if (tree_end_node != 0) { // Use temp_nodeptr from outside the loop (init) when no nodes are in tree (=0)
			temp_nodeptr = tree.getNodeDataptr(curr_leaf_ind);
		}
		// SET:
		TS->set(node_list[curr_leaf_ind]->i);
		//std::cout<<" SET NODE: "<<node_list[curr_leaf_ind]->i;
		for (int i=0; i<node_size; ++i) {
			//std::cout<<", "<<node_list[curr_leaf_ind]->v[i];
			dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
		}
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
		std::pair<unsigned int, IVFlexLexS*> src;
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
					}
				} else { // eval DFAi
					// There could be multiple logical statements the correspond to the same
					// underlying label, therefore we must test all of the statements until
					// one is found. Only one is needed due to determinism.
					const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
					found_connection = false;
					//std::cout<<"\n before eval"<<std::endl;
					for (int ii=0; ii<lbls->size(); ++ii) {
						//std::cout<<"DFA--- "<<lbls->operator[](ii);
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
			IVFlexLexS* new_temp_nodeptr = newNode();
			parents.push_back(src.first);
			new_temp_nodeptr->i = TS->getCurrNode();
			bool all_accepting = true;
			//std::cout<<"printing temp lex set fill: "<<std::endl;
			for (int i=0; i<node_size; ++i) {
				new_temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();

				// If the dfa is accepting at the evolved ind, append no cost, otherwise append
				// the cost of taking the action:
				//std::cout<<"DFA: "<<i<<", curr becore accepting: "<<dfa_list_ordered->operator[](i)->getCurrNode()<<std::endl;
				//std::cout<<"DFA: ";
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
			std::pair<int, FlexLexSetS*> new_leaf = {tree_end_node, &(new_temp_nodeptr->lex_set)};
			pq.push(new_leaf); // add to prio queue
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
		//std::cout<<"\n\n ...Finished with plan."<<std::endl;
		return true;
	} else {
		std::cout<<"Failed (no plan)."<<std::endl;
		return false;
	}
}

void SymbSearch::extractPath(const std::vector<int>& parents, int accepting_state) {
	int curr_node = accepting_state;
	std::vector<int> reverse_TS_state_sequence;
	//std::cout<<" given accepting state: "<<accepting_state<<std::endl;
	//reverse_TS_state_sequence.push_back(node_list[accepting_state);
	std::cout<<"reverse plan: "<<std::endl;
	while (curr_node != 0) {
		//std::cout<<"node: "<<node_list[curr_node]->i<<", ";
		//std::cout<<"lexset: ";
		node_list[curr_node]->lex_set.print();
		reverse_TS_state_sequence.push_back(node_list[curr_node]->i);
		curr_node = parents[curr_node];
	}
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

SymbSearch::~SymbSearch() {
	for (int i=0; i<node_list.size(); ++i) {
		delete node_list[i];
	}
}


