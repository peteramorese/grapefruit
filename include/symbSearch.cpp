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

bool SymbSearch::search() {	
	std::vector<const std::vector<std::string>*> total_alphabet;
	for (int i=0; i<node_size; ++i) {
		total_alphabet[i] = dfa_list_ordered->operator[](i)->getAlphabetEVAL();
	}
	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet
	auto compare = [](std::pair<int, FlexLexSetS*> pair1, std::pair<int, FlexLexSetS*> pair2) {
		return *(pair1.second) < *(pair2.second);
	};
	std::priority_queue<std::pair<int, FlexLexSetS*>, std::vector<std::pair<int, FlexLexSetS*>>, decltype(compare)> pq(compare);
	Graph<IVFlexLexS> tree;
	bool finished = false;
	
	// Fill the root tree node (init node):
	IVFlexLexS* temp_nodeptr = newNode();
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
	while (!finished) {
		curr_leaf = pq.top();
		pq.pop();
		int curr_leaf_ind = curr_leaf.first;
		FlexLexSetS* curr_leaf_weight = curr_leaf.second;
		if (tree_end_node != 0) { // Use temp_nodeptr from outside the loop (init) when no nodes are in tree (=0)
			temp_nodeptr = tree.getNodeDataptr(curr_leaf_ind);
		}
		// SET:
		TS->set(node_list[curr_leaf_ind]->i);
		for (int i=0; i<node_size; ++i) {
			dfa_list_ordered->operator[](i)->set(node_list[curr_leaf_ind]->v[i]);
		}
		TS->getConnectedDataEVAL(con_data);
		std::pair<unsigned int, IVFlexLexS*> src;
		src.first = curr_leaf_ind;
		src.second = temp_nodeptr;
		for (auto con_data_ptr : con_data) {
			std::string temp_str = con_data_ptr->label;
			float temp_weight = con_data_ptr->weight;
			bool found_connection = true;

			for (int i=0; i<node_size + 1; ++i) {
				if (i == 0) { // eval TS first!!! (so that getCurrNode spits out the adjacent node, not the current node)
					found_connection = TS->eval(temp_str);
				} else { // eval DFAi
					// There could be multiple logical statements the correspond to the same
					// underlying label, therefore we must test all of the statements until
					// one is found. Only one is needed due to determinism.
					const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
					found_connection = false;
					for (int ii=0; ii<lbls->size(); ++ii) {
						if (dfa_list_ordered->operator[](i-1)->eval(lbls->operator[](ii))) {
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
			// Made it through connectivity check, therefore we can append the state to the tree:
			// GET:
			IVFlexLexS* new_temp_nodeptr = newNode();
			new_temp_nodeptr->i = TS->getCurrNode();
			bool all_accepting = true;
			for (int i=0; i<node_size; ++i) {
				new_temp_nodeptr->v[i] = dfa_list_ordered->operator[](i)->getCurrNode();

				// If the dfa is accepting at the evolved ind, append no cost, otherwise append
				// the cost of taking the action:
				if (dfa_list_ordered->operator[](i)->isCurrAccepting()) {
					temp_lex_set_fill[i] = 0;
				} else {
					all_accepting = false;
					temp_lex_set_fill[i] = temp_weight;
				}
			}
			new_temp_nodeptr->lex_set += temp_lex_set_fill;
			std::cout<<"printing flex set: \n";
			new_temp_nodeptr->lex_set.print();

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
					std::cout<<"Found a solution!"<<std::endl;
					break;
				}
			}
		}
	}
	if (finished) {
		return true;
	} else {
		return false;
	}
}


