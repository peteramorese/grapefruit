#include<iostream>

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "tools/Logging.h"

using namespace TP;
using namespace TP::DiscreteModel;

int main() {
 
	/////////////////   Set up the system   /////////////////

	std::vector<std::string> locations = {"L0", "L1", "L2", "L3"};
	std::vector<std::string> objects = {"obj_0_loc", "obj_1_loc"};
	std::vector<std::string> init_state_vars = {"stow", "L0", "L3", "F"};


	/////////////////   State Space   /////////////////

	StateSpace ss_manipulator(objects.size() + 2);

	std::vector<std::string> ee_locations = locations;
	ee_locations.push_back("stow");

	std::vector<std::string> obj_locations = locations;
	obj_locations.push_back("ee");
	
	// Create state space:
	ss_manipulator.setDimension(0, "ee_loc", ee_locations); // end effector locations
    uint32_t i = 1;
    for (const auto& obj : objects) ss_manipulator.setDimension(i++, obj, obj_locations); 
	ss_manipulator.setDimension(i, "holding", {"T", "F"}); // end effector is holding an object

	// Add a label group denoting the obj locations
	ss_manipulator.addGroup("obj_locations", objects);

	// Add a domain capturing only the locations that an object can be dropped in ('locations')
	ss_manipulator.addDomain("drop_locs", locations);

    TransitionSystemProperties props(&ss_manipulator);

	/////////////////   TransitionConditions   /////////////////

    props.conditions.reserve(4);
	{
	TransitionCondition cond("grasp", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

	// Pre: "End effector is not holding and the variable in 'ee_loc' is found in group 'obj_locations'"
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); 
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg");

	// Post: "End effector is holding and the label found from 'ArgFind' in the precondition assumes the variable 'ee'"
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); 
	cond.addCondition(ConditionType::Post, ConditionArg::Variable, "ee", ConditionOperator::Equals, ConditionArg::ArgLabel, "", "arg");

    props.conditions.push_back(cond);
	}
	{
	TransitionCondition cond("transport", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

	// Pre: "End effector is holding and ee_loc variable does not match any in obj_locations"
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T");
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg_1", ConditionLogical::Negate);
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "ee_loc", ConditionOperator::ArgFind, ConditionArg::None, "", "arg_2");

	// Post: "End effector is holding and the arg variable stored in 'arg2' (the ee_loc) does not equal the new variable stored in ee_loc"
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T");
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgVariable, "", "arg_2", ConditionLogical::Negate); // ee_loc has changed
	cond.addCondition(ConditionType::Post, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "na", ConditionLogical::Negate); // ee_loc is not in an object's location

    props.conditions.push_back(cond);
	}
	{
	TransitionCondition cond("release", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

	// Pre: "End effector is holding and ee_loc var is not found among obj_locations and 'ee' is found among obj_locations"
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); 
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg_1", ConditionLogical::Negate);
	cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Variable, "ee", "arg_2");

	// Post: "End effector is no longer holding, the arg label found in 'arg_2' equals the variable in ee_loc"
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); 
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgLabel, "", "arg_2"); 

	// Force ee_loc to stay the same
	cond.forceExclusionComparison("ee_loc");

    props.conditions.push_back(cond);
	}
	{
	TransitionCondition cond("transit", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

	// Pre: "End effector is not holding"
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F");
	cond.addCondition(ConditionType::Pre, ConditionArg::Label, "ee_loc", ConditionOperator::ArgFind, ConditionArg::None, "", "arg");

	// Post: "End effector is not holding and the arg variable stored in 'arg' (the ee_loc) does not equal the new variable stored in ee_loc"
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F");
	cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgVariable, "", "arg", ConditionLogical::Negate); // ee_loc has changed

    props.conditions.push_back(cond);
	}

	/////////////////   Initial State   /////////////////

    props.init_state = init_state_vars;

	/////////////////   Propositions   /////////////////

    uint32_t num_propositions = locations.size() * objects.size();
    props.propositions.reserve(num_propositions);
    for (uint32_t i = 0; i < locations.size(); ++i) {
        for (uint32_t j = 0; j < objects.size(); ++j) {
            Condition prop;
            prop.addCondition(ConditionArg::Label, objects[j], ConditionOperator::Equals, ConditionArg::Variable, locations[i]);
            prop.addCondition(ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F");
            prop.setName(objects[j] + "_" + locations[i]);
            props.propositions.push_back(prop);
        }
    }

    // Generate
    std::shared_ptr<TransitionSystem> ts = TransitionSystemGenerator::generate(props);
    ts->print();
	ts->listPropositions();


	/////////////////   DFAs   /////////////////

	// Formula: F(obj_0_loc_L2 & F obj_1_L1) 
	std::shared_ptr<FormalMethods::DFA> dfa_1;
	dfa_1->setAcceptingStates({0});
	dfa_1->setInitStates({2});
	dfa_1->setAlphabet({"!obj_0_loc_L2", "obj_0_loc_L2 & !obj_1_L1", "obj_0_loc_L2 & obj_1_L1", "!obj_1_L1", "obj_1_L1", "1"});
	dfa_1->connect(0, 0, "1");
	dfa_1->connect(1, 0, "obj_1_L1");
	dfa_1->connect(1, 1, "!obj_1_L1");
	dfa_1->connect(2, 1, "obj_0_loc_L2 & !obj_1_L1");
	dfa_1->connect(2, 0, "obj_0_loc_L2 & obj_1_L1");
	dfa_1->connect(2, 2, "!obj_0_loc_L2");

	// Formula: F(obj_1_loc_L1)
	std::shared_ptr<FormalMethods::DFA> dfa_2;
	dfa_2->setAcceptingStates({0});
	dfa_2->setInitStates({1});
	dfa_2->setAlphabet({"!obj_1_loc_L1", "obj_1_loc_L1", "1"});
	dfa_2->connect(0, 0, "1");
	dfa_2->connect(1, 0, "obj_1_loc_L1");
	dfa_2->connect(1, 1, "!obj_1_loc_L1");

	std::vector<std::shared_ptr<FormalMethods::DFA>> dfas = {dfa_1, dfa_2};


	/////////////////   Symbolic Product   /////////////////

	SymbolicProductAutomaton<TransitionSystem, FormalMethods::DFA> product(ts, dfas);

	Containers::SizedArray<Node> p_unwrapped(product.rank());
	// Set transition system node:
	p_unwrapped[0] = 0;
	
	// Set automaton nodes:
	p_unwrapped[1] = 0;
	p_unwrapped[2] = 0;

	WideNode p = AugmentedNodeIndex::wrap(p_unwrapped, product.getGraphSizes());

	auto children = product.getChildren(p);

	NEW_LINE;
	LOG("Node p: " << p);
	for (auto pp : children) {
		auto pp_unwrapped = AugmentedNodeIndex::unwrap(pp, product.getGraphSizes());
		LOG("   connects to pp: " << pp << " (ts: " << pp_unwrapped[0] << ", dfa 1: " << pp_unwrapped[1] << ", dfa 2:" << pp_unwrapped[2] <<")");
	}

	return 0;
}
