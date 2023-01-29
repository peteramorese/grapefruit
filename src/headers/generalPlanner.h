#pragma once
#include "transitionSystem.h"

template<class T>
class GeneralPlanner {
    private:
    public:
        std::vector<int> post(TransitionSystem<T>& ts, std::vector<DFA_EVAL*> dfas, const std::vector<int>& set);

};