#include<functional>
#include "game.h"

template<class T>
class MockGamePlay {
	private:
		Game<T>* game;
		DFA_EVAL* dfa;
		const std::string NAME = " [MockGamePlay] ";
		const Game<State>::Strategy* strat;
        const int evolve_player;
        unsigned s_curr;
        int curr_player;
		std::vector<int> graph_sizes;
		std::vector<std::string> action_seq;
		//void environmentAction(const std::string& action);
		bool executeAction(const std::string& action); // returns acceptance on live dfa
		void reset();
		bool violating;
        std::function<bool(unsigned, unsigned)> violatingCondition;
	public:
		MockGamePlay(Game<T>* game_, DFA_EVAL* dfa_, const std::function<bool(unsigned, unsigned)>& violatingCondition_, int evolve_player_);
		void setStrategy(const typename Game<State>::Strategy* strat_);
		bool run();
		void writeToFile(const std::string& filename, const std::vector<std::string>& xtra_info);
};

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
    game.getConnectedNodes(s_curr, node_list);
    game.getConnectedData(s_curr, data_list);
    bool found_connection = false;
    int last_player = game.getState(sp).second;
    for (int i = 0; i<node_list.size(); ++i) {
        std::string temp_str = data_list[i]->label;
        if (temp_str == action) {
            int sp = node_list[i];
            int pp;
            if (game.getState(sp).second == evolve_player || evolve_player == -1) {
                const std::vector<std::string>* lbls = game.returnStateLabels(sp);
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
            curr_player = game.getState(sp).second;
            s_curr = sp;
            break;
        }
    }
	std::string action_lbl = action;
    action_lbl = action_lbl + std::to_string(last_player);
	action_seq.push_back(action_lbl);

	violating = violatingCondition(s_curr, dfa->getCurrNode());
	return (live_dfa->isCurrAccepting()) ? true : false;
}

template<class T>
void MockGamePlay<T>::reset() {
    s_curr = 0;
    curr_player = getState(s_curr).second;
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
			std::cout<<NAME<<"Violated safety specification!"<<std::endl;
			return false;
		}
		if (found) {
			std::cout<<"\n"<<NAME<<"Current State ("<<s_curr<<"): \n";
			game->getState.first->print();
			std::cout<<"\n";
			//TS->getState(TS->getCurrNode());
			std::cout<<NAME<<"--Player "<<curr_player<<" turn--\n";
			int prod_ind = Graph<float>::augmentedStateImage({s_curr, cosafe_dfa->getCurrNode(), live_dfa->getCurrNode()}, graph_sizes);
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
				std::cout<<NAME<<"Error: Prod state unreachable. TS: "<<TS->getCurrNode()<<" CoSafe: "<<cosafe_dfa->getCurrNode()<<" Live: "<<live_dfa->getCurrNode()<<std::endl;
			}
		}
		std::cout<<"\n"<<NAME<<"Current State ("<<s_curr<<"): \n";
		game->getState.first->print();
		std::cout<<"\n";
		//TS->getState(TS->getCurrNode());
		std::cout<<NAME<<"--Environment's turn--\n";
		std::vector<WL*> con_data;
		game->getConnectedData(s_curr, con_data);
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
				finished = executeAction(con_data[opt]->label);
				std::cout<<NAME<<"Action taken: "<<con_data[opt]->label<<std::endl;
			} else {
				std::cout<<NAME<<"Invalid option. Try again\n";
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