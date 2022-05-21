#include "graph.h"
#include "condition.h"
#include "transitionSystem.h"
#include "stateSpace.h"
#include "state.h"



int main(){
    Graph<WL> G(true);
    TransitionSystem<State> T(&G);
    StateSpace SS;
    std::vector<std::string> labs_location = {"store", "bank", "road", "park"};
    std::vector<std::string> labs_money = {"5", "10", "50"};
    SS.setStateDimension(labs_location, 0);
    SS.setStateDimension(labs_money, 1);
    SS.setStateDimensionLabel(0, "location");
    SS.setStateDimensionLabel(1, "money");

    State s(&SS);
    std::vector<std::string> set_state = {"store", "50"};
    s.setState(set_state);

	/* Propositions */
	SimpleCondition p_in_store;
	p_in_store.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "store");
	p_in_store.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_in_store.setLabel("p_in_store");

	/* Propositions */
	SimpleCondition p_rich;
	p_rich.addCondition(Condition::SIMPLE, Condition::LABEL, "money", Condition::EQUALS, Condition::VAR, "50");
	p_rich.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_rich.setLabel("p_rich");

	/* Propositions */
	SimpleCondition p_inside;
	p_inside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "store");
	p_inside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "bank");
	p_inside.setCondJunctType(Condition::SIMPLE, Condition::DISJUNCTION);
	p_inside.setLabel("p_inside");

	/* Propositions */
	SimpleCondition p_outside;
	p_outside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "road");
	p_outside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "park");
	p_outside.setCondJunctType(Condition::SIMPLE, Condition::DISJUNCTION);
	p_outside.setLabel("p_inside");

    std::string logic_label = "p_in_store | (p_rich & p_inside)";

    T.parseLabelAndEval(&logic_label, &s);

}