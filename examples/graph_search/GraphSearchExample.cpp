#include<iostream>

#include "tools/Logging.h"
#include "graph_search/SearchProblem.h"

using namespace TP::GraphSearch;

template <class NODE_T, class COST_T>
struct MyHeuristic {
    MyHeuristic(COST_T test_) : test(test_) {}
    COST_T operator()(const NODE_T& node) const {return test;}
    COST_T test;
};

int main() {

    MyHeuristic<uint32_t, float> h(5.0f);

    SymbolicQuantitativeSearchProblem<uint32_t, float, float> test;
    LOG(test.retrieveHeuristic(0));
 
	return 0;
}
