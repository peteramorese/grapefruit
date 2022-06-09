#pragma once
#include "game.h"


class RiskAvoidStrategy {
    private: 
    public:
        struct Strategy {
            bool success;
            std::vector<std::string> policy; // Maps state index to action
            std::vector<bool> region; // Determines if the state is within the attractor 'O'
        };
        template<typename T> std::vector<int> pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set);
        template<typename T> std::vector<int> pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player);
        template<typename T> Strategy synthesize(Game<T>& game, DFA_EVAL* dfa);
};