#include<iostream>

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"

#include "tools/Logging.h"

using namespace DiscreteModel;

void printConditionEvaluation(const std::string& condition_name, Condition& cond, const State& state) {
	LOG("Input state for condition '" << condition_name << "':");
	state.print();
	bool result = cond.evaluate(state);
	PRINT_NAMED("Condition evaluated to ", ((result) ? "true" : "false"));
}

int main() {
 
	StateSpace ss_manipulator(5);
	std::vector<std::string> locations = {"L0", "L1", "L2", "L3", "L4"};

	std::vector<std::string> ee_locations = locations;
	ee_locations.push_back("stow");

	std::vector<std::string> obj_locations = locations;
	ee_locations.push_back("ee");
	
	// Create state space:
	ss_manipulator.setDimension(0, "ee_loc", ee_locations); // end effector locations
	ss_manipulator.setDimension(0, "obj_0_loc", obj_locations); // obj 0 locations
	ss_manipulator.setDimension(1, "obj_1_loc", obj_locations); // obj 1 locations
	ss_manipulator.setDimension(1, "obj_2_loc", obj_locations); // obj 2 locations
	ss_manipulator.setDimension(2, "holding", {"T", "F"}); // end effector is holding an object

	// Add a label group denoting the obj locations
	ss_manipulator.addGroup("obj_locations", {"obj_0_loc", "obj_1_loc", "obj_2_loc"});

	// Add a domain capturing only the locations that an object can be dropped in ('locations')
	ss_manipulator.addDomain("drop_locs", locations);


	/////////////////   Condition   /////////////////

	{
	// "Object 1 has been dropped in location 'L3'"
	Condition cond(ConditionJunction::Conjunction); // Each sub-condition is conjoined
	cond.addCondition(ConditionArg::Label, "obj_1_loc", ConditionOperator::Equals, ConditionArg::Variable, locations[3]); // obj_1_loc == "L3"
	cond.addCondition(ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); // holding == "F"
	
	State state(&ss_manipulator, {"stow", "L2", "L3", "L0", "F"});
	printConditionEvaluation("Obj 1 in L3", cond, state);
	}

	return 0;
}
