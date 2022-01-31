#include<iostream>
#include "graph.h"
#include "condition.h"
#include "transitionSystem.h"
#include "stateSpace.h"
#include "state.h"
#include "symbSearch.h"

int main() {


	//////////////////////////////////////////////////////
	/* Create the Transition System for the Manipualtor */
	//////////////////////////////////////////////////////

	/* CREATE ENVIRONMENT FOR MANIPULATOR */
	StateSpace SS_MANIPULATOR;

	std::vector<std::string> loc_labels = {"L0", "L1", "L2", "L3", "L4"};	
	std::vector<std::string> ee_labels = loc_labels;
	ee_labels.push_back("stow");
	std::vector<std::string> obj_labels = loc_labels;
	obj_labels.push_back("ee");
	std::vector<std::string> grip_labels = {"true","false"};

	// Create state space:
	SS_MANIPULATOR.setStateDimension(ee_labels, 0); // eef
	SS_MANIPULATOR.setStateDimension(obj_labels, 1); // rock
	SS_MANIPULATOR.setStateDimension(obj_labels, 2); // alien
	SS_MANIPULATOR.setStateDimension(grip_labels, 3); // eef engaged

	// Label state space:
	SS_MANIPULATOR.setStateDimensionLabel(0, "eeLoc");
	SS_MANIPULATOR.setStateDimensionLabel(1, "rock");
	SS_MANIPULATOR.setStateDimensionLabel(2, "alien");
	SS_MANIPULATOR.setStateDimensionLabel(3, "holding");

	// Create object location group:
	std::vector<std::string> obj_group = {"rock", "alien"};
	SS_MANIPULATOR.setLabelGroup("object locations", obj_group);

	// Set the initial state:
	std::vector<std::string> set_state = {"stow", "L0", "L1", "false"};
	//std::vector<std::string> test_set_state = {"L1", "L1", "L2", "false"};
	State init_state(&SS_MANIPULATOR);	
	init_state.setState(set_state);

	//State test_state(&SS_MANIPULATOR);	
	//test_state.setState(test_set_state);

	/* SET CONDITIONS */
	// Pickup domain conditions:
	std::vector<Condition> conds_m;
	std::vector<Condition*> cond_ptrs_m;
	conds_m.resize(4);
	cond_ptrs_m.resize(4);

	// Grasp 
	conds_m[0].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds_m[0].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc",Condition::TRUE, "arg");
	conds_m[0].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_m[0].addCondition(Condition::POST, Condition::ARG_L, Condition::FILLER, Condition::ARG_EQUALS, Condition::VAR, "ee",Condition::TRUE, "arg");
	conds_m[0].addCondition(Condition::POST, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds_m[0].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_m[0].setActionLabel("grasp");
	conds_m[0].setActionCost(0);

	// Transport 
	conds_m[1].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds_m[1].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg1");
	conds_m[1].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg2");
	conds_m[1].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_m[1].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg2"); // Stored eeLoc pre-state variable is not the same as post-state eeLoc (eeLoc has moved)
	conds_m[1].addCondition(Condition::POST, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE,"na");
	conds_m[1].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_m[1].setActionLabel("transport");
	conds_m[1].setActionCost(5);
	//conds_m[1].print();

	// Release 
	conds_m[2].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds_m[2].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg1");
	conds_m[2].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::VAR, "ee",Condition::TRUE, "arg2");
	conds_m[2].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_m[2].addCondition(Condition::POST, Condition::ARG_L, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::TRUE, "arg2");
	conds_m[2].addCondition(Condition::POST, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds_m[2].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_m[2].setActionLabel("release");
	conds_m[2].setActionCost(0);
	//conds_m[2].print();


	// Transit
	conds_m[3].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds_m[3].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg");
	conds_m[3].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_m[3].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE,"arg");
	conds_m[3].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_m[3].setActionLabel("transit");
	conds_m[3].setActionCost(0);
	//conds_m[3].print();


	for (int i=0; i<conds_m.size(); ++i){
		cond_ptrs_m[i] = &conds_m[i];
	}


	/* Propositions */
	std::cout<<"Setting Atomic Propositions... "<<std::endl;
	std::vector<SimpleCondition> AP_m(loc_labels.size()*2);
	std::vector<SimpleCondition*> AP_ptrs_m(loc_labels.size()*2);
	for (int i=0; i<loc_labels.size(); ++i) {
		AP_m[2*i].addCondition(Condition::SIMPLE, Condition::LABEL, "rock", Condition::EQUALS, Condition::VAR, loc_labels[i]);
		AP_m[2*i].addCondition(Condition::SIMPLE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
		AP_m[2*i].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
		AP_m[2*i].setLabel("r" + loc_labels[i]);

		AP_m[2*i + 1].addCondition(Condition::SIMPLE, Condition::LABEL, "alien", Condition::EQUALS, Condition::VAR, loc_labels[i]);
		AP_m[2*i + 1].addCondition(Condition::SIMPLE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
		AP_m[2*i + 1].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
		AP_m[2*i + 1].setLabel("a" + loc_labels[i]);
	}
	for (int i=0; i<AP_m.size(); ++i) {
		AP_ptrs_m[i] = &AP_m[i];
	}


	// Create the transition system:
	Graph<WL> ts_graph_m(true);
	TS_EVAL<State> ts_eval(&ts_graph_m, 0); // by default, the init node for the ts is 0
	ts_eval.setConditions(cond_ptrs_m);
	ts_eval.setPropositions(AP_ptrs_m);
	ts_eval.setInitState(&init_state);
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

	SymbSearch<DetourLex> search_obj;
	search_obj.setAutomataPrefs(&dfa_eval_ptrs);
	search_obj.setTransitionSystem(&ts_eval);
	search_obj.setFlexibilityParam(1000.0f);
	//search_obj.setFlexibilityParam(0.0f);
	bool success = search_obj.search();
	//std::cout<<"Found plan? "<<success<<std::endl;

	for (int i=0; i<dfa_eval_ptrs.size(); ++i) {
		delete dfa_eval_ptrs[i];
	}

	return 0;



}
