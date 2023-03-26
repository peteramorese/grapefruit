#include<iostream>

#include "tools/Logging.h"

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "planners/MOPreferencePlanner.h"
#include "planners/PreferenceCostObjectivePlugins.h"

#include "models/GridWorldAgent.h"


using namespace TP;
using namespace TP::Planner;

int main() {
 

	DiscreteModel::GridWorldAgentProperties ts_props;
	ts_props.n_x = 10;
	ts_props.n_y = 10;
	ts_props.init_coordinate_x = 0;
	ts_props.init_coordinate_y = 0;

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::GridWorldAgent::generate(ts_props);

	ts->print();
	//NEW_LINE;
	//ts->listPropositions();

	/////////////////   DFAs   /////////////////

	std::shared_ptr<FormalMethods::DFA> dfa_1 = std::make_shared<FormalMethods::DFA>();
	dfa_1->setAcceptingStates({0});
	dfa_1->setInitStates({2});
	dfa_1->setAlphabet({"!x_5_y_0", "x_5_y_0 & !x_4_y_9", "x_5_y_0 & x_4_y_9", "!x_4_y_9", "x_4_y_9", "1"});
	dfa_1->connect(0, 0, "1");
	dfa_1->connect(1, 0, "x_4_y_9");
	dfa_1->connect(1, 1, "!x_4_y_9");
	dfa_1->connect(2, 1, "x_5_y_0 & !x_4_y_9");
	dfa_1->connect(2, 0, "x_5_y_0 & x_4_y_9");
	dfa_1->connect(2, 2, "!x_5_y_0");

	NEW_LINE;
	dfa_1->print();
	//const auto& automaton_children = automaton->getChildren(unwrapped_nodes[automaton_ind]);

	std::shared_ptr<FormalMethods::DFA> dfa_2 = std::make_shared<FormalMethods::DFA>();
	dfa_2->setAcceptingStates({0});
	dfa_2->setInitStates({1});
	dfa_2->setAlphabet({"!x_3_y_2", "x_3_y_2", "1"});
	dfa_2->connect(0, 0, "1");
	dfa_2->connect(1, 0, "x_3_y_2");
	dfa_2->connect(1, 1, "!x_3_y_2");

	NEW_LINE;
	dfa_2->print();

	std::vector<std::shared_ptr<FormalMethods::DFA>> dfas = {dfa_1, dfa_2};

	FormalMethods::Alphabet combined_alphbet = dfa_1->getAlphabet() + dfa_2->getAlphabet();
	//for (const auto& letter : combined_alphbet) LOG("letter: " << letter);
	ts->addAlphabet(combined_alphbet);


	/////////////////   Planner   /////////////////

	using EdgeInheritor = DiscreteModel::ModelEdgeInheritor<DiscreteModel::TransitionSystem, FormalMethods::DFA>;
	using SymbolicGraph = DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::DFA, EdgeInheritor>;

	MOPreferencePlanner<
		EdgeInheritor, 
		FormalMethods::DFA,
		CostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, 
		SumDelayPreferenceCostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>,
		WeightedSumPreferenceCostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t, CostInheritor::Model>
	> planner(ts, dfas);

	// Set the weighting
	WeightedSumPreferenceCostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t, CostInheritor::Model>::setWeights({1.0f, 10.0f});

	DiscreteModel::State init_state = DiscreteModel::GridWorldAgent::makeInitState(ts_props, ts);

	LOG("Planning...");
	auto plan_set = planner.plan(init_state);
	LOG("Finished.");

	if (plan_set.size()) {
		LOG("Planner success!");
		uint32_t i = 0;
		for (const auto& plan : plan_set) {

			LOG("Plan " << i << " Cost: " << plan.cost.template get<0>().cost << " Sum Delay Cost: " << plan.cost.template get<1>().preferenceFunction() << " Weighted Sum Cost: " << plan.cost.template get<2>().preferenceFunction());
			plan.print();	
		}
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}


	return 0;
}
