#include<iostream>

#include "TaskPlanner.h"

int main(int argc, char** argv) {
 
	TP::ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v');

	std::string dfa_directory = parser.parse<std::string>("dfa-directory", "./dfas");
	std::string dfa_file_template = parser.parse<std::string>("dfa-file-template", "dfa_#.yaml");

	std::string config_filepath = parser.parse<std::string>("config-filepath");

	bool write_plans = parser.hasFlag('w');
	std::string plan_directory = parser.parse<std::string>("plan-directory", "./grid_world_plans");
	std::string plan_file_template = parser.parse<std::string>("plan-file-template", "plan_#.yaml");

	std::string pareto_front_filepath = parser.parse<std::string>("pareto-front-filepath");

	uint32_t n_dfas = parser.parse<uint32_t>("n-dfas", 1);

	TP::DiscreteModel::GridWorldAgentProperties ts_props;
	if (config_filepath.empty()) {
		ts_props.n_x = 10;
		ts_props.n_y = 10;
		ts_props.init_coordinate_x = 0;
		ts_props.init_coordinate_y = 0;
	} else {
		ts_props = TP::DiscreteModel::GridWorldAgent::deserializeConfig(config_filepath);
	}

	std::shared_ptr<TP::DiscreteModel::TransitionSystem> ts = TP::DiscreteModel::GridWorldAgent::generate(ts_props);

	if (verbose) ts->print();

	/////////////////   DFAs   /////////////////

	std::shared_ptr<TP::FormalMethods::PartialSatisfactionDFA> dfa_1 = std::make_shared<TP::FormalMethods::PartialSatisfactionDFA>();
	dfa_1->deserialize("dfas/dfa_0.yaml", "dfas/sub_map_0.yaml");
	dfa_1->print();

	std::shared_ptr<TP::FormalMethods::PartialSatisfactionDFA> dfa_2 = std::make_shared<TP::FormalMethods::PartialSatisfactionDFA>();
	dfa_2->deserialize("dfas/dfa_1.yaml", "dfas/sub_map_1.yaml");
	dfa_2->print();

	std::vector<std::shared_ptr<TP::FormalMethods::PartialSatisfactionDFA>> dfas = {dfa_1, dfa_2};

	TP::FormalMethods::Alphabet combined_alphbet = dfa_1->getAlphabet() + dfa_2->getAlphabet();

	ts->addAlphabet(combined_alphbet);


	/////////////////   Planner   /////////////////

	//using EdgeInheritor = TP::DiscreteModel::PartialSatisfactionAutomataEdgeInheritor<DiscreteModel::TransitionSystem>;
	//using SymbolicGraph = TP::DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::PartialSatisfactionDFA, EdgeInheritor>;

	//MOPreferencePlanner<
	//	EdgeInheritor, 
	//	FormalMethods::PartialSatisfactionDFA,
	//	CostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, 
	//	SumDelayPreferenceCostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, // Action costs
	//	WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost> // Automata costs
	//> planner(ts, dfas);

	//// Set the weighting
	//std::vector<FormalMethods::SubstitutionCost> weights = {1,2};
	//ASSERT(weights.size() == dfas.size(), "Number of weights must match number of tasks");
	//WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost>::setWeights(weights);

	//DiscreteModel::State init_state = DiscreteModel::GridWorldAgent::makeInitState(ts_props, ts);

	//LOG("Planning...");
	//auto plan_set = planner.plan(init_state);
	//LOG("Finished.");

	//if (plan_set.size()) {
	//	LOG("Planner success!");
	//	uint32_t i = 0;
	//	for (const auto& plan : plan_set) {

	//		LOG("Plan " << i << " Cost: " << plan.cost.template get<0>().cost << " Sum Delay Cost: " << plan.cost.template get<1>().preferenceFunction() << " Weighted Sum Cost: " << plan.cost.template get<2>().preferenceFunction());
	//		plan.print();	
	//	}
	//	DiscreteModel::GridWorldAgent::serializeConfig(ts_props, "test_grid_world_config.yaml");
	//} else {
	//	LOG("Planner failed using init state: " << init_state.to_str());
	//}


	return 0;
}
