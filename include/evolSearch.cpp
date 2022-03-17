
#include "evolSearch.h"


void evolSearch::calcEvol(const std::vector<DFA_EVAL*>& dfa_list) {

}

bool evolSearch::Search(TS_EVAL* TS, const std::vector<DFA_EVAL*>& dfa_list, pq_t& pq_base, const Item& src_item) {
	auto compare_tree = [](const std::pair<int, float> leafL, const std::pair<int, float> leafR) {
		return leafL.second > leafR.second;
	}; 
	std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float>>, decltype(compare_tree)> pq_tree(compare);
	std::vector<bool> visited(TS->size(), false);

	TS->set(src_item.src_node); // Set the initial search node to be the one found in the pq item
	pq_tree.push({TS->getCurrNode(), 0});

	Graph<WIV> tree; // W: cost, V: branch indices ([branch_dfa1, branch_dfa2, ...])


	while (pq_tree.size() > 0) {

	}
}

bool evolSearch::Plan(TS_EVAL* TS, const std::vector<DFA_EVAL*>& dfa_list) {
	const num_dfas = dfa_list.size();
	auto compare  = [](const Item& itemL, const Item& itemR) {
		return itemL.cost > itemR.cost; // pq is only ordered by the cost
	};

	pq_t pq(compare);

	calcEvol(dfa_list); // Determine the evol values for each DFA state

	// Construct the alphabet and map the states:
	std::vector<const std::vector<std::string>*> total_alphabet(num_dfas);
	TS->mapStatesToLabels(total_alphabet);

	TS->reset();
	int curr_state = TS->getCurrNode();

	bool finished = false;
	while (!finished) {



		// Finishing condition:
	}

}
