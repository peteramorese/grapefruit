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
		const SymbSearch::StrategyResult* strat;
		std::vector<int> graph_sizes;
		std::vector<std::string> action_seq;
		//void environmentAction(const std::string& action);
		bool executeAction(const std::string& action, bool system_action); // returns acceptance on live dfa
		void reset();
		bool violating;
	public:
		StrategyRTEVAL(TS_EVAL<State>* TS_, DFA_EVAL* cosafe_dfa_, DFA_EVAL* live_dfa_);
		void setStrategy(const SymbSearch::StrategyResult* strat_);
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
			break;
		}
	}
	if (!found_connection) {
		std::cout<<NAME<<"Error: Did not find connectivity in cosafe dfa\n";
	}
	found_connection = false;
	for (int i=0; i<lbls->size(); ++i) {
		if (live_dfa->eval(lbls->operator[](i), true)) {
			found_connection = true;
			break;
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

void StrategyRTEVAL::setStrategy(const SymbSearch::StrategyResult* strat_) {
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
			TS->getCurrState()->print();
			std::cout<<"\n";
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
		TS->getCurrState()->print();
		std::cout<<"\n";
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
	/* Create the Transition System for the Grid Robot  */
	//////////////////////////////////////////////////////

	/* CREATE ENVIRONMENT FOR GRID ROBOT */
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

	// Create state space:
	SS_GRID_ROBOT.setStateDimension(x_labels, 0); // x
	SS_GRID_ROBOT.setStateDimension(y_labels, 1); // y

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
	TS_EVAL<State> ts_eval(true, true, 0); // by default, the init node for the ts is 0
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
		filenames[i] = "../../spot_automaton_file_dump/dfas/dfa_" + std::to_string(i) +".txt";
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

	SymbSearch search_obj;
	//search_obj.setAutomataPrefs(&dfa_eval_ptrs);
	//search_obj.setTransitionSystem(&ts_eval);
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
	
	//float risk_param;
	//std::cout<<"\n------------------------------\n";
	//std::cout<<"Enter risk parameter: ";
	////std::cout<<"\n";
	//std::cin >> risk_param;
	
	//auto cFunc = [&risk_param](unsigned int d){
	//	//std::cout<<"received: "<<d<<std::endl;
	//	//std::cout<<"   ret: "<<1.0/static_cast<float>(d)<<std::endl;
	//	return risk_param/static_cast<float>(d);
	//};
	SymbSearch::StrategyResult S = search_obj.synthesizeRiskStrategy(&ts_eval, dfa_eval_ptrs[0], dfa_eval_ptrs[1]);
	
	std::cout<<"\n   Found strategy? "<<S.success<<" action_map size: "<<S.action_map.size()<<std::endl;
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

	if (finished) {
		std::vector<std::string> xtra_info;
		for (int i=0; i<dfa_arr.size(); ++i) {
			const std::vector<std::string>* ap_ptr = dfa_arr[i].getAP();
			std::string tag = (i == 0) ? "_safety" : "_liveness";
			for (int ii=0; ii<ap_ptr->size(); ++ii) {
				xtra_info.push_back(ap_ptr->operator[](ii));
				xtra_info.back() = xtra_info.back() + tag;
			}
		}
		xtra_info.push_back("GRID_SIZE_" + std::to_string(grid_size));
		rt_eval.writeToFile("../../matlab_scripts/preference_planning_demos/plan_files/strat_traj_execution.txt", xtra_info);
	}


	for (int i=0; i<dfa_eval_ptrs.size(); ++i) {
		delete dfa_eval_ptrs[i];
	}

	return 0;



}
