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

#include "tools/Misc.h"
#include "tools/ArgParser.h"

using namespace TP;
using namespace TP::Planner;

int main(int argc, char* argv[]) {
 

	ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v');

	std::string dfa_directory = parser.parse<std::string>("dfa-directory", "./dfas");
	std::string dfa_file_template = parser.parse<std::string>("dfa-file-template", "dfa_#.yaml");
	std::string sub_map_file_template = parser.parse<std::string>("sub-map-file-template", "sub_map_#.yaml");

	std::string config_filepath = parser.parse<std::string>("config-filepath");

	bool write_plans = parser.hasFlag('w');
	std::string plan_directory = parser.parse<std::string>("plan-directory", "./grid_world_plans");
	std::string plan_file_template = parser.parse<std::string>("plan-file-template", "plan_#.yaml");

	std::string pareto_front_filepath = parser.parse<std::string>("pareto-front-filepath");

	uint32_t n_dfas = parser.parse<uint32_t>("n-dfas", 1);

	DiscreteModel::GridWorldAgentProperties ts_props;
	if (config_filepath.empty()) {
		ts_props.n_x = 10;
		ts_props.n_y = 10;
		ts_props.init_coordinate_x = 0;
		ts_props.init_coordinate_y = 0;
	} else {
		ts_props = DiscreteModel::GridWorldAgent::deserializeConfig(config_filepath);
	}

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::GridWorldAgent::generate(ts_props);

	if (verbose) ts->print();

	/////////////////   DFAs   /////////////////
	std::vector<std::shared_ptr<FormalMethods::PartialSatisfactionDFA>> dfas(n_dfas);

	FormalMethods::Alphabet combined_alphbet;
	for (uint32_t i=0; i<n_dfas; ++i) {
		dfas[i] = std::make_shared<FormalMethods::PartialSatisfactionDFA>();
		std::string dfa_filepath = dfa_directory + "/" + templateToLabel(dfa_file_template, i);
		std::string sub_map_filepath = dfa_directory + "/" + templateToLabel(sub_map_file_template, i);
		if (verbose) LOG("Reading in dfa file: " << dfa_filepath << " with sub map: " << sub_map_filepath);
		dfas[i]->deserialize(dfa_filepath, sub_map_filepath);
		combined_alphbet = combined_alphbet + dfas[i]->getAlphabet();
		if (verbose) dfas[i]->print();
	}


	//for (const auto& letter : combined_alphbet) LOG("letter: " << letter);
	ts->addAlphabet(combined_alphbet);
	if (verbose) ts->getObservationContainer().print();


	/////////////////   Planner   /////////////////

	using EdgeInheritor = DiscreteModel::PartialSatisfactionAutomataEdgeInheritor<DiscreteModel::TransitionSystem>;
	using SymbolicGraph = DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::PartialSatisfactionDFA, EdgeInheritor>;

	using Obj1 = CostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>;
	using Obj2 = WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost>;

	BOPreferencePlanner<
		EdgeInheritor, 
		FormalMethods::PartialSatisfactionDFA,
		Obj1,	
		Obj2	
	> planner(ts, dfas);

	DiscreteModel::State init_state = ts->getGenericNodeContainer()[0];

	// Set the partial satisfaction weights
	std::vector<FormalMethods::SubstitutionCost> weights = {1, 1, 1, 1, 2};

	WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost>::setWeights(weights);

	// Plan
	LOG("Planning...");
	auto plan_set = planner.plan(init_state);
	LOG("Finished.");

	if (plan_set.size()) {
		LOG("Planner success!");
		uint32_t i = 0;
		for (const auto& plan : plan_set) {
			std::string title = "Plan " + std::to_string(i) + " Cost: " + std::to_string(plan.cost.template get<0>().cost) + " Preference Cost: " + std::to_string(plan.cost.template get<1>().preferenceFunction());
			LOG(title);
			if (verbose) plan.print();	
			if (write_plans) {
				std::string plan_filepath = plan_directory + "/" + templateToLabel(plan_file_template, i);
				plan.serialize(plan_filepath, title);
			}
			++i;
		}
		if (write_plans && !pareto_front_filepath.empty()) {
			auto objCostToFloatArray = [](const Containers::TypeGenericArray<Obj1, Obj2>& cost) {
				Containers::FixedArray<2, float> float_arr;
				float_arr[0] = cost.template get<0>().cost;
				float_arr[1] = cost.template get<1>().preferenceFunction();
				return float_arr;
			};
			serializeParetoFront(plan_set, {{"Cost", "Weighted-Sum Preference Cost"}}, objCostToFloatArray, pareto_front_filepath);
		}
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}


	return 0;
}
