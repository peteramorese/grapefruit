#include<iostream>

#include "tools/Logging.h"

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "planners/DeterministicTaskPlanner.h"

#include "models/Manipulator.h"


using namespace TP;
using namespace TP::Planner;

int main() {
 

	DiscreteModel::ManipulatorModelProperties ts_props;
	ts_props.n_locations = 3;
	ts_props.n_objects = 2;
	ts_props.init_obj_locations = {0, 1};

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::Manipulator::generate(ts_props);

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

	FormalMethods::Alphabet combined_alphbet = dfa_1->getAlphabet() + dfa_2->getAlphabet();
	ts->addAlphabet(combined_alphbet);


	/////////////////   Planner   /////////////////

	LOG("Planning...");
	DeterministicTaskPlanner planner(ts, dfas);

	DiscreteModel::State init_state = ts->getGenericNodeContainer()[0];

	Plan plan = planner.plan(init_state);
	LOG("Finished.");

	if (plan.success()) {
		LOG("Planner success!");
		plan.print();	
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}


	return 0;
}
