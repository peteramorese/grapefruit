#include "graph.h"
#include "condition.h"
#include "transitionSystem.h"
#include "stateSpace.h"
#include "state.h"

int main() {
	/* CREATE ENVIRONMENT FOR CAMERA */
	StateSpace SS_CAMERA;

	std::vector<std::string> pan_labels = {"left","center","right"};
	std::vector<std::string> tilt_labels = {"up","center","down"};
	std::vector<std::string> power_labels = {"on","off"};
	
	// Create state space:
	SS_CAMERA.setStateDimension(pan_labels, 0); // pan
	SS_CAMERA.setStateDimension(tilt_labels, 1); // tilt
	SS_CAMERA.setStateDimension(power_labels, 2); // power

	// Label state space:
	SS_CAMERA.setStateDimensionLabel(0, "pan");
	SS_CAMERA.setStateDimensionLabel(1, "tilt");
	SS_CAMERA.setStateDimensionLabel(2, "power");

	// Create object location group:
	std::vector<std::string> point_group = {"pan", "tilt"};
	SS_CAMERA.setLabelGroup("pointing locations", point_group);

	// Set the initial state:
	std::vector<std::string> set_state_c = {"center", "center", "off"};
	State init_state_c(&SS_CAMERA);	
	init_state_c.setState(set_state_c);

	//State test_state(&SS_MANIPULATOR);	
	//test_state.setState(test_set_state);

	/* SET CONDITIONS */
	// Pickup domain conditions:
	std::vector<Condition> conds_c;
	std::vector<Condition*> cond_ptrs_c;
	conds_c.resize(6);
	cond_ptrs_c.resize(6);

	// Turn On
	conds_c[0].addCondition(Condition::PRE, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "off");
	conds_c[0].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_c[0].addCondition(Condition::POST, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "on");
	conds_c[0].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[0].setActionLabel("power_on");

	// Turn Off
	conds_c[1].addCondition(Condition::PRE, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "on");
	conds_c[1].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_c[1].addCondition(Condition::POST, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "off");
	conds_c[1].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[1].setActionLabel("power_on");

	// Pan Center 
	conds_c[2].addCondition(Condition::PRE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[2].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[2].addCondition(Condition::POST, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[2].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[2].setActionLabel("pan_center");
	
	// Pan Off Center 
	conds_c[3].addCondition(Condition::PRE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[3].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[3].addCondition(Condition::POST, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[3].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[3].setActionLabel("pan_off_center");

	// Tilt Center 
	conds_c[4].addCondition(Condition::PRE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[4].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[4].addCondition(Condition::POST, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[4].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[4].setActionLabel("tilt_center");
	
	// Tilt Off Center 
	conds_c[5].addCondition(Condition::PRE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[5].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[5].addCondition(Condition::POST, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[5].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[5].setActionLabel("tilt_off_center");

	
	for (int i=0; i<conds_c.size(); ++i){
		cond_ptrs_c[i] = &conds_c[i];
	}


	/* Propositions */
	SimpleCondition p_p;
	p_p.addCondition(Condition::SIMPLE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "left");
	p_p.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_p.setLabel("p_p");

	/* Propositions */
	SimpleCondition p_t;
	p_t.addCondition(Condition::SIMPLE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "down");
	p_t.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_t.setLabel("p_t");
	
	/* Propositions */
	SimpleCondition p_on;
	p_on.addCondition(Condition::SIMPLE, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "on");
	p_on.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_on.setLabel("p_on");



	/* DFA_m & Graph Instantiations */
	//Edge DFA_c(true);
	//Edge PS_c(true);
	
	//bool didwork = conds_m[3].evaluate(&init_state, &test_state);
	//std::cout<<"is true??: "<<didwork<<std::endl;

	// Hard-code DFA_m automaton:
	//DFA_c.connect(1, 0, 1.0, "p_on & p_p & p_t");
	//DFA_c.connect(1, 1, 1.0, "!p_on | !p_p | !p_t");
	/*
	DFA_c.connect(2, 1, 1.0, "p_a & !p_r");
	DFA_c.connect(2, 3, 1.0, "!p_a & p_r");
	DFA_c.connect(1, 1, 1.0, "!p_r");
	DFA_c.connect(1, 0, 1.0, "p_r");
	DFA_c.connect(3, 3, 1.0, "!p_a");
	DFA_c.connect(3, 0, 1.0, "p_a");
	*/
	//DFA_c.print();

	//TransitionSystem<State> TS(&TS_c);
	//TransitionSystem<State> test(&TS_c);
	//PRODSYS_c.setConditions(cond_ptrs_c);
	//PRODSYS_c.setInitState(&init_state_c);
	//PRODSYS_c.generate();

	Graph<WL> TS_c(true);
	TransitionSystem<State> ts(&TS_c);
	ts.setConditions(cond_ptrs_c);
	ts.setInitState(&init_state_c);
	ts.generate();
	ts.print();

	
	//PRODSYS_c.addProposition(&p_p);
	//PRODSYS_c.addProposition(&p_t);
	//PRODSYS_c.addProposition(&p_on);
	//PRODSYS_c.setAutomatonInitStateIndex(1);
	//PRODSYS_c.addAutomatonAcceptingStateIndex(0);
	//PRODSYS_c.compose();
	//PRODSYS_c.print();

}
