#include "graph.h"
#include "condition.h"
#include "stateSpace.h"
#include "state.h"
#include "game.h"

int main() {

	//////////////////////////////////////////////////////
	/* Create the Game for the Manipualtor */
	//////////////////////////////////////////////////////

	/* CREATE ENVIRONMENT FOR MANIPULATOR */
	StateSpace SS_MANIPULATOR;

	//std::vector<std::string> loc_labels = {"L0", "L1", "L2", "L3"};	
	std::vector<std::string> loc_labels = {"L0", "L1", "L2"};	
	std::vector<std::string> ee_labels = loc_labels;
	ee_labels.push_back("stow");
	std::vector<std::string> obj_labels = loc_labels;
	obj_labels.push_back("ee");
	std::vector<std::string> grip_labels = {"true","false"};

	// Create state space:
	SS_MANIPULATOR.setStateDimension(ee_labels, 0); // eef
	SS_MANIPULATOR.setStateDimension(obj_labels, 1); // obj1
	//SS_MANIPULATOR.setStateDimension(obj_labels, 2); // obj2
	SS_MANIPULATOR.setStateDimension(grip_labels, 2); // eef engaged <-- CHANGE TO 3 INSTEAD OF 2

	// Label state space:
	SS_MANIPULATOR.setStateDimensionLabel(0, "eeLoc");
	SS_MANIPULATOR.setStateDimensionLabel(1, "obj_1");
	//SS_MANIPULATOR.setStateDimensionLabel(2, "obj_2");
	SS_MANIPULATOR.setStateDimensionLabel(2, "holding"); //<-- CHANGE TO 3 INSTEAD OF 2

	// Create object location group:
	//SS_MANIPULATOR.setLabelGroup("object locations", {"obj_1", "obj_2"});
	SS_MANIPULATOR.setLabelGroup("object locations", {"obj_1"});

	// Set the initial state:
	State init_state(&SS_MANIPULATOR);	
	//init_state.setState({"stow", "L0", "L1", "false"});
	init_state.setState({"stow", "L0", "false"});


	/* SET PLAYER CONDITIONS */
	Condition player_0_turn;
	player_0_turn.addCondition(Condition::PRE, Condition::LABEL, "player", Condition::EQUALS, Condition::VAR, "0");
	player_0_turn.setCondJunctType(Condition::PRE, Condition::CONJUNCTION);
	player_0_turn.addCondition(Condition::POST, Condition::LABEL, "player", Condition::EQUALS, Condition::VAR, "1");
	player_0_turn.setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	player_0_turn.setActionLabel("player 0 turn");

	Condition player_1_turn;
	player_1_turn.addCondition(Condition::PRE, Condition::LABEL, "player", Condition::EQUALS, Condition::VAR, "1");
	player_1_turn.setCondJunctType(Condition::PRE, Condition::CONJUNCTION);
	player_1_turn.addCondition(Condition::POST, Condition::LABEL, "player", Condition::EQUALS, Condition::VAR, "0");
	player_1_turn.setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	player_1_turn.setActionLabel("player 1 turn");

	//StateSpace player_space;
	//std::vector<std::string> player_lbls(2);
	//for (int i=0; i<2; ++i) {
	//	player_lbls[i] = std::to_string(i);
	//}
	//player_space.setStateDimension(player_lbls, 0);
	//player_space.setStateDimensionLabel(0, "player");


	//State test_state_1(&player_space);
	//test_state_1.setState("1", 0);

	//State test_state_2(&player_space);
	//test_state_2.setState("1", 0);

	//std::cout<<" bool result: "<<player_1_turn.evaluate(&test_state_1, &test_state_2)<<std::endl;

	//return 0;

	/* SET CONDITIONS */
	std::vector<Condition> conds;
	std::vector<Condition*> cond_ptrs;
	conds.resize(5);
	cond_ptrs.resize(5);

	// Grasp 
	conds[0].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds[0].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc",Condition::TRUE, "arg");
	conds[0].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds[0].addCondition(Condition::POST, Condition::ARG_L, Condition::FILLER, Condition::ARG_EQUALS, Condition::VAR, "ee",Condition::TRUE, "arg");
	conds[0].addCondition(Condition::POST, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds[0].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[0].setActionLabel("grasp");
	conds[0].setActionCost(0);

	// Transport 
	conds[1].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "true");
	conds[1].addCondition(Condition::PRE, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg1");
	conds[1].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg2");
	conds[1].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds[1].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE, "arg2"); // Stored eeLoc pre-state variable is not the same as post-state eeLoc (eeLoc has moved)
	conds[1].addCondition(Condition::POST, Condition::GROUP, "object locations", Condition::ARG_FIND, Condition::LABEL, "eeLoc", Condition::NEGATE,"na");
	conds[1].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[1].setActionLabel("transport");
	conds[1].setActionCost(5);
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
	conds[2].setActionCost(0);
	//conds[2].print();


	// Transit
	conds[3].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds[3].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg");
	conds[3].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds[3].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE,"arg");
	conds[3].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[3].setActionLabel("transit");
	conds[3].setActionCost(0);
	//conds[3].print();

	// Intervene (environment action)
	conds[4].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg1");
	conds[4].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg2");
	//conds[4].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg2");
	conds[4].setCondJunctType(Condition::PRE, Condition::CONJUNCTION); // Used to store eeLoc pre-state variable
	conds[4].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::TRUE, "arg2"); // Stored eeLoc pre-state variable is the same as post-state eeLoc 
	conds[4].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "holding", Condition::TRUE, "arg2"); // Stored eeLoc pre-state variable is the same as post-state eeLoc 
	conds[4].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds[4].setExclEq(false); // Don't enforce that all other states values must be equal
	conds[4].setActionLabel("intervene");
	conds[4].setActionCost(0);


	for (int i=0; i<conds.size(); ++i){
		cond_ptrs[i] = &conds[i];
	}


	/* Propositions */
	std::cout<<"Setting Atomic Propositions... "<<std::endl;
	std::vector<SimpleCondition> AP(loc_labels.size()*2);
	std::vector<SimpleCondition*> AP_ptrs(loc_labels.size()*2);
	for (int i=0; i<loc_labels.size(); ++i) {
		AP[2*i].addCondition(Condition::SIMPLE, Condition::LABEL, "obj_1", Condition::EQUALS, Condition::VAR, loc_labels[i]);
		AP[2*i].addCondition(Condition::SIMPLE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
		AP[2*i].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
		AP[2*i].setLabel("obj_1_" + loc_labels[i]);

		AP[2*i + 1].addCondition(Condition::SIMPLE, Condition::LABEL, "obj_2", Condition::EQUALS, Condition::VAR, loc_labels[i]);
		AP[2*i + 1].addCondition(Condition::SIMPLE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
		AP[2*i + 1].setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
		AP[2*i + 1].setLabel("obj_2_" + loc_labels[i]);
	}
	for (int i=0; i<AP.size(); ++i) {
		AP_ptrs[i] = &AP[i];
	}

	std::vector<Condition*> player_conditions = {&player_0_turn, &player_0_turn, &player_0_turn, &player_0_turn, &player_1_turn};

	// Create the game:
	Game<State> game(2); // by default, the init node for the ts is 0
	game.setConditions(cond_ptrs, player_conditions);
	game.setPropositions(AP_ptrs);
	game.setInitState(&init_state, 0);
	game.generate();
	std::cout<<"\n\n Printing the Game: \n\n"<<std::endl;
	game.print();


}
