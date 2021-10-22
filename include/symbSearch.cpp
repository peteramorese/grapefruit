#include "symbSearch.h"

SymbSearch::SymbSearch() {}

void SymbSearch::setAutomataPrefs(const std::vector<DFA_EVAL*>* dfa_list_ordered_) {
	dfa_list_ordered = dfa_list_ordered_;
	node_size = dfa_list_ordered->size();
}

void SymbSearch::setTransitionSystem(const TS_EVAL<State>* TS_) {
	TS = TS_;	
}

void SymbSearch::setFlexibilityParam(float mu_) {
	mu = mu_;
}

std::vector<unsigned int>* SymbSearch::newNode() {
	WIV* node_i = new WIV;
	node_i->v.resize(node_size);
	node_list.push_back(node_i);
}

bool SymbSearch::search() {	
	std::vector<std::vector<std::string>*> total_alphabet;
	for (int i=0; i<node_size; ++i) {
		total_alphabet[i] = dfa_list_ordered->getAlphabetEVAL();
	}
	TS->mapStatesToLabels(total_alphabet); // This is in efficient with diverse/large alphabet
	auto compare = [](std::pair<int, float> pair1, std::pair<int, float> pair2) {
		return pair1.second < pair2.second;
	};
	std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float>>, decltype(compare)> pq(compare);
	Graph<WIV> tree;
	bool finished = false;
	WIV* temp_nodeptr = newNode()
	temp_nodeptr->i = 0;
	for (int i=0; i<node_size; ++i) {
		temp_nodeptr->v[i] = dfa_list_ordered->operator[i]->getCurrNode();
	}
	std::pair<int, float> curr_leaf = {0, 0.0f};
	std::vector<WL*> con_data;
	pq.push(curr_leaf);
	int tree_size = 1;
	while (!finished) {
		curr_leaf = pq.top();
		int curr_leaf_ind = curr_leaf.first;
		float curr_leaf_weight = curr_leaf.second;
		pq.pop();
		// SET:
		TS->set(node_list[curr_leaf_ind]->i);
		for (int i=0; i<node_size; ++i) {
			dfa_list_ordered[i]->set(node_list[curr_leaf_ind]->v[i]);
		}
		TS->getConnectedDataEVAL(con_data);
		std::pair<unsigned int, WIV*> src;
		src.first = curr_leaf_ind;
		src.second =
		for (auto con_data_ptr : con_data) {
			std::string temp_str = con_data_ptr->label;
			float temp_weight = con_data_ptr->weight;
			bool found_connection = true;

			for (int i=0; i<node_size + 1; ++i) {
				if (i == 0) { // eval TS first!!! (so that getCurrNode spits out the adjacent node, not the current node)
					found_connection = TS->eval(temp_str)
				} else { // eval DFAi
					// There could be multiple logical statements the correspond to the same
					// underlying label, therefore we must test all of the statements until
					// one is found. Only one is needed due to determinism.
					const std::vector<std::string>* lbls = returnStateLabels(TS->getCurrNode());
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
			temp_nodeptr = newNode();
			temp_nodeptr->i = TS->getCurrNode();
			for (int i=0; i<node_size; ++i) {
				temp_nodeptr->v[i] = dfa_list_ordered->operator[i]->getCurrNode();
			}
			// BUILD CONNECTION:

		}
	}
}

