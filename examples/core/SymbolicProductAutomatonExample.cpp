#include<iostream>

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "models/Manipulator.h"

#include "tools/Logging.h"

using namespace TP;
using namespace TP::DiscreteModel;

int main() {
 

	Manipulator::ManipulatorModelProperties ts_props;
	ts_props.n_locations = 3;
	ts_props.n_objects = 2;
	ts_props.init_obj_locations = {0, 1};

	std::shared_ptr<TransitionSystem> ts = Manipulator::generate(ts_props);
        LOG("ptr: " << ts.get());
	ts->size();

	ts->print();
	NEW_LINE;
	ts->listPropositions();

	/////////////////   DFAs   /////////////////

	// Formula: F(obj_0_loc_L2 & F obj_1_L1) 
	std::shared_ptr<FormalMethods::DFA> dfa_1 = std::make_shared<FormalMethods::DFA>();
	dfa_1->setAcceptingStates({0});
	dfa_1->setInitStates({2});
	dfa_1->setAlphabet({"!obj_0_loc_L2", "obj_0_loc_L2 & !obj_1_loc_L1", "obj_0_loc_L2 & obj_1_loc_L1", "!obj_1_loc_L1", "obj_1_loc_L1", "1"});
	dfa_1->connect(0, 0, "1");
	dfa_1->connect(1, 0, "obj_1_loc_L1");
	dfa_1->connect(1, 1, "!obj_1_loc_L1");
	dfa_1->connect(2, 1, "obj_0_loc_L2 & !obj_1_loc_L1");
	dfa_1->connect(2, 0, "obj_0_loc_L2 & obj_1_loc_L1");
	dfa_1->connect(2, 2, "!obj_0_loc_L2");

	NEW_LINE;
	dfa_1->print();
	//const auto& automaton_children = automaton->getChildren(unwrapped_nodes[automaton_ind]);

	// Formula: F(obj_1_loc_L1)
	std::shared_ptr<FormalMethods::DFA> dfa_2 = std::make_shared<FormalMethods::DFA>();
	dfa_2->setAcceptingStates({0});
	dfa_2->setInitStates({1});
	dfa_2->setAlphabet({"!obj_1_loc_L1", "obj_1_loc_L1", "1"});
	dfa_2->connect(0, 0, "1");
	dfa_2->connect(1, 0, "obj_1_loc_L1");
	dfa_2->connect(1, 1, "!obj_1_loc_L1");

	NEW_LINE;
	dfa_2->print();

	std::vector<std::shared_ptr<FormalMethods::DFA>> dfas = {dfa_1, dfa_2};

	FormalMethods::Alphabet combined_alphbet = dfa_1->getAlphabet() + dfa_1->getAlphabet();
	ts->addAlphabet(combined_alphbet);


	/////////////////   Symbolic Product   /////////////////

	SymbolicProductAutomaton<TransitionSystem, FormalMethods::DFA> product(ts, dfas);

	// Get the children symbolically
	{
	Containers::SizedArray<Node> p_unwrapped(product.rank());
	// Set transition system node:
	p_unwrapped[0] = 22;
	
	// Set automaton nodes:
	p_unwrapped[1] = 1;
	p_unwrapped[2] = 1;

	WideNode p = AugmentedNodeIndex::wrap(p_unwrapped, product.getGraphSizes());

	auto children = product.getChildren(p);

	NEW_LINE;
	LOG("Children");
	PRINT_NAMED("Node p: " << p << " (ts: " << p_unwrapped[0] << ", dfa 1: " << p_unwrapped[1] << ", dfa 2:" << p_unwrapped[2] <<")", "connects to");
	for (auto pp : children) {
		auto pp_unwrapped = AugmentedNodeIndex::unwrap(pp, product.getGraphSizes());
		PRINT("   pp: " << pp << " (ts: " << pp_unwrapped[0] << ", dfa 1: " << pp_unwrapped[1] << ", dfa 2:" << pp_unwrapped[2] <<")");
	}
	}

	// Get the parents symbolically
	{
	Containers::SizedArray<Node> pp_unwrapped(product.rank());
	// Set transition system node:
	pp_unwrapped[0] = 24;
	
	// Set automaton nodes:
	pp_unwrapped[1] = 0;
	pp_unwrapped[2] = 0;

	WideNode pp = AugmentedNodeIndex::wrap(pp_unwrapped, product.getGraphSizes());

	auto parents = product.getParents(pp);

	NEW_LINE;
	LOG("Parents");
	PRINT_NAMED("Node pp: " << pp << " (ts: " << pp_unwrapped[0] << ", dfa 1: " << pp_unwrapped[1] << ", dfa 2:" << pp_unwrapped[2] <<")", "connects to");
	for (auto p : parents) {
		auto p_unwrapped = AugmentedNodeIndex::unwrap(p, product.getGraphSizes());
		PRINT("   p: " << p << " (ts: " << p_unwrapped[0] << ", dfa 1: " << p_unwrapped[1] << ", dfa 2:" << p_unwrapped[2] <<")");
	}
	}
	return 0;
}
