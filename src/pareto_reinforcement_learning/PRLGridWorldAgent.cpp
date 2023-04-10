#include<iostream>

#include "TaskPlanner.h"

#include "BehaviorHandler.h"
#include "PRL.h"

using namespace PRL;

struct TaskReward {
    void setToDefaultPrior() {dist.mean = 1.0f; dist.variance = 2.0f;}
    inline float getExpectation() const {return dist.mean;}
    inline float getVariance() const {return dist.variance;}

	TP::Distributions::Gaussian dist;
};

struct ActionCost {
    void setToDefaultPrior() {dist.mean = 1.0f; dist.variance = 2.0f;}
    inline float getExpectation() const {return dist.mean;}
    inline float getVariance() const {return dist.variance;}

	TP::Distributions::Gaussian dist;
};


int main(int argc, char* argv[]) {
 
	TP::ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v', "Run in verbose mode");

	std::string dfa_directory = parser.parse<std::string>("dfa-directory", "./dfas", "Directory that contains dfa files");
	std::string dfa_file_template = parser.parse<std::string>("dfa-file-template", "dfa_#.yaml", "Naming convention for dfa file");

	std::string config_filepath = parser.parse<std::string>("config-filepath", "", "Filepath to grid world config");

	bool write_plans = parser.hasFlag('w', "Write plans to plan files");
	std::string plan_directory = parser.parse<std::string>("plan-directory", "./grid_world_plans", "Directory to output plan files");
	std::string plan_file_template = parser.parse<std::string>("plan-file-template", "plan_#.yaml", "Naming convention for output plan files");

	uint32_t n_dfas = parser.parse<uint32_t>("n-dfas", 1, "Number of dfa files to read in");
	
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

	std::vector<std::shared_ptr<TP::FormalMethods::DFA>> dfas(n_dfas);

	TP::FormalMethods::Alphabet combined_alphbet;
	for (uint32_t i=0; i<n_dfas; ++i) {
		dfas[i] = std::make_shared<TP::FormalMethods::DFA>();
		std::string dfa_filepath = dfa_directory + "/" + TP::templateToLabel(dfa_file_template, i);
		if (verbose) LOG("Reading in dfa file: " << dfa_filepath);
		dfas[i]->deserialize(dfa_filepath);
		combined_alphbet = combined_alphbet + dfas[i]->getAlphabet();
		if (verbose) dfas[i]->print();
	}

	if (verbose) ts->listPropositions();

	ts->addAlphabet(combined_alphbet);

	/////////////////   Planner   /////////////////

	using EdgeInheritor = TP::DiscreteModel::ModelEdgeInheritor<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA>;
	using SymbolicGraph = TP::DiscreteModel::SymbolicProductAutomaton<TP::DiscreteModel::TransitionSystem, TP::FormalMethods::DFA, EdgeInheritor>;
	using BehaviorHandlerType = BehaviorHandler<SymbolicGraph, TaskReward, ActionCost>;
	using PreferenceDistributionType = TP::Distributions::FixedMultivariateGuassian<BehaviorHandlerType::numBehaviors()>;

	TP::Containers::SizedArray<TaskReward> rewards(dfas.size());
	for (auto& reward : rewards) {reward.setToDefaultPrior();}

 	std::shared_ptr<SymbolicGraph> product = std::make_shared<SymbolicGraph>(ts, dfas);
	std::shared_ptr<BehaviorHandlerType> behavior_handler = std::make_shared<BehaviorHandlerType>(product, rewards, 1);

	ParetoReinforcementLearner<BehaviorHandlerType> prl(behavior_handler);



	// Initialize the agent's state
	TP::DiscreteModel::State init_state = TP::DiscreteModel::GridWorldAgent::makeInitState(ts_props, ts);
	prl.initialize(init_state);

	// Input the preference behavior distribution
	PreferenceDistributionType p_ev;
	p_ev.mean(0) = 3.0f; // mean reward
	p_ev.mean(1) = 10.0f; // mean cost
	p_ev.covariance(0, 0) = 0.5f; // reward variance
	p_ev.covariance(1, 1) = 0.5f; // cost variance
	
	// Run the PRL
	prl.run(p_ev);

	return 0;
}