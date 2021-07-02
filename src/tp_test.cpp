#include<vector>
#include "edge.h"
#include "astar.h"
#include "state.h"
#include "condition.h"
#include "transitionSystem.h"

int main() {

	/* CREATE ENVIRONMENT */
	StateSpace SS_LANDER;

	std::vector<std::string> ee_labels = {"stow","basket", "container", "L1", "L2"};
	std::vector<std::string> rock_labels = {"basket","ee","L1", "L2"};
	std::vector<std::string> alien_labels = {"container","ee","L1", "L2"};
	std::vector<std::string> grip_labels = {"true","false"};
	
	// Create state space:
	SS_LANDER.setStateDimension(ee_labels, 0); // eef
	SS_LANDER.setStateDimension(rock_labels, 1); // rock
	SS_LANDER.setStateDimension(alien_labels, 2); // alien
	SS_LANDER.setStateDimension(grip_labels, 3); // eef engaged

	// Label state space:
	SS_LANDER.setStateDimensionLabel(0, "eeLoc");
	SS_LANDER.setStateDimensionLabel(1, "rock_loc");
	SS_LANDER.setStateDimensionLabel(2, "alien_loc");
	SS_LANDER.setStateDimensionLabel(3, "holding");

	// Create object location group:
	std::vector<std::string> obj_group = {"rock_loc", "alien_loc"};
	SS_LANDER.setLabelGroup("object locations", obj_group);

	// Set the initial state:
	std::vector<std::string> set_state = {"stow", "L1", "L2", "false"};
	//std::vector<std::string> test_set_state = {"L1", "L1", "L2", "false"};
	State init_state(&SS_LANDER);	
	init_state.setState(set_state);

	//State test_state(&SS_LANDER);	
	//test_state.setState(test_set_state);

	/* SET CONDITIONS */
	// Pickup domain conditions:
	std::vector<Condition> conds;
	std::vector<Condition*> cond_ptrs;
	conds.resize(4);
	cond_ptrs.resize(4);

	// Grasp 
	conds[0].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds[0].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc",Condition::TRUE, "arg");
	conds[0].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds[0].addCondition(Condition::POST, Condition::ARG_L, Condition::FILLER, Condition::ARG_EQUALS, Condition::VAR, "ee",Condition::TRUE, "arg");
	conds[0].addCondition(Condition::POST, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds[0].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[0].setActionLabel("grasp");
	
	// Transport 
	conds[1].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds[1].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg1");
	conds[1].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg2");
	conds[1].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds[1].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg2"); // Stored eeLoc pre-state variable is not the same as post-state eeLoc (eeLoc has moved)
	conds[1].addCondition(Condition::POST, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE,"na");
	conds[1].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[1].setActionLabel("transport");
	//conds[1].print();

	// Release 
	conds[2].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds[2].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg1");
	conds[2].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::VAR, "ee",Condition::TRUE, "arg2");
	conds[2].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds[2].addCondition(Condition::POST, Condition::ARG_L, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::TRUE, "arg2");
	conds[2].addCondition(Condition::POST, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds[2].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[2].setActionLabel("release");
	//conds[2].print();


	// Transit
	conds[3].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds[3].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg");
	conds[3].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds[3].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE,"arg");
	conds[3].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[3].setActionLabel("transit");
	//conds[3].print();

	
	for (int i=0; i<conds.size(); ++i){
		cond_ptrs[i] = &conds[i];
	}


	/* Propositions */
	SimpleCondition p1;
	p1.addCondition(Condition::SIMPLE, Condition::LABEL, "rock_loc", Condition::EQUALS, Condition::VAR, "basket");
	p1.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p1.setLabel("p_r");

	/* Propositions */
	SimpleCondition p2;
	p2.addCondition(Condition::SIMPLE, Condition::LABEL, "alien_loc", Condition::EQUALS, Condition::VAR, "container");
	p2.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p2.setLabel("p_a");


	/* DFA & Graph Instantiations */
	Edge TS(true);
	Edge DFA(true);
	Edge PS(true);
	
	//bool didwork = conds[3].evaluate(&init_state, &test_state);
	//std::cout<<"is true??: "<<didwork<<std::endl;

	// Hard-code DFA automaton:
	std::cout<<"made it here"<<std::endl;
	DFA.connect(2, 0, 1.0, "p_a & p_r");
	DFA.connect(2, 2, 1.0, "!p_a & !p_r");
	DFA.connect(2, 1, 1.0, "p_a & !p_r");
	DFA.connect(2, 3, 1.0, "!p_a & p_r");
	DFA.connect(1, 1, 1.0, "!p_r");
	DFA.connect(1, 0, 1.0, "p_r");
	DFA.connect(3, 3, 1.0, "!p_a");
	DFA.connect(3, 0, 1.0, "p_a");
	DFA.print();

	ProductSystem<State> PRODSYS(&TS, &DFA, &PS);
	//TransitionSystem<State> test(&TS);
	PRODSYS.setConditions(cond_ptrs);
	PRODSYS.setInitState(&init_state);
	PRODSYS.generate();
	//PRODSYS.print();
	
	PRODSYS.addProposition(&p1);
	PRODSYS.addProposition(&p2);
	PRODSYS.setAutomatonInitStateIndex(2);
	PRODSYS.addAutomatonAcceptingStateIndex(0);
	std::cout<<"b4 compose"<<std::endl;
	PRODSYS.compose();
	std::cout<<"af compose"<<std::endl;
	PRODSYS.print();

	std::vector<int> plan;
	float pathlength;
	PRODSYS.plan(plan);

	std::cout<<"\nPath: ";
	for (int i=0; i<plan.size(); ++i) {
		std::cout<<" -> "<<plan[i];	
	}
	std::cout<<"\n";










	return 0;
}
