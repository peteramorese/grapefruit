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
	StateSpace SS_GRID_ROBOT;

	std::vector<std::string> x_labels;
	std::vector<std::string> y_labels;
	const int grid_size = 10;
	for (int i=0; i<grid_size; ++i) {
		std::string temp_string;
		temp_string = "x" + std::to_string(i);
		x_labels.push_back(temp_string);
	}
	for (int i=0; i<grid_size; ++i) {
		std::string temp_string;
		temp_string = "y" + std::to_string(i);
		y_labels.push_back(temp_string);
	}
	//std::vector<std::string> ee_labels = loc_labels;
	//ee_labels.push_back("stow");
	//std::vector<std::string> obj_labels = loc_labels;
	//obj_labels.push_back("ee");
	//std::vector<std::string> grip_labels = {"true","false"};

	// Create state space:
	SS_GRID_ROBOT.setStateDimension(x_labels, 0); // x
	SS_GRID_ROBOT.setStateDimension(y_labels, 1); // y
	//	std::cout<<"y labels:";
	//for (int i=0; i<y_labels.size(); ++i) {
	//	std::cout<<y_labels[i]<<", "<<std::endl;
	//}
	//return 0;

	// Label state space:
	SS_GRID_ROBOT.setStateDimensionLabel(0, "x_coord");
	SS_GRID_ROBOT.setStateDimensionLabel(1, "y_coord");

	// Create object location group:
	std::vector<std::string> coordinates = {"x_coord", "y_coord"};
	SS_GRID_ROBOT.setLabelGroup("coordinates", coordinates);

	// Set the initial state:
	std::vector<std::string> set_state = {"x0", "y0"};
	State init_state(&SS_GRID_ROBOT);	
	init_state.setState(set_state);

	Graph<WL> ts_graph_m(true, true);
	TS_EVAL<State> ts_eval(&ts_graph_m, true, true, 0); // by default, the init node for the ts is 0
	ts_eval.setInitState(&init_state);

	for (int i=0; i<grid_size; ++i) { // x
		for (int ii=0; ii<grid_size; ++ii) { // y
			State src_state(&SS_GRID_ROBOT);
			State dst_state(&SS_GRID_ROBOT);
			src_state.setState(x_labels[i], 0);
			src_state.setState(y_labels[ii], 1);
			//std::cout<<" SRC:"<<std::endl;
			//src_state.print();
			bool stay_put = false;
			for (int dir=0; dir<4; ++dir) {
				switch (dir) {
					case 0: // left 
						//std::cout<<"i: "<<i<<"ii: "<<ii<<std::endl;
						if (i != 0) {
							//std::cout<<"move left"<<std::endl;
							dst_state.setState(x_labels[i-1], 0);
							dst_state.setState(y_labels[ii], 1);
							//std::cout<<"dst: "<<std::endl;
							//dst_state.print();
							ts_eval.connect(&src_state, &dst_state, 5.0f, "move_left");
						} else {
							if (!stay_put) {
								ts_eval.connect(&src_state, &src_state, 0.0f, "stay_put");
								stay_put = true;
							} 
						}
						break;
					case 1: // right
						if (i != (grid_size-1)) {
							//std::cout<<"move right"<<std::endl;
							dst_state.setState(x_labels[i+1], 0);
							dst_state.setState(y_labels[ii], 1);
							//std::cout<<"dst: "<<std::endl;
							//dst_state.print();
							ts_eval.connect(&src_state, &dst_state, 5.0f, "move_right");
						} else {
							if (!stay_put) {
								ts_eval.connect(&src_state, &src_state, 0.0f, "stay_put");
								stay_put = true;
							} 
						}

						break;
					case 2: // down
						if (ii != 0) {
							//std::cout<<"move down"<<std::endl;
							dst_state.setState(x_labels[i], 0);
							dst_state.setState(y_labels[ii-1], 1);
							//std::cout<<"dst: "<<std::endl;
							//dst_state.print();
							ts_eval.connect(&src_state, &dst_state, 5.0f, "move_down");
						} else {
							if (!stay_put) {
								ts_eval.connect(&src_state, &src_state, 0.0f, "stay_put");
								stay_put = true;
							} 
						}

						break;
					case 3: // up
						if (ii != (grid_size-1)) {
							//std::cout<<"move up"<<std::endl;
							dst_state.setState(x_labels[i], 0);
							dst_state.setState(y_labels[ii+1], 1);
							//std::cout<<"dst: "<<std::endl;
							//dst_state.print();
							ts_eval.connect(&src_state, &dst_state, 5.0f, "move_up");
						} else {
							if (!stay_put) {
								ts_eval.connect(&src_state, &src_state, 0.0f, "stay_put");
								stay_put = true;
							} 
						}
						break;
				}
			}
		}
	}
	std::cout<<"af make ts"<<std::endl;
	ts_eval.finishConnecting();


	/* Propositions */
	std::cout<<"Setting Atomic Propositions... "<<std::endl;
	std::vector<SimpleCondition> APs;
	std::vector<SimpleCondition*> AP_ptrs;
	for (int i=0; i<grid_size; ++i) {
		for (int ii=0; ii<grid_size; ++ii) {
			SimpleCondition temp_AP;
			temp_AP.addCondition(Condition::SIMPLE, Condition::LABEL, "x_coord", Condition::EQUALS, Condition::VAR, x_labels[i]);
			temp_AP.addCondition(Condition::SIMPLE, Condition::LABEL, "y_coord", Condition::EQUALS, Condition::VAR, y_labels[ii]);
			temp_AP.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
			temp_AP.setLabel("ap_" + x_labels[i] + "_" + y_labels[ii]);
			//std::cout<<"made ap: "<<"ap_" + x_labels[i] + "_" + y_labels[ii]<<std::endl;
			APs.push_back(temp_AP);
		}
	}
	AP_ptrs.resize(APs.size());
	for (int i=0; i<APs.size(); ++i) {
		AP_ptrs[i] = &APs[i];
	}

	ts_eval.setPropositions(AP_ptrs);
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
	auto cFunc = [](unsigned int d){
		//std::cout<<"received: "<<d<<std::endl;
		//std::cout<<"   ret: "<<1.0/static_cast<float>(d)<<std::endl;
		return 1/static_cast<float>(d);
	};
	SymbSearch<FlexLexSetS>::Strategy S;
	bool success = search_obj.generateRiskStrategy(dfa_eval_ptrs[0], dfa_eval_ptrs[1], cFunc, S, true);
	std::cout<<"\n   Found strategy? "<<success<<" action_map size: "<<S.action_map.size()<<std::endl;
	for (int i=0; i<S.action_map.size(); ++i) {
		std::cout<<"Prod ind: "<<i<<"  --> action: "<<S.action_map[i]<<std::endl;
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

	for (int i=0; i<dfa_eval_ptrs.size(); ++i) {
		delete dfa_eval_ptrs[i];
	}

	return 0;



}
