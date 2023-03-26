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
#include "orderedPlanner.h"

int main(int argc, char *argv[]) {

	// Set default arguments:
	bool manual_setup = true;
	int grid_size = 5;
	bool verbose = false;
	std::string plan_filename_path = "../../matlab_scripts/preference_planning_demos/plan_files/plan.txt";
	std::string dfa_filename_path_prefix = "../../spot_automaton_file_dump/dfas/";

	// These will be manually set if manual_setup = true
	int N_DFAs;
	float mu = 0;
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
				i_arg++;

			} else if (arg == "--verbose") {
				verbose = true;
			} else if (arg == "--dfas-filepath") {
				dfa_filename_path_prefix = argv[i_arg + 1];
				i_arg++;
			} else {
				std::cout<<"Unrecognized argument: "<<argv[i_arg]<<"\n";
				return 0;
			}
			i_arg++;
		}
	}


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
			bool stay_put = false;
			for (int dir=0; dir<4; ++dir) {
				switch (dir) {
					case 0: // left 
						if (i != 0) {
							dst_state.setState(x_labels[i-1], 0);
							dst_state.setState(y_labels[ii], 1);
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
							dst_state.setState(x_labels[i+1], 0);
							dst_state.setState(y_labels[ii], 1);
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
							dst_state.setState(x_labels[i], 0);
							dst_state.setState(y_labels[ii-1], 1);
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
							dst_state.setState(x_labels[i], 0);
							dst_state.setState(y_labels[ii+1], 1);
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
		ts_eval.print();
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

	std::vector<DFA_EVAL*> dfa_eval_ptrs;
	for (int i=0; i<N_DFAs; ++i) {
		DFA_EVAL* temp_dfa_eval_ptr = new DFA_EVAL(&dfa_arr[i]);
		dfa_eval_ptrs.push_back(temp_dfa_eval_ptr);
	}

	std::vector<const std::vector<std::string>*> total_alphabet(N_DFAs);
	for (int i=0; i<N_DFAs; ++i) {
		total_alphabet[i] = dfa_eval_ptrs[i]->getAlphabetEVAL();
	}
	ts_eval.mapStatesToLabels(total_alphabet);

	// Print graph sizes:
	std::vector<int> graph_sizes = SymbolicMethods::getGraphSizes(ts_eval, dfa_eval_ptrs);
	std::cout<<"Graph sizes: ";
	for (auto& graph_size : graph_sizes) {
		std::cout<<graph_size<<", ";
	}
	std::cout<<"\n";

	// Test post():
	std::vector<int> node = {3, 0, 0, 1};
	{
		int p = Graph<float>::augmentedStateImage(node, graph_sizes);
		std::vector<int> set = {p};
		std::vector<int> post_set = SymbolicMethods::post(ts_eval, dfa_eval_ptrs, set);

		std::cout<<"Post set: "<<std::endl;
		for (auto pp : post_set) {
			std::vector<int> ret_inds;
			Graph<float>::augmentedStatePreImage(graph_sizes, pp, ret_inds);
			std::cout<<" - Prod: "<<pp<<", TS: "<<ret_inds[0];
			for (int i=1; i<ret_inds.size(); ++i) {
				std::cout<<", Dfa"<<i-1<<": "<<ret_inds[i];
			}
			std::cout<<"\n";
		}
	}
	
	// Test pre():
	{
		int p = Graph<float>::augmentedStateImage(node, graph_sizes);
		std::vector<int> set = {p};
		std::vector<int> pre_set = SymbolicMethods::pre(ts_eval, dfa_eval_ptrs, set);
		std::cout<<"Pre set: "<<std::endl;
		for (auto pp : pre_set) {
			std::vector<int> ret_inds;
			Graph<float>::augmentedStatePreImage(graph_sizes, pp, ret_inds);
			std::cout<<" - Prod: "<<pp<<", TS: "<<ret_inds[0];
			for (int i=1; i<ret_inds.size(); ++i) {
				std::cout<<", Dfa"<<i-1<<": "<<ret_inds[i];
			}
			std::cout<<"\n";
		}
	}

	// Test pre() with skipMe:
	{
		int p = Graph<float>::augmentedStateImage(node, graph_sizes);
		std::vector<int> set = {p};
		bool found_and_skipped = false;
		auto inclMe = [&found_and_skipped](int p) {
			if (p == 175) {
				found_and_skipped = true;
				return false;
			} else {
				return true;
			}
		};
		std::vector<int> pre_set = SymbolicMethods::pre(ts_eval, dfa_eval_ptrs, set, inclMe);
		std::cout<<"Pre set: "<<std::endl;
		for (auto pp : pre_set) {
			std::vector<int> ret_inds;
			Graph<float>::augmentedStatePreImage(graph_sizes, pp, ret_inds);
			std::cout<<" - Prod: "<<pp<<", TS: "<<ret_inds[0];
			for (int i=1; i<ret_inds.size(); ++i) {
				std::cout<<", Dfa"<<i-1<<": "<<ret_inds[i];
			}
			std::cout<<"\n";
		}
		std::cout<<"Found and skipped? "<<((found_and_skipped) ? "True" : "False")<<std::endl;
	}



	for (int i=0; i<dfa_eval_ptrs.size(); ++i) {
		delete dfa_eval_ptrs[i];
	}
	return 0;
}
