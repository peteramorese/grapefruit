#include<array>

#include "graph.h"
#include "condition.h"
#include "stateSpace.h"
#include "state.h"
#include "game.h"
#include "stratSynthesis.h"
#include "mockGamePlay.h"


bool cardinalState(const int i, const int j, const int grid_size, std::vector<std::string>& ret_coords, int direction, const std::vector<std::string>& x_labels, const std::vector<std::string>& y_labels) {
	ret_coords.clear();
	ret_coords.resize(2);
	switch (direction) {
		case 0: // left 
			if (i != 0) {
				ret_coords[0] = x_labels[i-1];
				ret_coords[1] = y_labels[j];
				return true;
			} else {
				return false; // Stay put
			}
			break;
		case 1: // right
			if (i != (grid_size-1)) {
				ret_coords[0] = x_labels[i+1];
				ret_coords[1] = y_labels[j];
				return true;
			} else {
				return false;
			}

			break;
		case 2: // down
			if (j != 0) {
				ret_coords[0] = x_labels[i];
				ret_coords[1] = y_labels[j-1];
				return true;
			} else {
				return false;
			}

			break;
		case 3: // up
			if (j != (grid_size-1)) {
				ret_coords[0] = x_labels[i];
				ret_coords[1] = y_labels[j+1];
				return true;
			} else {
				return false;
			}
			break;
		default:
			std::cout<<"Error: Unrecognized direction!\n";
			return false;
	}
}




int main() {

	bool manual_setup = true;
	int grid_size = 3;
	bool verbose = false;
	bool use_benchmark = false;
	//std::string bm_filename_path = "./benchmark_data/preference_planner_bm.txt";
	//std::string plan_filename_path = "../../matlab_scripts/preference_planning_demos/plan_files/plan.txt";
	std::string dfa_filename_path_prefix = "../../spot_automaton_file_dump/dfas/";

	// These will be manually set if manual_setup = true
	int N_DFAs;
	float mu = 0;
	bool use_h_flag = false;
	bool write_file_flag = false;


	//////////////////////////////////////////////////////
	/* Create the Game for the Grid Robot  */
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
	SS_GRID_ROBOT.setStateDimension(x_labels, 0); // x_agent
	SS_GRID_ROBOT.setStateDimension(y_labels, 1); // y_agent
	SS_GRID_ROBOT.setStateDimension(x_labels, 2); // x_rogue
	SS_GRID_ROBOT.setStateDimension(y_labels, 3); // y_rogue

	// Label state space:
	SS_GRID_ROBOT.setStateDimensionLabel(0, "x_agent");
	SS_GRID_ROBOT.setStateDimensionLabel(1, "y_agent");
	SS_GRID_ROBOT.setStateDimensionLabel(2, "x_rogue");
	SS_GRID_ROBOT.setStateDimensionLabel(3, "y_rogue");

	// Create object location group:
	SS_GRID_ROBOT.setLabelGroup("agent_coords", {"x_agent", "y_agent"});
	SS_GRID_ROBOT.setLabelGroup("rogue_coords", {"x_rogue", "y_rogue"});

	// Set the initial state:
	State init_state(&SS_GRID_ROBOT);	
	init_state.setState({"x0", "y0", "x1", "y1"});

	Game<State> game(2, true, true); 
	game.setInitState(&init_state, 1);

	std::array<std::string, 4> agent_direction_labels = {"move_left", "move_right", "move_down", "move_up"};
	std::array<std::string, 4> rogue_direction_labels = {"r_move_left", "r_move_right", "r_move_down", "r_move_up"};
	int iters = 0;
	for (int i=0; i<grid_size; ++i) { // x_agent
	 	std::cout<<"Iterations: "<<iters<<std::endl;
		for (int ii=0; ii<grid_size; ++ii) { // y_agent
			for (int iii=0; iii<grid_size; ++iii) { // x_rogue
				for (int iv=0; iv<grid_size; ++iv) { // y_rogue
					iters++;
					State src(&SS_GRID_ROBOT);
					State dst(&SS_GRID_ROBOT);
					//std::cout<<"b4 set src"<<std::endl;
					src.setState({x_labels[i], y_labels[ii], x_labels[iii], y_labels[iv]});
					bool stay_put_incl = false;
					for (int agent_dir=0; agent_dir<4; ++agent_dir) {
						std::vector<std::string> new_coords;
						bool stay_put = !cardinalState(i, ii, grid_size, new_coords, agent_dir, x_labels, y_labels);
						if (!stay_put) {
							//std::cout<<"b4 set dst new_coords[0]: "<<new_coords[0]<<" new_coords[1]: "<<new_coords[1]<<std::endl;
							dst.setState({new_coords[0], new_coords[1], x_labels[iii], y_labels[iv]});
							game.connect(&src, 0, &dst, 1, 5.0f, agent_direction_labels[agent_dir]);
						} else if (!stay_put_incl) {
							game.connect(&src, 0, &src, 1, 0.0f, "stay_put");
							stay_put_incl = true;
						}
					}
					//int pause;
					//std::cin>>pause;
					stay_put_incl = false;
					game.connect(&src, 1, &src, 0, 0.0f, "no_intervention");
					for (int rogue_dir=0; rogue_dir<4; ++rogue_dir) {
						std::vector<std::string> new_coords;
						bool stay_put = !cardinalState(iii, iv, grid_size, new_coords, rogue_dir, x_labels, y_labels);
						if (!stay_put) {
							dst.setState({x_labels[i], y_labels[ii], new_coords[0], new_coords[1]});
							game.connect(&src, 1, &dst, 0, 0.0f, rogue_direction_labels[rogue_dir]);
						} 
					}
				}
			}
		}
	}
	game.finishConnecting();
	//game.print();



	std::vector<SimpleCondition> APs;
	std::vector<SimpleCondition*> AP_ptrs;
	for (int i=0; i<grid_size; ++i) {
		for (int ii=0; ii<grid_size; ++ii) {
			SimpleCondition temp_AP;
			temp_AP.addCondition(Condition::SIMPLE, Condition::LABEL, "x_agent", Condition::EQUALS, Condition::VAR, x_labels[i]);
			temp_AP.addCondition(Condition::SIMPLE, Condition::LABEL, "y_agent", Condition::EQUALS, Condition::VAR, y_labels[ii]);
			temp_AP.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
			temp_AP.setLabel("ap_" + x_labels[i] + "_" + y_labels[ii]);
			std::cout<<"made ap: "<<"ap_" + x_labels[i] + "_" + y_labels[ii]<<std::endl;
			APs.push_back(temp_AP);
		}
	}

	SimpleCondition AP_same_cell;
	AP_same_cell.addCondition(Condition::SIMPLE, Condition::LABEL, "x_agent", Condition::EQUALS, Condition::LABEL, "x_rogue");
	AP_same_cell.addCondition(Condition::SIMPLE, Condition::LABEL, "y_agent", Condition::EQUALS, Condition::LABEL, "y_rogue");
	AP_same_cell.setCondJunctType(Condition::SIMPLE, Condition::CONJUNCTION);
	AP_same_cell.setLabel("ap_same_cell");

	APs.push_back(AP_same_cell);

	// Get pointers after vector addresses wont be invalidated
	AP_ptrs.resize(APs.size());
	for (int i=0; i<APs.size(); ++i) {
		AP_ptrs[i] = &APs[i];
	}

	// Create the game:
	//Game<State> game(2, true); // by default, the init node for the ts is 0
	game.setPropositions(AP_ptrs);
	std::cout<<"\n\n Printing the Game: \n\n"<<std::endl;
	game.print();


	// Get the liveness specification:

	DFA A;
	A.readFileSingle("../../spot_automaton_file_dump/dfas/dfa_complete_liveness.txt");
	A.print();
	DFA_EVAL A_eval(&A);


	RiskAvoidStrategy<State> RAS;
	Game<State>::Strategy strat = RAS.synthesize(game, &A_eval);

	std::vector<int> graph_sizes(2);
	graph_sizes[0] = game.size();
	graph_sizes[1] = A.size();
	for (int i=0; i<strat.policy.size(); ++i) {
        std::vector<int> ret_inds;
        Graph<int>::augmentedStatePreImage(graph_sizes, i, ret_inds);
        int s = ret_inds[0]; // game state
		std::cout<<"action (s: "<<s<<", q: "<<ret_inds[1]<<", p: "<<i<<"): "<<strat.policy[i]<<std::endl;
	}

	auto violatingState = [](unsigned s, unsigned q){
		return false;//(q == 2) ? true : false;
	};

	MockGamePlay<State> mock(&game, &A_eval, violatingState, 0);
	mock.setStrategy(&strat);
	mock.run();


}
