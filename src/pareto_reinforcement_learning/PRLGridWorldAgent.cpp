#include<iostream>

#include "TaskPlanner.h"

#include "BehaviorHandler.h"
#include "Learner.h"
#include "TrueBehavior.h"
#include "Benchmark.h"

using namespace PRL;

int main(int argc, char* argv[]) {
 
	TP::ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v', "Run in verbose mode");
	bool benchmark = parser.hasFlag('b', "Benchmark run");
	bool animate = parser.hasFlag('a', "Animate run");
	bool compare = parser.hasKey("compare", "Compare the learned estimates to the true estimates");

	std::string formula_filepath = parser.parse<std::string>("formula-filepath", "formulas.yaml", "File that contains all formulas");
	std::string config_filepath = parser.parse<std::string>("config-filepath", "", "Filepath to grid world config");
	std::string benchmark_filepath = parser.parse<std::string>("bm-filepath", "prl_grid_world_bm.yaml", "File that benchmark data will be written to");
	std::string animation_filepath = parser.parse<std::string>("animation-filepath", "prl_animation.yaml", "File that contains data necessary for animation");

	uint32_t max_planning_instances = parser.parse<uint32_t>("instances", 10, "Max number of planning instances");
	uint32_t n_trials = parser.parse<uint32_t>("trials", 1, "Number of trials to run");
	uint32_t n_efe_samples = parser.parse<uint32_t>("efe-samples", 1000, "Number of samples used for approximating the expected posterior entropy");

	float pc_mean = parser.parse<float>("pc-mean", 50.0f, "Preference distribution cost mean");
	float pc_var = parser.parse<float>("pc-var", 25.0f, "Preference distribution cost variance");
	float pr_mean = parser.parse<float>("pr-mean", 10.0f, "Preference distribution reward mean");
	float pr_var = parser.parse<float>("pr-var", 4.0f, "Preference distribution reward variance");
	float confidence = parser.parse<float>("confidence", 1.0f, "UCB confidence for planner (exploration/expoitation)");

	if (parser.enableHelp()) return 0;

	/////////////////   Transition System   /////////////////
	
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

	auto dfas = TP::FormalMethods::createDFAsFromFile(formula_filepath);

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

	std::shared_ptr<GridWorldTrueBehavior<N>> true_behavior = std::make_shared<GridWorldTrueBehavior<N>>(product, config_filepath);

	//if (verbose)
	//	true_behavior->print();

	// Make the preference behavior distribution
	PreferenceDistributionType p_ev;
	p_ev.mu(0) = pr_mean; // mean reward
	p_ev.mu(1) = pc_mean; // mean cost
	p_ev.Sigma(0, 0) = pr_var; // reward variance
	p_ev.Sigma(1, 1) = pc_var; // cost variance


	QuantifierSet<N> quantifier_set(p_ev);
	std::shared_ptr<Animator<N>> animator;
	if (animate)
		animator = std::make_shared<Animator<N>>(product, p_ev);

	for (uint32_t trial = 0; trial < n_trials; ++trial) {

		std::shared_ptr<BehaviorHandlerType> behavior_handler = std::make_shared<BehaviorHandlerType>(product, 1, confidence);
		Learner<N> prl(behavior_handler, n_efe_samples, animator, verbose);

		// Initialize the agent's state
		TP::DiscreteModel::State init_state = TP::DiscreteModel::GridWorldAgent::makeInitState(ts_props, ts);
		prl.initialize(init_state);

		auto samplerFunction = [&](TP::WideNode src_node, TP::WideNode dst_node, const TP::DiscreteModel::Action& action) {
			return true_behavior->sample(src_node, dst_node, action);
		};

		// Run the PRL
		auto quantifier = prl.run(p_ev, samplerFunction, max_planning_instances);
		if (verbose) {
			LOG("Finished!");
			std::string total_cost_str{};
			for (uint32_t i = 0; i < N; ++i) 
				total_cost_str += std::to_string(quantifier.cumulative_cost[i]) + ((i < N) ? ", " : "");
			PRINT_NAMED("Total Cost...............................", total_cost_str);
			PRINT_NAMED("Steps....................................", quantifier.steps);
			PRINT_NAMED("Instances................................", quantifier.instances);
			std::string avg_cost_str{};
			auto avg_cost_per_instance = quantifier.avgCostPerInstance();
			for (uint32_t i = 0; i < N; ++i) 
				avg_cost_str += std::to_string(avg_cost_per_instance[i]) + ((i < N) ? ", " : "");
			PRINT_NAMED("Average cost per per instance............", avg_cost_str);
		}

		//if (compare) 
		//	true_behavior->compare(*behavior_handler);

		quantifier_set.push_back(std::move(quantifier));

		if (animate) {
			TP::Serializer szr(animation_filepath);
			animator->serialize(szr, quantifier_set.back());
			szr.done();
		}

	}

	//if (benchmark) {
	//	TP::Serializer szr(benchmark_filepath);
	//	quantifier_set.serializeInstances(szr, "Cumulative Cost", "Cumulative Reward");
	//	szr.done();
	//}

	return 0;
}