#include<iostream>
#include<unordered_map>
#include<chrono>
#include<ctime>
#include<fstream>
#include "graph.h"
#include "condition.h"
#include "transitionSystem.h"
#include "stateSpace.h"
#include "state.h"
#include "symbSearch.h"
#include "benchmark.h"

int main(int argc, char *argv[]) {
	//std::cout<<"time init: "<<time_init<<std::endl;
	//for (int i=0; i<argc; ++i) {
	//	std::cout<<argv[i]<<std::endl;
	//}
	//std::cout<<"time init: "<<benchmark.measureMicro()<<std::endl;
	// Set default arguments:

	bool manual_setup = true;
	int grid_size = 10;
	bool verbose = false;
	bool use_benchmark = false;
	std::string bm_filename_path = "./benchmark_data/preference_planner_bm.txt";
	std::string plan_filename_path = "../../matlab_scripts/preference_planning_demos/plan_files/plan.txt";
	std::string dfa_filename_path_prefix = "../../spot_automaton_file_dump/dfas/";

	// These will be manually set if manual_setup = true
	int N_DFAs;
	float mu = 0;
	bool use_h_flag = false;
	bool write_file_flag = false;

	// Parse arguments:
	if (argc > 1){
		manual_setup = false;
		int i_arg = 1;
		while (i_arg < argc) {
			std::string arg = argv[i_arg];
			if (arg == "--gridsize") {
				std::string::size_type size_t;
				grid_size = std::stoi(argv[i_arg + 1], &size_t);
				i_arg++;
			} else if (arg == "--numdfas") {
				std::string::size_type size_t;
				N_DFAs = std::stoi(argv[i_arg + 1], &size_t);

			} else if (arg == "--mu") {
				std::string::size_type size_t;
				mu = std::stof(argv[i_arg + 1], &size_t);
			} else if (arg == "--use-h") {
				use_h_flag = true;
			} else if (arg == "--write-plan") {
				write_file_flag = true;
			} else if (arg == "--verbose") {
				verbose = true;
			} else if (arg == "--benchmark") {
				use_benchmark = true;
			} else if (arg == "--plan-file") {
				plan_filename_path = argv[i_arg + 1];
			} else if (arg == "--dfas-filepath") {
				dfa_filename_path_prefix = argv[i_arg + 1];
			} else if (arg == "--bm-file") {
				bm_filename_path = argv[i_arg + 1];
				//std::cout<<"BM FILE NAME PREF: "<<bm_filename_path<<std::endl;
			}
			i_arg++;
		}
	}

	Benchmark benchmark(bm_filename_path);
	
	if (use_benchmark) {
		benchmark.addAttribute("num_dfas: " + std::to_string(N_DFAs));
		benchmark.addAttribute("flexibility: " + std::to_string(mu));
	}

	//std::cout<<"PRINTING ARGS:"<<argv[1]<<std::endl;


	//////////////////////////////////////////////////////
	/* Create the Transition System for the Grid Robot  */
	//////////////////////////////////////////////////////

	/* CREATE ENVIRONMENT FOR GRID ROBOT */
	StateSpace SS_GRID_ROBOT;

	std::vector<std::string> x_labels;
	std::vector<std::string> y_labels;
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
							ts_eval.connect(&src_state, &dst_state, 1.0f, "move_left");
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
							ts_eval.connect(&src_state, &dst_state, 1.0f, "move_right");
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
							ts_eval.connect(&src_state, &dst_state, 1.0f, "move_down");
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
							ts_eval.connect(&src_state, &dst_state, 1.0f, "move_up");
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
	ts_eval.finishConnecting();


	/* Propositions */
	if (verbose) {
		std::cout<<"Setting Atomic Propositions... "<<std::endl;
	}
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
	if (verbose) {
		std::cout<<"\n\n Printing the Transition System: \n\n"<<std::endl;
		ts_eval.printTS();
	}



	/////////////////////////////////////////////////
	/*       Read in the DFA's from the files      */
	/////////////////////////////////////////////////

	DFA A;

	if (manual_setup){
		std::cout<<"\n------------------------------\n";
		std::cout<<"Enter number of formulas: ";
		std::cin >> N_DFAs;
		std::cout<<"\n";
	}
	// Get input from user for how many formulas to read in:

	std::vector<DFA> dfa_arr(N_DFAs);
	std::vector<std::string> filenames(N_DFAs);
	for (int i=0; i<N_DFAs; ++i) {
		filenames[i] = dfa_filename_path_prefix +"dfa_" + std::to_string(i) +".txt";
	}
	for (int i=0; i<N_DFAs; ++i) {
		dfa_arr[i].readFileSingle(filenames[i]);
	}
	if (verbose) {
		std::cout<<"\n\nPrinting all DFA's (read into an array)...\n\n"<<std::endl;
		for (int i=0; i<N_DFAs; ++i) {
			dfa_arr[i].print();
			std::cout<<"\n"<<std::endl;
		}
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

	SymbSearch<DetourLex> search_obj(bm_filename_path, verbose);
	search_obj.setAutomataPrefs(&dfa_eval_ptrs);
	search_obj.setTransitionSystem(&ts_eval);
	if (manual_setup) {
		std::cout<<"\n------------------------------\n";
		std::cout<<"Enter flexibility parameter: ";
		std::cout<<"\n";
		std::cin >> mu;
	
		char use_h;
		std::cout<<"\n------------------------------\n";
		std::cout<<"Use heuristic? [y/n]: ";
		std::cout<<"\n";
		std::cin >> use_h;
		use_h_flag = (use_h == 'y') ? true : false;
	
		char write_f;
		std::cout<<"\n------------------------------\n";
		std::cout<<"Write to file? [y/n]: ";
		std::cout<<"\n";
		std::cin >> write_f;
		write_file_flag = (write_f == 'y') ? true : false;
	}

	//char use_dfs;
	//std::cout<<"\n------------------------------\n";
	//std::cout<<"Use iterative DFS? [y/n]: ";
	//std::cout<<"\n";
	//std::cin >> use_dfs;
	//bool use_dfs_flag = (use_dfs == 'y') ? true : false;
	//search_obj.setFlexibilityParam(0.0f);
	//bool success = search_obj.search(use_h_flag, use_dfs_flag);
	search_obj.setFlexibilityParam(mu);
	benchmark.pushStartPoint("total_search");
	bool success = search_obj.search(use_h_flag);
	//std::cout<<"search time: "<<benchmark.measureMicro("before_search")<<std::endl;
	benchmark.measureMilli("total_search");
	benchmark.pushAttributesToFile();
	benchmark.finishSessionInFile();
	//std::cout<<"Found plan? "<<success<<std::endl;
	if (success) {
		std::vector<std::string> xtra_info;
		for (int i=0; i<dfa_arr.size(); ++i) {
			const std::vector<std::string>* ap_ptr = dfa_arr[i].getAP();
			for (int ii=0; ii<ap_ptr->size(); ++ii) {
				xtra_info.push_back(ap_ptr->operator[](ii));
				xtra_info.back() = xtra_info.back() + "_prio" + std::to_string(i);
			}
		}
		if (write_file_flag) {
			search_obj.writePlanToFile(plan_filename_path, xtra_info);
		}
	}
	for (int i=0; i<dfa_eval_ptrs.size(); ++i) {
		delete dfa_eval_ptrs[i];
	}
	return 0;
}
