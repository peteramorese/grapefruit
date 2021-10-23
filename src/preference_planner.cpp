#include<iostream>
#include "graph.h"
#include "condition.h"
#include "transitionSystem.h"
#include "stateSpace.h"
#include "state.h"
#include "symbSearch.h"

int main() {


	/////////////////////////////////////////////////
	/* Create the Transition System for the Camera */
	/////////////////////////////////////////////////
	


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
	conds_c[0].setActionCost(50);

	// Turn Off
	conds_c[1].addCondition(Condition::PRE, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "on");
	conds_c[1].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_c[1].addCondition(Condition::POST, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "off");
	conds_c[1].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[1].setActionLabel("power_off");
	conds_c[1].setActionCost(5);

	// Pan Center 
	conds_c[2].addCondition(Condition::PRE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[2].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[2].addCondition(Condition::POST, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[2].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[2].setActionLabel("pan_center");
	conds_c[2].setActionCost(25);
	
	// Pan Off Center 
	conds_c[3].addCondition(Condition::PRE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[3].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[3].addCondition(Condition::POST, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[3].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[3].setActionLabel("pan_off_center");
	conds_c[3].setActionCost(25);

	// Tilt Center 
	conds_c[4].addCondition(Condition::PRE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[4].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[4].addCondition(Condition::POST, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[4].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[4].setActionLabel("tilt_center");
	conds_c[4].setActionCost(25);
	
	// Tilt Off Center 
	conds_c[5].addCondition(Condition::PRE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::TRUE, "center");
	conds_c[5].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_c[5].addCondition(Condition::POST, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center", Condition::NEGATE, "not_center");
	conds_c[5].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_c[5].setActionLabel("tilt_off_center");
	conds_c[5].setActionCost(25);

	
	for (int i=0; i<conds_c.size(); ++i){
		cond_ptrs_c[i] = &conds_c[i];
	}


	/* Propositions */
	int N_APs = 9;
	std::vector<SimpleCondition> APs(N_APs);
	APs[0].addCondition(Condition::SIMPLE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "left");
	APs[0].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[0].setLabel("facing_left");

	/* Propositions */
	APs[1].addCondition(Condition::SIMPLE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "right");
	APs[1].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[1].setLabel("facing_right");

	/* Propositions */
	APs[2].addCondition(Condition::SIMPLE, Condition::LABEL, "pan", Condition::EQUALS, Condition::VAR, "center");
	APs[2].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[2].setLabel("facing_middle");

	/* Propositions */
	APs[3].addCondition(Condition::SIMPLE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "down");
	APs[3].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[3].setLabel("facing_down");
	
	/* Propositions */
	APs[4].addCondition(Condition::SIMPLE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "up");
	APs[4].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[4].setLabel("facing_up");

	/* Propositions */
	APs[5].addCondition(Condition::SIMPLE, Condition::LABEL, "tilt", Condition::EQUALS, Condition::VAR, "center");
	APs[5].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[5].setLabel("facing_forward");

	/* Propositions */
	APs[6].addCondition(Condition::SIMPLE, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "on");
	APs[6].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[6].setLabel("powered_on");

	/* Propositions */
	APs[7].addCondition(Condition::SIMPLE, Condition::LABEL, "power", Condition::EQUALS, Condition::VAR, "off");
	APs[7].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	APs[7].setLabel("powered_off");

	std::vector<SimpleCondition*> AP_ptrs_c(N_APs);
	for (int i=0; i<AP_ptrs_c.size(); ++i){
		AP_ptrs_c[i] = &APs[i];
	}


	// Create the transition system:
	Graph<WL> ts_graph_c(true);
	TS_EVAL<State> ts_eval(&ts_graph_c, 0); // by default, the init node for the ts is 0
	ts_eval.setConditions(cond_ptrs_c);
	ts_eval.setPropositions(AP_ptrs_c);
	ts_eval.setInitState(&init_state_c);
	ts_eval.generate();
	std::cout<<"\n\n Printing the Transition System: \n\n"<<std::endl;
	ts_eval.printTS();

	

	/////////////////////////////////////////////////
	/*       Read in the DFA's from the files      */
	/////////////////////////////////////////////////
	
	DFA A;
	int N_DFAs;
	
	// Get input from user for how many formulas to read in:
	std::cout<<"Enter number of formulas: "<<std::endl;
	std::cin >> N_DFAs;

	std::vector<DFA> dfa_arr(N_DFAs);
	std::vector<std::string> filenames(N_DFAs);
	for (int i=0; i<N_DFAs; ++i) {
		filenames[i] = "../spot_automaton_file_dump/dfas/dfa_" + std::to_string(i) +".txt";
	}
	for (int i=0; i<N_DFAs; ++i) {
		dfa_arr[i].readFileSingle(filenames[i]);
	}
	std::cout<<"\n\nPrinting all DFA's (read into an array)...\n\n"<<std::endl;
	for (int i=0; i<N_DFAs; ++i) {
		dfa_arr[i].print();
		std::cout<<"\n"<<std::endl;
	}



	/////////////////////////////////////////////////
	/*        Construct the Symbolic Search        */
	/////////////////////////////////////////////////
	
	// First construct the graph evaluation objects:
	//TS_EVAL<State> ts_eval(&ts, 0); 
	//std::vector<DFA_EVAL> dfa_eval_vec;
	std::vector<DFA_EVAL*> dfa_eval_ptrs;
	for (int i=0; i<N_DFAs; ++i) {
		DFA_EVAL* temp_dfa_eval_ptr = new DFA_EVAL(&dfa_arr[i]);
		dfa_eval_ptrs.push_back(temp_dfa_eval_ptr);
	}
	
	SymbSearch search_obj;
	search_obj.setAutomataPrefs(&dfa_eval_ptrs);
	search_obj.setTransitionSystem(&ts_eval);
	search_obj.setFlexibilityParam(75.0f);
	bool success = search_obj.search();
	std::cout<<"Found plan? "<<success<<std::endl;

	for (int i=0; i<dfa_eval_ptrs.size(); ++i) {
		delete dfa_eval_ptrs[i];
	}




}
