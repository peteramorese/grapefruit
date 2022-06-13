#pragma once
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
		void setStrategy(const typename Game<T>::Strategy* strat_);
		bool run();
		void writeToFile(const std::string& filename, const std::vector<std::string>& xtra_info);
};