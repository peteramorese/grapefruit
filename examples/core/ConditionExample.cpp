#include<iostream>

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"

#include "tools/Logging.h"

using namespace GF;
using namespace GF::DiscreteModel;

void printConditionEvaluation(const std::string& condition_name, Condition& cond, const State& state) {
	LOG("Input state for condition '" << condition_name << "':");
	state.print();
	bool result = cond.evaluate(state);
	PRINT_NAMED("Condition evaluated to", ((result) ? "true" : "false"));
}

void printTransitionConditionEvaluation(const std::string& condition_name, TransitionCondition& cond, const State& pre_state, const State& post_state) {
	LOG("Input pre state for condition '" << condition_name << "':");
	pre_state.print();
	LOG("Input post state:");
	post_state.print();
	bool result = cond.evaluate(pre_state, post_state);
	PRINT_NAMED("Condition evaluated to", ((result) ? "true" : "false"));
}

int main() {
 
	StateSpace ss_manipulator(5);
	std::vector<std::string> locations = {"L0", "L1", "L2", "L3", "L4"};

	std::vector<std::string> ee_locations = locations;
	ee_locations.push_back("stow");

	std::vector<std::string> obj_locations = locations;
	obj_locations.push_back("ee");
	
	// Create state space:
	ss_manipulator.setDimension(0, "ee_loc", ee_locations); // end effector locations
	ss_manipulator.setDimension(1, "obj_0_loc", obj_locations); // obj 0 locations
	ss_manipulator.setDimension(2, "obj_1_loc", obj_locations); // obj 1 locations
	ss_manipulator.setDimension(3, "obj_2_loc", obj_locations); // obj 2 locations
	ss_manipulator.setDimension(4, "holding", {"T", "F"}); // end effector is holding an object

	// Add a label group denoting the obj locations
	ss_manipulator.addGroup("obj_locations", {"obj_0_loc", "obj_1_loc", "obj_2_loc"});
	ss_manipulator.addGroup("spc_objs", {"obj_0_loc", "obj_1_loc"});

	// Add a domain capturing only the locations that an object can be dropped in ('locations')
	ss_manipulator.addDomain("drop_locs", locations);


	/////////////////   Condition   /////////////////

	// Equals:
	PRINT("\n Equals:");
	{
	// "Object 1 has been dropped in location 'L3'"
	Condition cond(ConditionJunction::Conjunction); // Each sub-condition is conjoined
	cond.addCondition(ConditionArg::Label, "obj_1_loc", ConditionOperator::Equals, ConditionArg::Variable, locations[3]); // obj_1_loc == "L3"
	cond.addCondition(ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); // holding == "F"
	
	State state(&ss_manipulator, {"stow", "L2", "L3", "L0", "F"});
	printConditionEvaluation("Obj 1 in L3", cond, state);

	state = {"stow", "L2", "L3", "L0", "T"};
	printConditionEvaluation("Obj 1 in L3", cond, state);
	}
	{
	// "Object 2 is not in location 'L2' and object 1 is in location 'L1'"
	Condition cond(ConditionJunction::Conjunction); // Each sub-condition is conjoined
	cond.addCondition(ConditionArg::Label, "obj_2_loc", ConditionOperator::Equals, ConditionArg::Variable, locations[2], ConditionLogical::Negate); // obj_2_loc != "L2"
	cond.addCondition(ConditionArg::Label, "obj_1_loc", ConditionOperator::Equals, ConditionArg::Variable, locations[1]); // obj_1_loc == "L1"
	
	State state(&ss_manipulator, {"L3", "L2", "L1", "L0", "F"});
	printConditionEvaluation("Obj 2 not in L2 and obj 1 in L1", cond, state);

	state = {"L3", "L2", "L1", "L2", "F"};
	printConditionEvaluation("Obj 2 not in L2 and obj 1 in L1", cond, state);
	}

	// InDomain:
	PRINT("\n InDomain:");
	{
	// "Object 0 and end effector are in domain 'drop_locs'"
	Condition cond(ConditionJunction::Conjunction); // Each sub-condition is conjoined
	cond.addCondition(ConditionArg::Label, "obj_0_loc", ConditionOperator::InDomain, ConditionArg::Domain, "drop_locs"); // obj_2_loc in "drop_locs"
	cond.addCondition(ConditionArg::Label, "ee_loc", ConditionOperator::InDomain, ConditionArg::Domain, "drop_locs"); // ee_loc in "drop_locs"
	
	State state(&ss_manipulator, {"L3", "L2", "L1", "L0", "F"});
	printConditionEvaluation("Obj 0 and ee_loc in drop_locs", cond, state);

	state = {"L3", "ee", "L1", "L2", "F"};
	printConditionEvaluation("Obj 0 and ee_loc in drop_locs", cond, state);
	}
	{
	// "Group 'obj_locations' in domain 'drop_locs'"
	Condition cond(ConditionJunction::Conjunction); // Each sub-condition is conjoined
	cond.addCondition(ConditionArg::Group, "obj_locations", ConditionOperator::InDomain, ConditionArg::Domain, "drop_locs"); // obj_locations in "drop_locs"
	
	State state(&ss_manipulator, {"L3", "L2", "L1", "L0", "F"});
	printConditionEvaluation("obj_locations in drop_locs", cond, state);

	state = {"L3", "ee", "L1", "L2", "F"};
	printConditionEvaluation("obj_locations in drop_locs", cond, state);

	Serializer szr("test_condition.yaml");
	cond.serialize(szr);
	szr.done();

	Condition same_cond;
	Deserializer dszr("test_condition.yaml");
	same_cond.deserialize(dszr);

	state = {"L3", "L2", "L1", "L0", "F"};
	printConditionEvaluation("obj_locations in drop_locs (after serialization)", same_cond, state);

	state = {"L3", "ee", "L1", "L2", "F"};
	printConditionEvaluation("obj_locations in drop_locs (after serialization)", same_cond, state);
	}

	// ArgFind:
	PRINT("\n ArgFind:");
	{
	// "'ee' is found among the group 'spc_locations'"
	Condition cond(ConditionJunction::Conjunction); // Each sub-condition is conjoined
	cond.addCondition(ConditionArg::Group, "spc_objs", ConditionOperator::ArgFind, ConditionArg::Variable, "ee", "ee_spc_objs"); // find among "spc_objs" the variable "ee"
	cond.addCondition(ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T", "is_holding"); 
	
	State state(&ss_manipulator, {"L3", "L2", "ee", "L0", "F"});
	printConditionEvaluation("ee found among spc_objs", cond, state);

	state = {"L3", "L4", "L1", "ee", "F"};
	printConditionEvaluation("ee found among spc_objs", cond, state);

	}

	/////////////////   TransitionCondition   /////////////////
	// ArgFind:
	PRINT("\n Manipulator Examples:");
	{
	TransitionCondition cond("grasp", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

	// Pre: "End effector is not holding and the variable in 'ee_loc' is found in group 'obj_locations'"
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); // 'holding' == 'F'
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg");

	// Post: "End effector is holding and the label found from 'ArgFind' in the precondition assumes the variable 'ee'"
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); // 'holding' == 'T'
	cond.addCondition(ConditionType::Post, ConditionArg::Variable, "ee", ConditionOperator::Equals, ConditionArg::ArgLabel, "", "arg");
	
	State pre_state(&ss_manipulator, {"L3", "L2", "L3", "L0", "F"});
	State post_state(&ss_manipulator, {"L3", "L2", "ee", "L0", "T"});
	printTransitionConditionEvaluation("Grasp", cond, pre_state, post_state);

	// Demonstrate exclusion equals:
	post_state = {"L3", "L2", "ee", "L1", "T"};
	printTransitionConditionEvaluation("Grasp", cond, pre_state, post_state);
	}
	{
	TransitionCondition cond("transport", 2.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

	// Pre: "End effector is holding and ee_loc variable does not match any in obj_locations"
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); // 'holding' == 'T'
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg_1", ConditionLogical::Negate);
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "ee_loc", ConditionOperator::ArgFind, ConditionArg::None, "", "arg_2");

	// Post: "End effector is holding and the arg variable stored in 'arg2' (the ee_loc) does not equal the new variable stored in ee_loc"
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); // 'holding' == 'T'
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgVariable, "", "arg_2", ConditionLogical::Negate); // ee_loc has changed
	cond.addCondition(ConditionType::Post, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "na", ConditionLogical::Negate); // ee_loc is not in an object's location
	
	State pre_state(&ss_manipulator, {"L4", "L2", "ee", "L0", "T"});
	State post_state(&ss_manipulator, {"L3", "L2", "ee", "L0", "T"});
	printTransitionConditionEvaluation("Transport", cond, pre_state, post_state);

	pre_state = {"L4", "L2", "ee", "L0", "T"};
	post_state = {"L2", "L2", "ee", "L0", "T"};
	printTransitionConditionEvaluation("Transport", cond, pre_state, post_state);

	// Demonstrate exclusion equals:
	pre_state = {"L4", "L2", "ee", "L0", "T"};
	post_state = {"L3", "L1", "ee", "L0", "T"};
	printTransitionConditionEvaluation("Transport", cond, pre_state, post_state);
	}
	{
	TransitionCondition cond("release", 2.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

	// Pre: "End effector is holding and ee_loc var is not found among obj_locations and 'ee' is found among obj_locations"
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); 
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg_1", ConditionLogical::Negate);
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Variable, "ee", "arg_2");

	// Post: "End effector is no longer holding, the arg label found in 'arg_2' equals the variable in ee_loc"
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); 
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgLabel, "", "arg_2"); 
	
	State pre_state(&ss_manipulator, {"L0", "ee", "L3", "L1", "T"});
	State post_state(&ss_manipulator, {"L0", "L0", "L3", "L1", "F"});
	printTransitionConditionEvaluation("Release", cond, pre_state, post_state);

	pre_state = {"L4", "L2", "ee", "L0", "T"};
	post_state = {"L3", "L2", "L4", "L0", "F"};
	printTransitionConditionEvaluation("Release", cond, pre_state, post_state);

	// Demonstrate exclusion equals:
	pre_state = {"L4", "L2", "ee", "L0", "T"};
	post_state = {"L4", "ee", "L4", "L0", "T"};
	printTransitionConditionEvaluation("Release", cond, pre_state, post_state);
	}



	return 0;
}
