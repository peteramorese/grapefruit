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

	bool verbose = parser.parse<void>('v', "Run in verbose mode");

	auto dfa_directory = parser.parse<std::string>("dfa-directory", 'd', "./dfas", "Directory that contains dfa files");
	auto dfa_file_template = parser.parse<std::string>("dfa-file-template", "dfa_#.yaml", "Naming convention for dfa file");
	auto sub_map_file_template = parser.parse<std::string>("sub-map-file-template", "sub_map_#.yaml", "Naming convention for sub map file");

	auto config_filepath = parser.parse<std::string>("config-filepath", 'c', "Filepath to grid world config");

	auto plan_directory = parser.parse<std::string>("plan-directory", 'w', "Directory to output plan files");
	auto plan_file_template = parser.parse<std::string>("plan-file-template", "plan_#.yaml", "Naming convention for output plan files");

	auto n_dfas = parser.parse<uint32_t>("n-dfas", 'n', 1, "Number of dfa files to read in");
	
	auto pareto_front_filepath = parser.parse<std::string>("pareto-front-filepath", "File to write pareto front to");

	parser.enableHelp();

	DiscreteModel::GridWorldAgentProperties ts_props;
	if (!config_filepath) {
		ts_props.n_x = 10;
		ts_props.n_y = 10;
		ts_props.init_coordinate_x = 0;
		ts_props.init_coordinate_y = 0;
	} else {
		ts_props = DiscreteModel::GridWorldAgent::deserializeConfig(config_filepath.get());
	}

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::GridWorldAgent::generate(ts_props);

	if (verbose) ts->print();

	/////////////////   DFAs   /////////////////

	std::vector<std::shared_ptr<FormalMethods::PartialSatisfactionDFA>> dfas(n_dfas.get());

	FormalMethods::Alphabet combined_alphbet;
	for (uint32_t i=0; i<n_dfas.get(); ++i) {
		dfas[i] = std::make_shared<FormalMethods::PartialSatisfactionDFA>();
		std::string dfa_filepath = dfa_directory.get() + "/" + templateToLabel(dfa_file_template.get(), i);
		std::string sub_map_filepath = dfa_directory.get() + "/" + templateToLabel(sub_map_file_template.get(), i);
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
	weights[0] = 5;
	weights[1] = 2;
	weights[2] = 1;

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
			if (plan_directory) {
				std::string plan_filepath = plan_directory.get() + "/" + templateToLabel(plan_file_template.get(), i);
				Serializer szr(plan_filepath);
				plan.serialize(szr, title);
				szr.done();
			}
			++i;
		}
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}

	return 0;
}
