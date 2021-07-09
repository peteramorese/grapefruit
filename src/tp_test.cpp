#include<vector>
#include "edge.h"
#include "astar.h"
#include "state.h"
#include "condition.h"
#include "transitionSystem.h"

int main() {
	Edge top_automaton(true);

	// Hard-code DFA_m automaton:
	//top_automaton.connect(2, 0, 1.0, "p_a & p_r");
	/*
	top_automaton.connect(2, 2, 1.0, "!phi_1 & !phi_2");
	top_automaton.connect(2, 1, 1.0, "phi_1 & !phi_2");
	top_automaton.connect(2, 3, 1.0, "!phi_1 & phi_2");
	*/
	//top_automaton.connect(2, 2, 1.0, "!phi_1 & !phi_2");
	top_automaton.connect(2, 1, 1.0, "phi_1");
	top_automaton.connect(2, 3, 1.0, "phi_2");
	top_automaton.connect(1, 1, 1.0, "!phi_2");
	top_automaton.connect(1, 0, 1.0, "phi_2");
	top_automaton.connect(3, 3, 1.0, "!phi_1");
	top_automaton.connect(3, 0, 1.0, "phi_1");
	top_automaton.print();

	Astar schedule;
	schedule.setGraph(&top_automaton);
	schedule.setVInit(2);
	std::vector<int> goal_set;
	goal_set.push_back(0);
	schedule.setVGoalSet(goal_set);
	//std::vector<int> reverse_plan;
	std::vector<int> top_plan;
	float path_length_top;
	bool plan_found = schedule.searchDijkstra(top_plan, path_length_top);
	/*
	top_plan.resize(reverse_plan.size());
	for (int i=0; i<reverse_plan.size(); ++i) {
		top_plan[i] = reverse_plan[reverse_plan.size()-1-i];
	}
	*/
	std::cout<<"\nSchedule Plan:";
	for (int i=0; i<top_plan.size(); ++i) {
		std::cout<<" -> "<<top_plan[i];	
	}
	std::cout<<"\n";

	// Extract the observations from the plan:
	std::vector<std::string> label_plan;
	auto heads_top = top_automaton.getHeads();
	for (int i=0; i<top_plan.size()-1; i++) {
		auto currptr_top = heads_top[top_plan[i]]->adjptr;
		while (currptr_top != nullptr) {
			if (currptr_top->nodeind == top_plan[i+1]) {
				label_plan.push_back(currptr_top->label);
			}
			currptr_top = currptr_top->adjptr;
		}
	}



	/* CREATE ENVIRONMENT FOR MANIPULATOR */
	StateSpace SS_MANIPULATOR;

	std::vector<std::string> ee_labels = {"stow","basket", "container", "L1", "L2"};
	std::vector<std::string> rock_labels = {"basket","ee","L1", "L2"};
	std::vector<std::string> alien_labels = {"container","ee","L1", "L2"};
	std::vector<std::string> grip_labels = {"true","false"};
	
	// Create state space:
	SS_MANIPULATOR.setStateDimension(ee_labels, 0); // eef
	SS_MANIPULATOR.setStateDimension(rock_labels, 1); // rock
	SS_MANIPULATOR.setStateDimension(alien_labels, 2); // alien
	SS_MANIPULATOR.setStateDimension(grip_labels, 3); // eef engaged

	// Label state space:
	SS_MANIPULATOR.setStateDimensionLabel(0, "eeLoc");
	SS_MANIPULATOR.setStateDimensionLabel(1, "rock_loc");
	SS_MANIPULATOR.setStateDimensionLabel(2, "alien_loc");
	SS_MANIPULATOR.setStateDimensionLabel(3, "holding");

	// Create object location group:
	std::vector<std::string> obj_group = {"rock_loc", "alien_loc"};
	SS_MANIPULATOR.setLabelGroup("object locations", obj_group);

	// Set the initial state:
	std::vector<std::string> set_state = {"stow", "L1", "L2", "false"};
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
	
	// Transport 
	conds_m[1].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds_m[1].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg1");
	conds_m[1].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg2");
	conds_m[1].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds_m[1].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg2"); // Stored eeLoc pre-state variable is not the same as post-state eeLoc (eeLoc has moved)
	conds_m[1].addCondition(Condition::POST, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE,"na");
	conds_m[1].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_m[1].setActionLabel("transport");
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
	//conds_m[2].print();


	// Transit
	conds_m[3].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds_m[3].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg");
	conds_m[3].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_m[3].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE,"arg");
	conds_m[3].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_m[3].setActionLabel("transit");
	//conds_m[3].print();

	
	for (int i=0; i<conds_m.size(); ++i){
		cond_ptrs_m[i] = &conds_m[i];
	}


	/* Propositions */
	SimpleCondition p_r;
	p_r.addCondition(Condition::SIMPLE, Condition::LABEL, "rock_loc", Condition::EQUALS, Condition::VAR, "basket");
	p_r.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_r.setLabel("p_r");

	/* Propositions */
	SimpleCondition p_a;
	p_a.addCondition(Condition::SIMPLE, Condition::LABEL, "alien_loc", Condition::EQUALS, Condition::VAR, "container");
	p_a.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	p_a.setLabel("p_a");


	/* DFA_m & Graph Instantiations */
	Edge TS_m(true);
	Edge DFA_m(true);
	Edge PS_m(true);
	
	//bool didwork = conds_m[3].evaluate(&init_state, &test_state);
	//std::cout<<"is true??: "<<didwork<<std::endl;

	// Hard-code DFA_m automaton:
	DFA_m.connect(2, 0, 1.0, "p_a & p_r");
	DFA_m.connect(2, 2, 1.0, "!p_a & !p_r");
	DFA_m.connect(2, 1, 1.0, "p_a & !p_r");
	DFA_m.connect(2, 3, 1.0, "!p_a & p_r");
	DFA_m.connect(1, 1, 1.0, "!p_r");
	DFA_m.connect(1, 0, 1.0, "p_r");
	DFA_m.connect(3, 3, 1.0, "!p_a");
	DFA_m.connect(3, 0, 1.0, "p_a");
	//DFA_m.print();

	ProductSystem<State> PRODSYS_m(&TS_m, &DFA_m, &PS_m);
	//TransitionSystem<State> test(&TS_m);
	PRODSYS_m.setConditions(cond_ptrs_m);
	PRODSYS_m.setInitState(&init_state);
	PRODSYS_m.generate();
	//PRODSYS_m.print();
	
	PRODSYS_m.addProposition(&p_r);
	PRODSYS_m.addProposition(&p_a);
	PRODSYS_m.setAutomatonInitStateIndex(2);
	PRODSYS_m.addAutomatonAcceptingStateIndex(0);
	PRODSYS_m.compose();
	//PRODSYS_m.print();

	std::vector<int> plan_m;
	float pathlength_m;
	PRODSYS_m.plan(plan_m);

	

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
	Edge TS_c(true);
	Edge DFA_c(true);
	Edge PS_c(true);
	
	//bool didwork = conds_m[3].evaluate(&init_state, &test_state);
	//std::cout<<"is true??: "<<didwork<<std::endl;

	// Hard-code DFA_m automaton:
	DFA_c.connect(1, 0, 1.0, "p_on & p_p & p_t");
	DFA_c.connect(1, 1, 1.0, "!p_on | !p_p | !p_t");
	/*
	DFA_c.connect(2, 1, 1.0, "p_a & !p_r");
	DFA_c.connect(2, 3, 1.0, "!p_a & p_r");
	DFA_c.connect(1, 1, 1.0, "!p_r");
	DFA_c.connect(1, 0, 1.0, "p_r");
	DFA_c.connect(3, 3, 1.0, "!p_a");
	DFA_c.connect(3, 0, 1.0, "p_a");
	*/
	DFA_c.print();

	ProductSystem<State> PRODSYS_c(&TS_c, &DFA_c, &PS_c);
	//TransitionSystem<State> test(&TS_c);
	PRODSYS_c.setConditions(cond_ptrs_c);
	//test.setConditions(cond_ptrs_c);
	PRODSYS_c.setInitState(&init_state_c);
	//test.setInitState(&init_state_c);
	PRODSYS_c.generate();
	//test.generate();
	//test.print();

	
	PRODSYS_c.addProposition(&p_p);
	PRODSYS_c.addProposition(&p_t);
	PRODSYS_c.addProposition(&p_on);
	PRODSYS_c.setAutomatonInitStateIndex(1);
	PRODSYS_c.addAutomatonAcceptingStateIndex(0);
	PRODSYS_c.compose();
	//PRODSYS_c.print();

	std::vector<int> plan_c;
	float pathlength_c;
	PRODSYS_c.plan(plan_c);
	std::cout<<"PLAN C SIZE: "<<plan_c.size()<<std::endl;

	std::cout<<"\n\n-------------\n\n";

	std::cout<<"\nSchedule Plan Labels:";
	for (int i=0; i<label_plan.size(); ++i) {
		std::cout<<" -> "<<label_plan[i];	
	}
	std::cout<<"\n";

	std::cout<<"\n\n-------------\n\n";
	for (int ii=0; ii<label_plan.size(); ii++) {
		std::cout<<"Plan for: "<<label_plan[ii]<<std::endl;
		if (label_plan[ii] == "phi_1") {
			std::cout<<"\nCamera Plan:";
			for (int i=0; i<plan_c.size(); ++i) {
				std::cout<<" -> "<<plan_c[i];	
			}
			std::cout<<"\n";
		} else if (label_plan[ii] == "phi_2") {
			std::cout<<"\nManipulator Plan:";
			for (int i=0; i<plan_m.size(); ++i) {
				std::cout<<" -> "<<plan_m[i];	
			}
			std::cout<<"\n";
		}
	}


	std::cout<<"\n\n-------------\n\n";
	PRODSYS_m.updateEdgeWeight(4, 100.0);
	PRODSYS_m.plan(plan_m);

	std::cout<<"\nManipulator Plan:";
	for (int i=0; i<plan_m.size(); ++i) {
		std::cout<<" -> "<<plan_m[i];	
	}
	std::cout<<"\n";

	return 0;
}
