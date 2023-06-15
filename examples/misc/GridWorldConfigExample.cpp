#include<iostream>

#include "TaskPlanner.h"


using namespace TP;
using namespace TP::Planner;

int main(int argc, char* argv[]) {
	ArgParser parser(argc, argv);
 
	Argument<std::string> config_filepath = parser.parse<std::string>("config-filepath", "Filepath to grid world config");

	parser.enableHelp();

	DiscreteModel::GridWorldAgentProperties ts_props = DiscreteModel::GridWorldAgent::deserializeConfig(config_filepath.get());

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::GridWorldAgent::generate(ts_props);


	ts->print();
	NEW_LINE;
	ts->listPropositions();

	return 0;
}
