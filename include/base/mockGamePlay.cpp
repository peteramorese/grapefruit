#include<iostream>
#include<fstream>
#include "mockGamePlay.h"

/* CLASS DEFINITION */
template<class T>
MockGamePlay<T>::MockGamePlay(Game<T>* game_, DFA_EVAL* dfa_, const std::function<bool(unsigned, unsigned)>& violatingCondition_, int evolve_player_) : game(game_), dfa(dfa_), violatingCondition(violatingCondition_), evolve_player(evolve_player_) {
	std::cout<<NAME<<"Setting up Mock Game Play...\n";
	graph_sizes.resize(2);
	graph_sizes[0] = game->size();
	graph_sizes[1] = dfa->getDFA()->size();
	violating = false;
	action_seq.clear();
}

//void StrategyRTEVAL::environmentAction(const std::string& action) {
//	TS->eval(action, true);	
//	cosafe_dfa->eval(action, true);	
//	live_dfa->eval(action, true);	
//}

template<class T>
bool MockGamePlay<T>::executeAction(const std::string& action) {

 
    //std::vector<int> ret_inds;
    //Graph<int>::augmentedStatePreImage(graph_sizes, p, ret_inds);
    //int s = ret_inds[0]; // game state
    //int q = ret_inds[1]; // automaton state
    std::vector<int> node_list;
    std::vector<WL*> data_list;
    game->getConnectedNodes(s_curr, node_list);
    game->getConnectedData(s_curr, data_list);
    bool found_connection = false;
    int last_player = game->getState(s_curr).second;
    for (int i = 0; i<node_list.size(); ++i) {
        std::string temp_str = data_list[i]->label;
        if (temp_str == action) {
            int sp = node_list[i];
            int pp;
            if (game->getState(sp).second == evolve_player || evolve_player == -1) {
                const std::vector<std::string>* lbls = game->returnStateLabels(sp);
                for (auto& lbl : (*lbls)) {
                    if (dfa->eval(lbl, true)) {
                        found_connection = true;
                        break;
                    }
                }
                if (!found_connection) {
                    std::cout<<"Error (executeAction): Did not find connectivity in DFA.\n";
                    return false;
                }
            } 
            curr_player = game->getState(sp).second;
            s_curr = sp;
            break;
        }
    }
	std::string action_lbl = action;
    action_lbl = action_lbl + std::to_string(last_player);
	action_seq.push_back(action_lbl);

	violating = violatingCondition(s_curr, dfa->getCurrNode());
	return (dfa->isCurrAccepting()) ? true : false;
}

template<class T>
void MockGamePlay<T>::reset() {
    s_curr = 0;
    curr_player = game->getState(s_curr).second;
	dfa->reset();
	action_seq.clear();
	violating = false;
}

template<class T>
void MockGamePlay<T>::setStrategy(const typename Game<T>::Strategy* strat_) {
	strat = strat_;
}

template<class T>
bool MockGamePlay<T>::run() {
	std::cout<<NAME<<"Running... (Quit: 'q', Reset: 'r')\n";
	bool finished = false;
	bool found = true;
	reset();
	while (!finished) {
		if (violatingCondition(s_curr, dfa->getCurrNode())) {
			std::cout<<NAME<<"Violated!"<<std::endl;
			return false;
		}
        std::cout<<"\n"<<NAME<<"Current State (s: "<<s_curr<<", q: "<<dfa->getCurrNode()<<"): \n";
        game->getState(s_curr).first->print();
        std::cout<<"\n";
        std::cout<<NAME<<"--Player "<<curr_player<<" turn--\n";
		if (curr_player == 0) {
			int prod_ind = Graph<float>::augmentedStateImage({static_cast<int>(s_curr), dfa->getCurrNode()}, graph_sizes);
			std::string act;
			if (strat->region[prod_ind]) {
				act = strat->policy[prod_ind];
				if (act == "") {
					std::cout<<NAME<<"Error: Empty action in strategy\n";
                    return false;
				} else {
					std::cout<<NAME<<"Found and executed action in strategy: "<<act<<"\n";
					finished = executeAction(act);
				}
			} else {
				std::cout<<NAME<<"Error: Prod state unreachable. Game: "<<s_curr<<" DFA: "<<dfa->getCurrNode()<<std::endl;
                return false;
			}
		} else {
            std::vector<int> con_nodes;
            game->getConnectedNodes(s_curr, con_nodes);
            std::vector<WL*> con_data;
            game->getConnectedData(s_curr, con_data);
            for (int i=0; i<con_data.size(); ++i) {
                std::cout<<NAME<<" Opt "<<i<<": "<<con_data[i]->label<<std::endl;
            }
            std::cout<<NAME<<"Enter 's' to manually enter state variables"<<std::endl;
            std::cout<<NAME<<"--: ";
            std::string input;
            std::cin >> input;
            found = false;
            if (input == "q") {
                return false;
            } else if (input == "r") {
                found = true;
                reset();
            } else if (input == "s") {
                std::cout<<NAME<<"Custom state input mode:\n";
                auto SS_ptr = game->getState(0).first->getSS();
                int state_space_dim = SS_ptr->getDim();
                std::vector<std::string> state_vars(state_space_dim);
                for (int i=0; i<state_space_dim; ++i) {
                    std::cout<<NAME<<"  Dim "<<i<<" --: ";
                    std::string input_var;
                    std::cin >> input_var;
                    state_vars[i] = input_var;
                }
                State state(SS_ptr);
                state.setState(state_vars);
                bool state_exists = false;
                for (int i=0; i<con_nodes.size(); ++i) {
                    if (*(game->getState(con_nodes[i]).first) == state){
                        std::cout<<NAME<<"State found! (State: "<<con_nodes[i]<<")\n";
                        finished = executeAction(con_data[i]->label);
                        state_exists = true;
                        break;
                    }
                }
                if (!state_exists) {
                    std::cout<<NAME<<"Cannot transition to entered custom state! Try again. \n";
                }
            } else {
                std::string::size_type sz;
                int opt = std::stoi(input, &sz);
                if (opt >= 0 && opt<con_data.size()) {
                    found = true;
                    finished = executeAction(con_data[opt]->label);
                    std::cout<<NAME<<"Action taken: "<<con_data[opt]->label<<std::endl;
                } else {
                    std::cout<<NAME<<"Invalid option. Try again\n";
                }
            }
        }
	}
	return finished;
}

template<class T>
void MockGamePlay<T>::writeToFile(const std::string& filename, const std::vector<std::string>& xtra_info) {
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


template class MockGamePlay<State>;