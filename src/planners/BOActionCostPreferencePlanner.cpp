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

#include "tools/Misc.h"
#include "tools/ArgParser.h"

using namespace TP;
using namespace TP::Planner;

int main(int argc, char* argv[]) {
 
	ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v');

	std::string dfa_directory = parser.parseAsString("dfa-directory", "./dfas");
	std::string dfa_file_template = parser.parseAsString("dfa-file-template", "dfa_#.yaml");

	std::string config_filepath = parser.parseAsString("config-filepath");

	bool write_plans = parser.hasFlag('w');
	std::string plan_directory = parser.parseAsString("plan-directory", "./grid_world_plans");
	std::string plan_file_template = parser.parseAsString("plan-file-template", "plan_#.yaml");


	uint32_t n_dfas = parser.parseAsUnsignedInt("n-dfas", 1);

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

	//ts->print();

	/////////////////   DFAs   /////////////////

	std::vector<std::shared_ptr<FormalMethods::DFA>> dfas(n_dfas);

	FormalMethods::Alphabet combined_alphbet;
	for (uint32_t i=0; i<n_dfas; ++i) {
		dfas[i] = std::make_shared<FormalMethods::DFA>();
		std::string dfa_filepath = dfa_directory + "/" + templateToLabel(dfa_file_template, i);
		if (verbose) LOG("Reading in dfa file: " << dfa_filepath);
		dfas[i]->deserialize(dfa_filepath);
		combined_alphbet = combined_alphbet + dfas[i]->getAlphabet();
		dfas[i]->print();
	}


	//for (const auto& letter : combined_alphbet) LOG("letter: " << letter);
	ts->addAlphabet(combined_alphbet);


	/////////////////   Planner   /////////////////

	using EdgeInheritor = DiscreteModel::ModelEdgeInheritor<DiscreteModel::TransitionSystem, FormalMethods::DFA>;
	using SymbolicGraph = DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::DFA, EdgeInheritor>;

	BOPreferencePlanner<
		EdgeInheritor, 
		FormalMethods::DFA,
		CostObjective<SymbolicGraph, DiscreteModel::TransitionSystemLabel::cost_t>, 
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
			LOG("Plan " << i << " Cost: " << plan.cost.template get<0>().cost << " Preference Cost: " << plan.cost.template get<1>().preferenceFunction());
			if (verbose) plan.print();	
			if (write_plans) {
				std::string plan_filepath = plan_directory + "/" + templateToLabel(plan_file_template, i);
				plan.serialize(plan_filepath);
			}
			++i;
		}
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}


	return 0;
}
