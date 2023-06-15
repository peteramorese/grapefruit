#include<iostream>

#include "TaskPlanner.h"

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
			if (plan_directory) {
				std::string plan_filepath = plan_directory.get() + "/" + templateToLabel(plan_file_template.get(), i);
				Serializer szr(plan_filepath);
				plan.serialize(szr, title);
				szr.done();
			}
			++i;
		}
		if (pareto_front_filepath) {
			auto objCostToFloatArray = [](const Containers::TypeGenericArray<Obj1, Obj2>& cost) {
				Containers::FixedArray<2, float> float_arr;
				float_arr[0] = cost.template get<0>().cost;
				float_arr[1] = cost.template get<1>().preferenceFunction();
				return float_arr;
			};
			Serializer szr(pareto_front_filepath.get());
			ParetoFrontSerializer::serialize2DAxes(szr, {{"Cost", "Weighted-Sum Preference Cost"}});
			ParetoFrontSerializer::serialize(szr, plan_set, "default", objCostToFloatArray);
			szr.done();
		}
	} else {
		LOG("Planner failed using init state: " << init_state.to_str());
	}


	return 0;
}
