#include<iostream>

#include "Grapefruit.h"

#include "BehaviorHandler.h"
#include "Learner.h"
#include "TrueBehavior.h"
#include "RandomGridWorldGenerator.h"
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
 
	GF::ArgParser parser(argc, argv);

	bool verbose = parser.parse<void>('v', "Run in verbose mode").has();
	bool incl_dist_data = parser.parse<void>("dist-data", "Include the distribution data for each decision instance in the data file").has();
	bool incl_plan_data = parser.parse<void>("plan-data", "Include the plan data for each decision instance in the data file").has();
	//bool compare = parser.hasKey("compare", "Compare the learned estimates to the true estimates");

	auto formula_filepath = parser.parse<std::string>("formula-filepath", 'f', "formulas.yaml", "File that contains all formulas");
	auto random_config_filepath = parser.parse<std::string>("random-config-filepath", 'c', "Filepath to random grid world config");
	auto data_filepath = parser.parse<std::string>("data-filepath", 'd', "Specify a filepath to write the collected data to");
	auto selector_label = parser.parse<std::string>("selector", "aif", "Pareto point selector (aif (active inference), uniform, topsis, weights)");

	auto max_planning_instances = parser.parse<uint32_t>("instances", 'i', 10, "Max number of planning instances");
	auto n_trials = parser.parse<uint32_t>("trials", 't', 1, "Number of trials to run");
	auto n_efe_samples = parser.parse<uint32_t>("efe-samples", 300u, "Number of samples used for approximating the expected posterior entropy");
	auto starting_seed = parser.parse<uint32_t>("starting-seed", 's', 0, "Starting seed for random environment");

	auto confidence = parser.parse<float>("confidence", 1.0f, "UCB confidence for planner (exploration/expoitation)");

	parser.enableHelp();

	constexpr uint64_t N = 2;


	/////////////////   Planner   /////////////////

	using EdgeInheritor = GF::DiscreteModel::ModelEdgeInheritor<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA>;
	using SymbolicGraph = GF::DiscreteModel::SymbolicProductAutomaton<GF::DiscreteModel::TransitionSystem, GF::FormalMethods::DFA, EdgeInheritor>;
	using BehaviorHandlerType = BehaviorHandler<SymbolicGraph, N>;
	using PreferenceDistributionType = GF::Stats::Distributions::FixedMultivariateNormal<N>;

	// Make the preference behavior distribution
	PreferenceDistributionType p_ev = deserializePreferenceDist<N>(random_config_filepath.get());

	std::unique_ptr<GF::Serializer> szr_ptr;
	if (data_filepath.has()) {
		szr_ptr.reset(new GF::Serializer(data_filepath.get()));
		if (n_trials.get() > 1) {
			YAML::Emitter& out = szr_ptr->get();
			out << YAML::Key << "Trials" << YAML::Value << n_trials.get();
		}
	}
	
	GF::Deserializer dszr(formula_filepath.get());
	Selector selector = getSelector(selector_label.get());
	RandomGridWorldProperties props = RandomGridWorldGenerator<N>::deserializeConfig(random_config_filepath.get());

	auto dfas = GF::FormalMethods::createDFAsFromFile(dszr);
	ASSERT(dfas.size() == 2, "Did not receive two formulas (liveness & safety), check your formulas file");
	GF::Node init_cs_state = dfas[0]->getInitStates()[0];
	GF::Node acc_cs_state = *dfas[0]->getAcceptingStates().begin();
	dfas[0]->disconnect(acc_cs_state, acc_cs_state, "1");
	dfas[0]->connect(acc_cs_state, init_cs_state, "1");


	uint32_t trial = 0;
	uint32_t seed = 10 + starting_seed.get();
	while (trial < n_trials.get()) {

		// For easy to read log files
		std::cout << "Trial: " << trial + 1 << "/" << n_trials.get() << "\n";

		//PRINT_NAMED("Seed", trial + 20);
		GF::RNG::seed(seed);
		auto targets = RandomGridWorldGenerator<N>::generate(props, dfas, confidence.get());
		GF::RNG::seed(GF::RNG::randiUnbounded());
		
		std::shared_ptr<Regret<SymbolicGraph, N>> regret_handler = std::make_shared<Regret<SymbolicGraph, N>>(targets.product, targets.true_behavior);
		std::shared_ptr<DataCollector<N>> data_collector = std::make_shared<DataCollector<N>>(targets.product, p_ev, regret_handler);


		Learner<N> prl(targets.behavior_handler, data_collector, n_efe_samples.get(), verbose);

		// Initialize the agent's state
		GF::DiscreteModel::State init_state = GF::DiscreteModel::GridWorldAgent::makeInitState(targets.props, targets.ts);
		prl.initialize(init_state);

		auto samplerFunction = [&](GF::WideNode src_node, GF::WideNode dst_node, const GF::DiscreteModel::Action& action) {
			return targets.true_behavior->sample(src_node, dst_node, action);
		};

		// Run the PRL
		bool success = prl.run(p_ev, samplerFunction, max_planning_instances.get(), selector);

		if (!success) {
			++seed;
			continue;
		}

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


		if (data_filepath.has()) {
			if (n_trials.get() > 1) {
				YAML::Emitter& out = szr_ptr->get();
				out << YAML::Key << "Trial " + std::to_string(trial) << YAML::Value << YAML::BeginMap;
				data_collector->serialize(*szr_ptr, incl_dist_data, incl_plan_data);
				out << YAML::EndMap;
			} else {
				data_collector->serialize(*szr_ptr, incl_dist_data, incl_plan_data);
			}
		}

		//LOG("Done");
		//PAUSE;

		++trial;
		++seed;
	}
	
	if (szr_ptr)
		szr_ptr->done();


	return 0;
}
