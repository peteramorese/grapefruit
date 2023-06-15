#include<iostream>

#include "TaskPlanner.h"

#include "BehaviorHandler.h"
#include "Learner.h"
#include "TrueBehavior.h"
//#include "Benchmark.h"
#include "Misc.h"

using namespace PRL;

Selector getSelector(const std::string& label) {
	if (label == "aif" || label == "Aif") {
		return Selector::Aif;
	} else if (label == "uniform" || label == "Uniform") {
		return Selector::Uniform;
	} else if (label == "topsis" || label == "TOPSIS") {
		return Selector::Topsis;
	} else if (label == "weights" || label == "Weights") {
		return Selector::Weights;
	} 
	ASSERT(false, "Unrecognized selector '" << label <<"'");
}

int main(int argc, char* argv[]) {
 
	TP::ArgParser parser(argc, argv);

	bool verbose = parser.parse<void>('v', "Run in verbose mode");
	bool calc_regret = parser.parse<void>("regret", 'r', "Calculate Pareto regret");
	//bool compare = parser.hasKey("compare", "Compare the learned estimates to the true estimates");

	auto formula_filepath = parser.parse<std::string>("formula-filepath", 'f', "formulas.yaml", "File that contains all formulas");
	auto config_filepath = parser.parse<std::string>("config-filepath", 'c', "Filepath to grid world config");
	auto data_filepath = parser.parse<std::string>("data-filepath", 'd', "Specify a filepath to write the collected data to");
	auto selector_label = parser.parse<std::string>("selector", "aif", "Pareto point selector (aif (active inference), uniform, topsis, weights)");

	auto max_planning_instances = parser.parse<uint32_t>("instances", 'i', 10, "Max number of planning instances");
	auto n_trials = parser.parse<uint32_t>("trials", 't', 1, "Number of trials to run");
	auto n_efe_samples = parser.parse<uint32_t>("efe-samples", 1000u, "Number of samples used for approximating the expected posterior entropy");

	auto confidence = parser.parse<float>("confidence", 1.0f, "UCB confidence for planner (exploration/expoitation)");

	parser.enableHelp();

	Selector selector = getSelector(selector_label.get());
	
	/////////////////   Transition System   /////////////////
	
	TP::DiscreteModel::GridWorldAgentProperties ts_props;
	if (!config_filepath) {
		ts_props.n_x = 10;
		ts_props.n_y = 10;
		ts_props.init_coordinate_x = 0;
		ts_props.init_coordinate_y = 0;
	} else {
		ts_props = TP::DiscreteModel::GridWorldAgent::deserializeConfig(config_filepath.get());
	}

	std::shared_ptr<TP::DiscreteModel::TransitionSystem> ts = TP::DiscreteModel::GridWorldAgent::generate(ts_props);

	if (verbose) ts->print();

	/////////////////   DFAs   /////////////////

	auto dfas = TP::FormalMethods::createDFAsFromFile(formula_filepath.get());

	TP::FormalMethods::Alphabet combined_alphbet;
	for (const auto& dfa : dfas) {
		combined_alphbet = combined_alphbet + dfa->getAlphabet();
		if (verbose) dfa->print();
	}

	if (verbose) ts->listPropositions();

	ts->addAlphabet(combined_alphbet);

	/////////////////   Planner   /////////////////

	constexpr uint64_t N = 2;
	using EdgeInheritor = TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>;
	using SymbolicGraph = TP::DiscreteModel::SymbolicProductAutomaton<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA, EdgeInheritor>;
	using BehaviorHandlerType = BehaviorHandler<SymbolicGraph, N>;
	using PreferenceDistributionType = TP::Stats::Distributions::FixedMultivariateNormal<N>;

 	std::shared_ptr<SymbolicGraph> product = std::make_shared<SymbolicGraph>(ts, dfas);

	/////////////////   True Behavior   /////////////////

	std::shared_ptr<GridWorldTrueBehavior<N>> true_behavior = std::make_shared<GridWorldTrueBehavior<N>>(product, config_filepath.get());

	//if (verbose)
	//	true_behavior->print();

	// Make the preference behavior distribution
	PreferenceDistributionType p_ev = deserializePreferenceDist<N>(config_filepath.get());

	// Get the default transition mean if the file contains it
	std::pair<bool, Eigen::Matrix<float, N, 1>> default_mean = deserializeDefaultMean<N>(config_filepath.get());

	std::shared_ptr<Regret<SymbolicGraph, N>> regret_handler;
	if (calc_regret)
 		regret_handler = std::make_shared<Regret<SymbolicGraph, N>>(product, true_behavior);

	std::shared_ptr<DataCollector<N>> data_collector = std::make_shared<DataCollector<N>>(product, p_ev, regret_handler);

	for (uint32_t trial = 0; trial < n_trials; ++trial) {

		std::shared_ptr<BehaviorHandlerType> behavior_handler;
		if (default_mean.first)
			behavior_handler = std::make_shared<BehaviorHandlerType>(product, 1, confidence, default_mean.second);
		else 
			behavior_handler = std::make_shared<BehaviorHandlerType>(product, 1, confidence);

		Learner<N> prl(behavior_handler, data_collector, n_efe_samples, verbose);

		// Initialize the agent's state
		TP::DiscreteModel::State init_state = TP::DiscreteModel::GridWorldAgent::makeInitState(ts_props, ts);
		prl.initialize(init_state);

		auto samplerFunction = [&](TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action) {
			return true_behavior->sample(src_node, dst_node, action);
		};

		// Run the PRL
		prl.run(p_ev, samplerFunction, max_planning_instances, selector);
		if (verbose) {
			LOG("Finished!");
			std::string total_cost_str{};
			for (uint32_t i = 0; i < N; ++i) 
				total_cost_str += std::to_string(data_collector->cumulativeCost()[i]) + ((i < N) ? ", " : "");
			PRINT_NAMED("Total Cost...............................", total_cost_str);
			PRINT_NAMED("Steps....................................", data_collector->steps());
			PRINT_NAMED("Instances................................", data_collector->numInstances());
			std::string avg_cost_str{};
			auto avg_cost_per_instance = data_collector->avgCostPerInstance();
			for (uint32_t i = 0; i < N; ++i) 
				avg_cost_str += std::to_string(avg_cost_per_instance[i]) + ((i < N) ? ", " : "");
			PRINT_NAMED("Average cost per per instance............", avg_cost_str);
		}

		//if (compare) 
		//	true_behavior->compare(*behavior_handler);


		if (data_filepath) {
			TP::Serializer szr(data_filepath.get());
			data_collector->serialize(szr);
			szr.done();
		}

	}

	return 0;
}