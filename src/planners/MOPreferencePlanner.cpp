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

#include "theory/PartialSatisfactionAutomaton.h"
#include "theory/PartialSatisfactionAutomatonEdgeInheritor.h"

#include "models/GridWorldAgent.h"

#include "tools/ArgParser.h"

using namespace TP;
using namespace TP::Planner;

int main(int argc, char* argv[]) {
 
	ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v', "Run in verbose mode");

	std::string dfa_directory = parser.parse<std::string>("dfa-directory", "./dfas", "Directory that contains dfa files");
	std::string dfa_file_template = parser.parse<std::string>("dfa-file-template", "dfa_#.yaml", "Naming convention for dfa file");
	std::string sub_map_file_template = parser.parse<std::string>("sub-map-file-template", "sub_map_#.yaml", "Naming convention for sub map file");

	std::string config_filepath = parser.parse<std::string>("config-filepath", "", "Filepath to grid world config");

	bool write_plans = parser.hasFlag('w', "Write plans to plan files");
	std::string plan_directory = parser.parse<std::string>("plan-directory", "./grid_world_plans", "Directory to output plan files");
	std::string plan_file_template = parser.parse<std::string>("plan-file-template", "plan_#.yaml", "Naming convention for output plan files");

	uint32_t n_dfas = parser.parse<uint32_t>("n-dfas", 1, "Number of dfa files to read in");
	
	if (parser.enableHelp()) return 0;

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


	ts->addAlphabet(combined_alphbet);

	/////////////////   Planner   /////////////////

	using EdgeInheritor = DiscreteModel::PartialSatisfactionAutomataEdgeInheritor<DiscreteModel::TransitionSystem>;
	using SymbolicGraph = DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::PartialSatisfactionDFA, EdgeInheritor>;

	MOPreferencePlanner<
		EdgeInheritor, 
		FormalMethods::PartialSatisfactionDFA,
		CostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, 
		SumDelayPreferenceCostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, // Action costs
		WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost> // Automata costs
	> planner(ts, dfas);

	// Set the weighting
	std::vector<FormalMethods::SubstitutionCost> weights(dfas.size(), 1);
	weights[0] = 2;
	weights[1] = 4;

	ASSERT(weights.size() == dfas.size(), "Number of weights must match number of tasks");
	WeightedSumPreferenceCostObjective<SymbolicGraph, FormalMethods::SubstitutionCost>::setWeights(weights);

	DiscreteModel::State init_state = DiscreteModel::GridWorldAgent::makeInitState(ts_props, ts);

	LOG("Planning...");
	auto plan_set = planner.plan(init_state);
	LOG("Finished.");

	if (plan_set.size()) {
		LOG("Planner success!");
		uint32_t i = 0;
		for (const auto& plan : plan_set) {

			std::string title = "Plan " + std::to_string(i) + 
				" Cost: " + std::to_string(plan.cost.template get<0>().cost) + 
				" Sum Delay Cost: " + std::to_string(plan.cost.template get<1>().preferenceFunction()) +
				" Weighted Sum PS Cost: " + std::to_string(plan.cost.template get<2>().preferenceFunction());
			LOG(title);
			if (verbose) plan.print();	
			if (write_plans) {
				std::string plan_filepath = plan_directory + "/" + templateToLabel(plan_file_template, i);
				plan.serialize(plan_filepath, title);
			}
			++i;
		}
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}

	return 0;
}
