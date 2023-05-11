#include<iostream>

#include "TaskPlanner.h"

#include "BehaviorHandler.h"
#include "PRL.h"
#include "TrueBehavior.h"
#include "Benchmark.h"

using namespace PRL;

int main(int argc, char* argv[]) {
 
	TP::ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v', "Run in verbose mode");
	bool benchmark = parser.hasFlag('b', "Benchmark run");
	bool write_plans = parser.hasFlag('w', "Write plans to plan files");
	bool compare = parser.hasKey("compare", "Compare the learned estimates to the true estimates");

	std::string formula_filepath = parser.parse<std::string>("formula-filepath", "formulas.yaml", "File that contains all formulas");
	std::string config_filepath = parser.parse<std::string>("config-filepath", "", "Filepath to grid world config");
	std::string plan_directory = parser.parse<std::string>("plan-directory", "./grid_world_plans", "Directory to output plan files");
	std::string benchmark_filepath = parser.parse<std::string>("bm-filepath", "prl_grid_world_bm.yaml", "File that benchmark data will be written to");
	std::string plan_file_template = parser.parse<std::string>("plan-file-template", "plan_#.yaml", "Naming convention for output plan files");

	uint32_t max_planning_instances = parser.parse<uint32_t>("instances", 10, "Max number of planning instances");
	uint32_t n_trials = parser.parse<uint32_t>("trials", 1, "Number of trials to run");

	float pc_mean = parser.parse<float>("pc-mean", 50.0f, "Preference distribution cost mean");
	float pc_var = parser.parse<float>("pc-var", 25.0f, "Preference distribution cost variance");
	float pr_mean = parser.parse<float>("pr-mean", 10.0f, "Preference distribution reward mean");
	float pr_var = parser.parse<float>("pr-var", 4.0f, "Preference distribution reward variance");

	if (parser.enableHelp()) return 0;

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

	using EdgeInheritor = TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>;
	using SymbolicGraph = TP::DiscreteModel::SymbolicProductAutomaton<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA, EdgeInheritor>;
	using BehaviorHandlerType = BehaviorHandler<SymbolicGraph, 1>;
	using TrueBehaviorType = TrueBehavior<SymbolicGraph, 1>;
	using PreferenceDistributionType = TP::Stats::Distributions::FixedMultivariateNormal<BehaviorHandlerType::numBehaviors()>;

	TP::Stats::Distributions::Normal default_reward;
	default_reward.mu = 10.0f;
	default_reward.sigma_2 = 3.0f;
	typename TrueBehaviorType::CostDistributionArray default_cost_array;
	default_cost_array[0].mu = 5.0f;
	default_cost_array[0].sigma_2 = 0.5f;


 	std::shared_ptr<SymbolicGraph> product = std::make_shared<SymbolicGraph>(ts, dfas);
	
	/////////////////   True Behavior   /////////////////
	std::shared_ptr<TrueBehaviorType> true_behavior = std::make_shared<TrueBehaviorType>(product, dfas.size(), default_reward, default_cost_array);
	std::vector<std::string> x_labels(ts_props.n_x);
	std::vector<std::string> y_labels(ts_props.n_y);
	for (int i=0; i<ts_props.n_x; ++i) {
		x_labels[i] = "x" + std::to_string(i);
	}
	for (int i=0; i<ts_props.n_y; ++i) {
		y_labels[i] = "y" + std::to_string(i);
	}
	auto ss_grid_agent = ts->getStateSpace().lock();
	for (const auto& region : ts_props.environment.regions) {
		for (uint32_t i = region.lower_left_x; i <= region.upper_right_x; ++i) {
			for (uint32_t j = region.lower_left_y; j <= region.upper_right_y; ++j) {
				TP::DiscreteModel::State s(ss_grid_agent.get());	
				s[TP::DiscreteModel::GridWorldAgent::s_x_coord_label] = x_labels[i];
				s[TP::DiscreteModel::GridWorldAgent::s_y_coord_label] = y_labels[j];
				TP::Node model_node = ts->getGenericNodeContainer()[s];
				for (const auto& outgoing_edge : ts->getOutgoingEdges(model_node)) {
					TrueBehaviorType::CostDistributionArray& cost_array = true_behavior->getNAPElement(model_node, outgoing_edge.action);
					cost_array[0].convolveWith(TP::Stats::Distributions::Normal(region.exit_cost, 0.3f*region.exit_cost + 0.2f));
					TrueBehaviorType::CostDistributionArray& cost_array_check = true_behavior->getNAPElement(model_node, outgoing_edge.action);
				}
			}
		}
	}

	if (verbose)
		true_behavior->print();

	// Make the preference behavior distribution
	PreferenceDistributionType p_ev;
	p_ev.mu(0) = pr_mean; // mean reward
	p_ev.mu(1) = pc_mean; // mean cost
	p_ev.covariance(0, 0) = pr_var; // reward variance
	p_ev.covariance(1, 1) = pc_var; // cost variance


	QuantifierSet<1> quantifier_set(p_ev);

	for (uint32_t trial = 0; trial < n_trials; ++trial) {
		if (!write_plans)
			plan_directory = std::string();

		std::shared_ptr<BehaviorHandlerType> behavior_handler = std::make_shared<BehaviorHandlerType>(product, 1, 1.0f);
		ParetoReinforcementLearner<BehaviorHandlerType> prl(behavior_handler, plan_directory);

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
			PRINT_NAMED("Total Reward.............................", quantifier.cumulative_reward);
			PRINT_NAMED("Total Cost...............................", quantifier.cumulative_cost[0]);
			PRINT_NAMED("Steps....................................", quantifier.steps);
			PRINT_NAMED("Decision Instances.......................", quantifier.decision_instances);
			PRINT_NAMED("Average reward per decision instance.....", quantifier.avgRewardPerDI());
			PRINT_NAMED("Average cost per per decision instance...", quantifier.avgCostPerDI());
			PRINT_NAMED("Average reward per step..................", quantifier.cumulative_reward / static_cast<float>(quantifier.steps));
			PRINT_NAMED("Average cost per step....................", quantifier.cumulative_cost[0] / static_cast<float>(quantifier.steps));
		}

		if (compare) 
			true_behavior->compare(*behavior_handler);

		quantifier_set.push_back(std::move(quantifier));
	}

	if (benchmark) {
		TP::Serializer szr(benchmark_filepath);
		quantifier_set.serialize(szr, "Cumulative Cost", "Cumulative Reward");
		szr.done();
	}

	return 0;
}