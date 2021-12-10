#include<iostream>
#include<fstream>
#include "graph.h"
#include "condition.h"
#include "transitionSystem.h"
#include "stateSpace.h"
#include "state.h"
#include "symbSearch.h"

/* HEADER */
class StrategyRTEVAL {
	private:
		TS_EVAL<State>* TS;
		DFA_EVAL* cosafe_dfa;
		DFA_EVAL* live_dfa;
		const std::string NAME = " [StrategyRTEVAL] ";
		const SymbSearch<FlexLexSetS>::Strategy* strat;
		std::vector<int> graph_sizes;
		std::vector<std::string> action_seq;
		//void environmentAction(const std::string& action);
		bool executeAction(const std::string& action, bool system_action); // returns acceptance on live dfa
		void reset();
		bool violating;
	public:
		StrategyRTEVAL(TS_EVAL<State>* TS_, DFA_EVAL* cosafe_dfa_, DFA_EVAL* live_dfa_);
		void setStrategy(const SymbSearch<FlexLexSetS>::Strategy* strat_);
		bool run();
		void writeToFile(const std::string& filename, const std::vector<std::string>& xtra_info);
};

/* CLASS DEFINITION */
StrategyRTEVAL::StrategyRTEVAL(TS_EVAL<State>* TS_, DFA_EVAL* cosafe_dfa_, DFA_EVAL* live_dfa_) : TS(TS_), cosafe_dfa(cosafe_dfa_), live_dfa(live_dfa_) {
	std::cout<<NAME<<"Setting up Strategy Realtime EVAL...\n";
	graph_sizes.resize(3);
	graph_sizes[0] = TS->size();
	graph_sizes[1] = cosafe_dfa->getDFA()->size();
	graph_sizes[2] = live_dfa->getDFA()->size();
	violating = false;
	action_seq.clear();
}

//void StrategyRTEVAL::environmentAction(const std::string& action) {
//	TS->eval(action, true);	
//	cosafe_dfa->eval(action, true);	
//	live_dfa->eval(action, true);	
//}

bool StrategyRTEVAL::executeAction(const std::string& action, bool system_action) {
	TS->eval(action, true);	
	const std::vector<std::string>* lbls = TS->returnStateLabels(TS->getCurrNode());
	bool found_connection = false;
	for (int i=0; i<lbls->size(); ++i) {
		if (cosafe_dfa->eval(lbls->operator[](i), true)) {
			found_connection = true;
		}
	}
	if (!found_connection) {
		std::cout<<NAME<<"Error: Did not find connectivity in cosafe dfa\n";
	}
	found_connection = false;
	for (int i=0; i<lbls->size(); ++i) {
		if (live_dfa->eval(lbls->operator[](i), true)) {
			found_connection = true;
		}
	}
	if (!found_connection) {
		std::cout<<NAME<<"Error: Did not find connectivity in liveness dfa\n";
	}
	
	//live_dfa->eval(action, true);	
	std::string action_lbl = action;
	if (system_action) {
		action_lbl = action_lbl + "_SYS";
	} else {
		action_lbl = action_lbl + "_ENV";
	}
	action_seq.push_back(action_lbl);
	violating = (cosafe_dfa->isCurrAccepting()) ? true : false;
	return (live_dfa->isCurrAccepting()) ? true : false;
}

void StrategyRTEVAL::reset() {
	TS->reset();
	cosafe_dfa->reset();
	live_dfa->reset();
	action_seq.clear();
	violating = false;
}

void StrategyRTEVAL::setStrategy(const SymbSearch<FlexLexSetS>::Strategy* strat_) {
	strat = strat_;
}

bool StrategyRTEVAL::run() {
	std::cout<<NAME<<"Running... (Quit: 'q', Reset: 'r')\n";
	bool finished = false;
	bool found = true;
	reset();
	while (!finished) {
		if (violating) {
			std::cout<<NAME<<"Violated safety specification!"<<std::endl;
			return false;
		}
		if (found) {
			std::cout<<"\n"<<NAME<<"Current State ("<<TS->getCurrNode()<<"): \n";
			//TS->getState(TS->getCurrNode());
			std::cout<<NAME<<"--System's turn--\n";
			int prod_ind = Graph<float>::augmentedStateImage({TS->getCurrNode(), cosafe_dfa->getCurrNode(), live_dfa->getCurrNode()}, graph_sizes);
			std::string act;
			if (strat->reachability[prod_ind]) {
				act = strat->action_map[prod_ind];
				if (act == "") {
					std::cout<<NAME<<"Error: Empty action in strategy\n";
				} else {
					std::cout<<NAME<<"Found and executed action in strategy: "<<act<<std::endl;
					finished = executeAction(act, true);
				}
			} else {
				std::cout<<NAME<<"Error: Prod state unreachable. TS: "<<TS->getCurrNode()<<" CoSafe: "<<cosafe_dfa->getCurrNode()<<" Live: "<<live_dfa->getCurrNode()<<std::endl;
			}
		}
		std::cout<<"\n"<<NAME<<"Current State ("<<TS->getCurrNode()<<"): \n";
		//TS->getState(TS->getCurrNode());
		std::cout<<NAME<<"--Environment's turn--\n";
		std::vector<WL*> con_data;
		TS->getConnectedDataEVAL(con_data);
		std::cout<<NAME<<" Opt 0: No Intervention"<<std::endl;
		for (int i=0; i<con_data.size(); ++i) {
			std::cout<<NAME<<" Opt "<<i+1<<": "<<con_data[i]->label<<std::endl;
		}
		std::string input;
		std::cin >> input;
		found = false;
		if (input == "q") {
			return false;
		} else if (input == "r") {
			found = true;
			reset();
		} else {
			std::string::size_type sz;
			int opt = std::stoi(input, &sz) - 1;
			if (opt == -1) {
				std::cout<<NAME<<"No action taken." <<std::endl;
				found = true;
			} else if (opt >= 0 && opt<con_data.size()) {
				found = true;
				finished = executeAction(con_data[opt]->label, false);
				std::cout<<NAME<<"Action taken: "<<con_data[opt]->label<<std::endl;
			} else {
				std::cout<<NAME<<"Invalid option. Try again\n";
			}
		}
	}
	return finished;
}

void StrategyRTEVAL::writeToFile(const std::string& filename, const std::vector<std::string>& xtra_info) {
	std::string line;
	std::ofstream plan_file;
	plan_file.open(filename);
	for (int i=0; i<action_seq.size(); ++i) {
			plan_file <<action_seq[i]<<"\n";
	}
	for (int i=0; i<xtra_info.size(); ++i) {
			plan_file <<xtra_info[i]<<"\n";
	}
	plan_file.close();
}

/* MAIN */

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
	SS_MANIPULATOR.setStateDimensionLabel(1, "blueBox_1");
	SS_MANIPULATOR.setStateDimensionLabel(2, "pinkBox_1");
	SS_MANIPULATOR.setStateDimensionLabel(3, "blueBox_2");
	SS_MANIPULATOR.setStateDimensionLabel(4, "pinkBox_2");
	SS_MANIPULATOR.setStateDimensionLabel(5, "greenBox_1");
	SS_MANIPULATOR.setStateDimensionLabel(6, "holding");

	// Create object location group:
	std::vector<std::string> obj_group = {"blueBox_1", "pinkBox_1", "blueBox_2", "pinkBox_2", "greenBox_1"};
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
	conds_m[0].setActionCost(1);

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
	conds_m[2].setActionCost(1);
	//conds_m[2].print();


	// Transit
	conds_m[3].addCondition(Condition::PRE, Condition::LABEL, "holding", Condition::EQUALS, Condition::VAR, "false");
	conds_m[3].addCondition(Condition::PRE, Condition::LABEL, "eeLoc", Condition::ARG_FIND, Condition::NONE, Condition::FILLER, Condition::TRUE, "arg");
	conds_m[3].setCondJunctType(Condition::PRE, Condition::CONJUNCTION);

	conds_m[3].addCondition(Condition::POST, Condition::ARG_V, Condition::FILLER, Condition::ARG_EQUALS, Condition::LABEL, "eeLoc", Condition::NEGATE,"arg");
	conds_m[3].setCondJunctType(Condition::POST, Condition::CONJUNCTION);
	conds_m[3].setActionLabel("transit");
	conds_m[3].setActionCost(1);
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
	int N_DFAs = 2;

	//// Get input from user for how many formulas to read in:
	//std::cout<<"\n------------------------------\n";
	//std::cout<<"Enter number of formulas: ";
	//std::cin >> N_DFAs;
	//std::cout<<"\n";

	std::vector<DFA> dfa_arr(N_DFAs);
	std::vector<std::string> filenames(N_DFAs);
	for (int i=0; i<N_DFAs; ++i) {
		filenames[i] = "../spot_automaton_file_dump/dfas/dfa_" + std::to_string(i) +".txt";
	}
	for (int i=0; i<N_DFAs; ++i) {
		dfa_arr[i].readFileSingle(filenames[i]);
	}
	//std::cout<<"\n\nPrinting all DFA's (read into an array)...\n\n"<<std::endl;
	bool two_formulas = true;
	for (int i=0; i<N_DFAs; ++i) {
		if (i == 0) {
			std::cout<<"\n\n  Printing negation of Safety DFA:\n";
		} else if (i == 1) {
			std::cout<<"\n\n  Printing Liveness DFA:\n";
		} 
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

	SymbSearch<FlexLexSetS> search_obj;
	//search_obj.setAutomataPrefs(&dfa_eval_ptrs);
	search_obj.setTransitionSystem(&ts_eval);
	//float mu;
	//char use_h;
	//char use_dfs;
	//std::cout<<"\n------------------------------\n";
	//std::cout<<"Enter flexibility parameter: ";
	//std::cout<<"\n";
	//std::cin >> mu;
	//search_obj.setFlexibilityParam(mu);

	//std::cout<<"\n------------------------------\n";
	//std::cout<<"Use heuristic? [y/n]: ";
	//std::cout<<"\n";
	//std::cin >> use_h;
	//bool use_h_flag = (use_h == 'y') ? true : false;

	//std::cout<<"\n------------------------------\n";
	//std::cout<<"Use iterative DFS? [y/n]: ";
	//std::cout<<"\n";
	//std::cin >> use_dfs;
	//bool use_dfs_flag = (use_dfs == 'y') ? true : false;
	//search_obj.setFlexibilityParam(0.0f);
	
	float risk_param;
	std::cout<<"\n------------------------------\n";
	std::cout<<"Enter risk parameter: ";
	//std::cout<<"\n";
	std::cin >> risk_param;
	
	auto cFunc = [&risk_param](unsigned int d){
		//std::cout<<"received: "<<d<<std::endl;
		//std::cout<<"   ret: "<<1.0/static_cast<float>(d)<<std::endl;
		return risk_param/static_cast<float>(d);
	};
	SymbSearch<FlexLexSetS>::Strategy S;
	bool success = search_obj.generateRiskStrategy(dfa_eval_ptrs[0], dfa_eval_ptrs[1], cFunc, S, false);
	std::cout<<"\n   Found strategy? "<<success<<" action_map size: "<<S.action_map.size()<<std::endl;
	for (int i=0; i<S.action_map.size(); ++i) {
		std::vector<int> ret_inds;
		Graph<float>::augmentedStatePreImage({static_cast<int>(ts_eval.size()),dfa_eval_ptrs[0]->getDFA()->size(),dfa_eval_ptrs[0]->getDFA()->size()}, i, ret_inds);
		std::cout<<"Prod ind: "<<i<<" TS ind: "<<ret_inds[0]<<" CoSafe: "<<ret_inds[1]<<" Liveness: "<<ret_inds[2]<<"  --> action: "<<S.action_map[i]<<std::endl;
	}
	//if (success) {
	//	std::vector<std::string> xtra_info;
	//	for (int i=0; i<dfa_arr.size(); ++i) {
	//		const std::vector<std::string>* ap_ptr = dfa_arr[i].getAP();
	//		for (int ii=0; ii<ap_ptr->size(); ++ii) {
	//			xtra_info.push_back(ap_ptr->operator[](ii));
	//			xtra_info.back() = xtra_info.back() + "_prio" + std::to_string(i);
	//		}
	//	}
	//	search_obj.writePlanToFile("/Users/Peter/Documents/MATLAB/preference_planning_demos/plan.txt", xtra_info);
	//}
	

	StrategyRTEVAL rt_eval(&ts_eval, dfa_eval_ptrs[0], dfa_eval_ptrs[1]);
	rt_eval.setStrategy(&S);
	bool finished = rt_eval.run();
	finished = true;

	//if (finished) {
	//	std::vector<std::string> xtra_info;
	//	for (int i=0; i<dfa_arr.size(); ++i) {
	//		const std::vector<std::string>* ap_ptr = dfa_arr[i].getAP();
	//		std::string tag = (i == 0) ? "_safety" : "_liveness";
	//		for (int ii=0; ii<ap_ptr->size(); ++ii) {
	//			xtra_info.push_back(ap_ptr->operator[](ii));
	//			xtra_info.back() = xtra_info.back() + tag;
	//		}
	//	}
	//	xtra_info.push_back("GRID_SIZE_" + std::to_string(grid_size));
	//	rt_eval.writeToFile("/Users/Peter/Documents/MATLAB/preference_planning_demos/strat_traj_execution.txt", xtra_info);
	//}


	for (int i=0; i<dfa_eval_ptrs.size(); ++i) {
		delete dfa_eval_ptrs[i];
	}

	return 0;



}
