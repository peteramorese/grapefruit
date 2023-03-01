#include<iostream>

#include "tools/Logging.h"

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/SymbolicProductAutomaton.h"

#include "theory/PartialSatisfactionAutomaton.h"
#include "theory/PartialSatisfactionAutomatonEdgeInheritor.h"

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
	ts_props.init_coordinate_y = 1;
	ts_props.environment.addRegion("water", 3, 3, 5, 5, "blue");
	ts_props.environment.addRegion("carpet", 6, 0, 8, 2, "orange");
	DiscreteModel::GridWorldAgent::serializeConfig(ts_props, "test_config.yaml");

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::GridWorldAgent::generate(ts_props);

	ts->print();
	NEW_LINE;
	ts->listPropositions();
	NEW_LINE;

	/////////////////   DFAs   /////////////////

	std::shared_ptr<FormalMethods::PartialSatisfactionDFA> dfa_1 = std::make_shared<FormalMethods::PartialSatisfactionDFA>();
	dfa_1->deserialize("dfas/dfa_0.yaml", "dfas/sub_map_0.yaml");
	//dfa_1->deserialize("dfas/dfa_0.yaml");
	dfa_1->print();

	std::shared_ptr<FormalMethods::PartialSatisfactionDFA> dfa_2 = std::make_shared<FormalMethods::PartialSatisfactionDFA>();
	dfa_2->deserialize("dfas/dfa_1.yaml", "dfas/sub_map_1.yaml");
	//dfa_2->deserialize("dfas/dfa_1.yaml");
	dfa_2->print();

	std::vector<std::shared_ptr<FormalMethods::PartialSatisfactionDFA>> dfas = {dfa_1, dfa_2};

	FormalMethods::Alphabet combined_alphbet = dfa_1->getAlphabet() + dfa_2->getAlphabet();

	//for (const auto& obs : combined_alphbet) LOG("obs: " << obs);

	ts->addAlphabet(combined_alphbet);
	


	/////////////////   Planner   /////////////////

	using EdgeInheritor = DiscreteModel::PartialSatisfactionAutomataEdgeInheritor<DiscreteModel::TransitionSystem>;
	using SymbolicGraph = DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::PartialSatisfactionDFA, EdgeInheritor>;

	BOPreferencePlanner<
		EdgeInheritor, 
		FormalMethods::PartialSatisfactionDFA,
		CostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, 
		WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost>
	> planner(ts, dfas);

	DiscreteModel::State init_state = ts->getGenericNodeContainer()[0];

	// Set the partial satisfaction weights
	std::vector<FormalMethods::SubstitutionCost> weights = {1, 1};

	WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost>::setWeights(weights);

	// Plan
	LOG("Planning...");
	auto plan_set = planner.plan(init_state);
	LOG("Finished.");

	if (plan_set.size()) {
		LOG("Planner success!");
		uint32_t i = 0;
		for (const auto& plan : plan_set) {
			LOG("Plan " << i << " Cost: " << plan.cost.template get<0>().cost << " Preference Cost: " << plan.cost.template get<1>().preferenceFunction());
			plan.print();	
			plan.serialize("test_plan.yaml");
		}
		DiscreteModel::GridWorldAgent::serializeConfig(ts_props, "test_grid_world_config.yaml");
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}


	return 0;
}
