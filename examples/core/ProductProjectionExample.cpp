#include<iostream>

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"
#include "core/ProductProjection.h"

#include "models/Manipulator.h"

#include "tools/Logging.h"

using namespace GF;
using namespace GF::DiscreteModel;

int main() {
 

	ManipulatorModelProperties ts_props;
	ts_props.objects = {"obj_0", "obj_1"};
	ManipulatorModelProperties ts_props;
	ts_props.objects = {"obj_0", "obj_1"};
	ts_props.locations = {"loc_L0", "loc_L1", "loc_L2"};
	ts_props.init_obj_locations["obj_0"] = "loc_L0";
	ts_props.init_obj_locations["obj_1"] = "loc_L1";

	std::shared_ptr<TransitionSystem> ts = Manipulator::generate(ts_props);

	/////////////////   DFAs   /////////////////

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

	std::shared_ptr<FormalMethods::DFA> dfa_2 = std::make_shared<FormalMethods::DFA>();
	dfa_2->setAcceptingStates({0});
	dfa_2->setInitStates({1});
	dfa_2->setAlphabet({"!obj_1_loc_L1", "obj_1_loc_L1", "1"});
	dfa_2->connect(0, 0, "1");
	dfa_2->connect(1, 0, "obj_1_loc_L1");
	dfa_2->connect(1, 1, "!obj_1_loc_L1");

	std::shared_ptr<FormalMethods::DFA> dfa_3 = std::make_shared<FormalMethods::DFA>();
	dfa_3->setAcceptingStates({0});
	dfa_3->setInitStates({2});
	dfa_3->setAlphabet({"!obj_0_loc_L1", "obj_0_loc_L1 & !obj_1_loc_L1", "obj_0_loc_L1 & obj_1_loc_L1", "!obj_1_loc_L1", "obj_1_loc_L1", "1"});
	dfa_3->connect(0, 0, "1");
	dfa_3->connect(1, 0, "obj_1_loc_L1");
	dfa_3->connect(1, 1, "!obj_1_loc_L1");
	dfa_3->connect(2, 1, "obj_0_loc_L1 & !obj_1_loc_L1");
	dfa_3->connect(2, 0, "obj_0_loc_L1 & obj_1_loc_L1");
	dfa_3->connect(2, 2, "!obj_0_loc_L1");



	FormalMethods::Alphabet combined_alphbet = dfa_1->getAlphabet() + dfa_1->getAlphabet() + dfa_3->getAlphabet();
	ts->addAlphabet(combined_alphbet);

	/////////////////   Symbolic Product   /////////////////

	// Edge inheritor defaults to inheriting the model edge
	SymbolicProductAutomaton<TransitionSystem, FormalMethods::DFA> from_product(ts, {dfa_1, dfa_2, dfa_3});
	SymbolicProductAutomaton<TransitionSystem, FormalMethods::DFA> onto_product(ts, {dfa_2, dfa_3});

	UnwrappedNode from_node_unwrapped(from_product.rank());
	from_node_unwrapped.ts_node = 0;
	from_node_unwrapped.automata_nodes[0] = 2;
	from_node_unwrapped.automata_nodes[1] = 1;
	from_node_unwrapped.automata_nodes[2] = 2;
	WideNode from_node = from_product.getWrappedNode(from_node_unwrapped.ts_node, from_node_unwrapped.automata_nodes);

	LOG("Full product wrapped node: ts: " << from_node_unwrapped.ts_node << " dfa1: " << from_node_unwrapped.automata_nodes[0] << " dfa2: " << from_node_unwrapped.automata_nodes[1] << " dfa3: " << from_node_unwrapped.automata_nodes[2]);

	LOG("Projected wrapped node: " << ProductAutomatonProjection::project(from_product, onto_product, from_node));
	

	UnwrappedNode onto_node_unwrapped = ProductAutomatonProjection::project(from_product, onto_product, from_node_unwrapped);
	LOG("Projected wrapped node: ts: " << onto_node_unwrapped.ts_node);
	for (ProductRank i; i<onto_node_unwrapped.automata_nodes.size(); ++i) {
		LOG("dfa" << (uint32_t)i << ": " << onto_node_unwrapped.automata_nodes[i]);
	}

	return 0;
}
