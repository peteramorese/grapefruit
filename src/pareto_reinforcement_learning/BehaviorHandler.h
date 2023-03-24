#pragma once

#include <memory>

#include "TaskPlanner.h"

namespace PRL {

    template <class...T_ARGS>
    struct Behavior {
        Behavior() {
            auto setAllToDefaultPrior = []<typename T>(const T& behavior) {behavior.setToDefaultPrior();};
            m_behavior_criteria.forEach(setAllToDefaultPrior);
        }

        Behavior(T_ARGS&&...priors) 
            : m_behavior_criteria(std::forward<T_ARGS>(priors)...) 
        {}

        TP::Containers::TypeGenericArray<T_ARGS...> m_behavior_criteria;
    };

    class BehaviorHandler {
        public:
            struct StateActionPair {

            };
        public:
            BehaviorHandler(const std::shared_ptr<TP::DiscreteModel::TransitionSystem>& ts);
            


        private:
            std::shared_ptr<TP::DiscreteModel::TransitionSystem> m_ts;
    }
}