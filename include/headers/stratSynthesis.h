#pragma once
#include "game.h"


template<class T>
class RiskAvoidStrategy {
    private: 
        std::vector<int> pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set);
        std::vector<int> pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player);
        std::vector<int> post(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set);
        std::vector<int> post(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player);
    public:
        struct Strategy {
            bool success;
            std::vector<std::string> policy; // Maps state index to action
            std::vector<bool> region; // Determines if the state is within the attractor 'O'
        };
        Strategy synthesize(Game<T>& game, DFA_EVAL* dfa);
};