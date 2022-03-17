#pragma once

#include<memory>
#include "graph.h"
#include "transitionSystem.h"

class evolSearch {
	private:
		struct Item {
			float cost; // Cost of travelling to the target node
			float evolution; // Cumulative DFA evol value
			std::vector<int> path; // Path from src node to the target. Last index is dst node
			int src_node; // src node
		};
		struct LabelSet {
			
		};
		typedef std::priority_queue<Item, std::vector<Item>, decltype(compare)> pq_t;
	public:
};
