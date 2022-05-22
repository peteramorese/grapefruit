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
    std::vector<std::string> labs_money = {"5", "10", "50", "100"};
    SS.setStateDimension(labs_location, 0);
    SS.setStateDimension(labs_money, 1);
    SS.setStateDimensionLabel(0, "location");
    SS.setStateDimensionLabel(1, "money");

    State s(&SS);
    std::vector<std::string> set_state = {"bank", "100"};
    s.setState(set_state);

	/* Propositions */
	SimpleCondition p_in_store;
	p_in_store.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "store");
	p_in_store.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_in_store.setLabel("p_in_store");

	SimpleCondition p_rich;
	p_rich.addCondition(Condition::SIMPLE, Condition::LABEL, "money", Condition::EQUALS, Condition::VAR, "50");
	p_rich.addCondition(Condition::SIMPLE, Condition::LABEL, "money", Condition::EQUALS, Condition::VAR, "100");
	p_rich.setCondJunctType(Condition::SIMPLE, Condition::DISJUNCTION);
	p_rich.setLabel("p_rich");

	SimpleCondition p_inside;
	p_inside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "store");
	p_inside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "bank");
	p_inside.setCondJunctType(Condition::SIMPLE, Condition::DISJUNCTION);
	p_inside.setLabel("p_inside");

	SimpleCondition p_outside;
	p_outside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "road");
	p_outside.addCondition(Condition::SIMPLE, Condition::LABEL, "location", Condition::EQUALS, Condition::VAR, "park");
	p_outside.setCondJunctType(Condition::SIMPLE, Condition::DISJUNCTION);
	p_outside.setLabel("p_outside");

	SimpleCondition p_benji;
	p_benji.addCondition(Condition::SIMPLE, Condition::LABEL, "money", Condition::EQUALS, Condition::VAR, "100");
	p_benji.setCondJunctType(Condition::SIMPLE, Condition::DISJUNCTION);
	p_benji.setLabel("p_benji");

	std::vector<SimpleCondition*> props = {&p_in_store, &p_rich, &p_inside, &p_outside, &p_benji};
	T.setPropositions(props);
    std::string logic_label = "p_in_store | ((p_rich & p_inside) & (p_benji | p_outside))";

	std::cout<<"Parsing Label..."<<std::endl;
    bool eval = T.parseLabelAndEval(&logic_label, &s);
	std::cout<<"Evaluation: "<<eval<<std::endl;

}