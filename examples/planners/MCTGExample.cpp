
#include "tools/Logging.h"

#include "core/Condition.h"
#include "core/StateSpace.h"
#include "core/State.h"
#include "core/TransitionSystem.h"
#include "core/Automaton.h"
#include "core/SymbolicProductAutomaton.h"

#include "planners/heuristic/DecentralizedMCTG.h"

#include "models/GridWorldAgent.h"

using namespace TP;
using namespace TP::Planner;

int main() {
 
	DiscreteModel::GridWorldAgentProperties ts_props;
	ts_props.n_x = 5;
	ts_props.n_y = 5;
	ts_props.init_coordinate_x = 0;
	ts_props.init_coordinate_y = 0;

	std::shared_ptr<DiscreteModel::TransitionSystem> ts = DiscreteModel::GridWorldAgent::generate(ts_props);

	ts->print();
	//NEW_LINE;
	//ts->listPropositions();

	/////////////////   DFAs   /////////////////

	std::shared_ptr<FormalMethods::DFA> dfa = std::make_shared<FormalMethods::DFA>();
	dfa->setAcceptingStates({0});
	dfa->setInitStates({2});
	dfa->setAlphabet({"!x_4_y_0", "x_4_y_0 & !x_2_y_4", "x_4_y_0 & x_2_y_4", "!x_2_y_4", "x_2_y_4", "1"});
	dfa->connect(0, 0, "1");
	dfa->connect(1, 0, "x_2_y_4");
	dfa->connect(1, 1, "!x_2_y_4");
	dfa->connect(2, 1, "x_4_y_0 & !x_2_y_4");
	dfa->connect(2, 0, "x_4_y_0 & x_2_y_4");
	dfa->connect(2, 2, "!x_4_y_0");

	ts->addAlphabet(dfa->getAlphabet());

    auto mctg = MinCostToGo<DiscreteModel::TransitionSystem, FormalMethods::DFA, DiscreteModel::ModelEdgeInheritor<DiscreteModel::TransitionSystem, FormalMethods::DFA>>::search(ts, dfa);

    DiscreteModel::SymbolicProductAutomaton<DiscreteModel::TransitionSystem, FormalMethods::DFA, DiscreteModel::ModelEdgeInheritor<DiscreteModel::TransitionSystem, FormalMethods::DFA>> product(ts, {dfa});
	NEW_LINE;
	LOG("Min cost to go:");
	for (auto v_t : *mctg) {
		auto unwrapped_n = product.getUnwrappedNode(v_t.first);
		LOG("ts: " << unwrapped_n.ts_node << " dfa1: " << unwrapped_n.automata_nodes[0] << " cost: " << v_t.second);
	}
	NEW_LINE;
    
}