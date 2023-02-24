#include<iostream>

#include "tools/Logging.h"

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "planners/BOPreferencePlanner.h"
#include "planners/PreferenceCostObjectivePlugins.h"

#include "models/GridWorldAgent.h"


using namespace TP;
using namespace TP::Planner;

int main() {
 

	DiscreteModel::GridWorldAgentProperties ts_props;
	ts_props.n_x = 10;
	ts_props.n_y = 10;
	ts_props.init_coordinate_x = 0;
	ts_props.init_coordinate_x = 1;

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::GridWorldAgent::generate(ts_props);

	ts->print();
	NEW_LINE;
	ts->listPropositions();

	/////////////////   DFAs   /////////////////

	// Formula: F(obj_0_loc_L2 & F obj_1_L1) 
	std::shared_ptr<FormalMethods::DFA> dfa_1 = std::make_shared<FormalMethods::DFA>();
	dfa_1->setAcceptingStates({0});
	dfa_1->setInitStates({2});
	dfa_1->setAlphabet({"!x_4_y_0", "x_4_y_0 & !x_4_y_9", "x_4_y_0 & x_4_y_9", "!x_4_y_9", "x_4_y_9", "1"});
	dfa_1->connect(0, 0, "1");
	dfa_1->connect(1, 0, "x_4_y_9");
	dfa_1->connect(1, 1, "!x_4_y_9");
	dfa_1->connect(2, 1, "x_4_y_0 & !x_4_y_9");
	dfa_1->connect(2, 0, "x_4_y_0 & x_4_y_9");
	dfa_1->connect(2, 2, "!x_4_y_0");

	NEW_LINE;
	dfa_1->print();
	//const auto& automaton_children = automaton->getChildren(unwrapped_nodes[automaton_ind]);

	// Formula: F(obj_1_loc_L1)
	std::shared_ptr<FormalMethods::DFA> dfa_2 = std::make_shared<FormalMethods::DFA>();
	dfa_2->setAcceptingStates({0});
	dfa_2->setInitStates({1});
	dfa_2->setAlphabet({"!x_9_y_9", "x_9_y_9", "1"});
	dfa_2->connect(0, 0, "1");
	dfa_2->connect(1, 0, "x_9_y_9");
	dfa_2->connect(1, 1, "!x_9_y_9");

	NEW_LINE;
	dfa_2->print();

	std::vector<std::shared_ptr<FormalMethods::DFA>> dfas = {dfa_1, dfa_2};

	FormalMethods::Alphabet combined_alphbet = dfa_1->getAlphabet() + dfa_1->getAlphabet();
	ts->addAlphabet(combined_alphbet);


	/////////////////   Planner   /////////////////

	using EdgeInheritor = DiscreteModel::ModelEdgeInheritor<DiscreteModel::TransitionSystem, FormalMethods::DFA>;
	using SymbolicGraph = DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::DFA, EdgeInheritor>;

	BOPreferencePlanner<
		EdgeInheritor, 
		CostPreferenceObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, 
		SumDelayPreferenceCostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>
	> planner(ts, dfas);

	DiscreteModel::State init_state = ts->getGenericNodeContainer()[0];

	LOG("Planning...");
	auto plan_set = planner.plan(init_state);
	LOG("Finished.");

	if (plan_set.size()) {
		LOG("Planner success!");
		uint32_t i = 0;
		for (const auto& plan : plan_set) {
			LOG("Plan " << i);
			plan.print();	
		}
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}


	return 0;
}
