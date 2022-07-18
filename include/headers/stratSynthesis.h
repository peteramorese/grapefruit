#pragma once
#include "game.h"
#include "lexSet.h"


template<class T>
class RiskAvoidStrategy {
    private: 
        std::vector<int> pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set);
        std::vector<int> pre(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player);
        std::vector<int> post(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set);
        std::vector<int> post(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player);
        std::vector<int> post(Game<T>& game, DFA_EVAL* dfa, const std::vector<int>& graph_sizes, const std::vector<int>& set, unsigned evolve_player, std::vector<float>& transition_costs);
    public:
        typename Game<T>::Strategy synthesize(Game<T>& game, DFA_EVAL* dfa);
};