#include<functional>
#include "transitionSystem.h"
#include "graph.h"

class SymbolicMethods {
    public:
        static std::vector<int> getGraphSizes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas);
        static std::vector<int> post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set);
        static std::vector<int> pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set);
        static std::vector<int> post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
        static std::vector<int> pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
};

class OrderedPlanner {
    public:
        struct Plan {
            std::vector<State> state_sequence;
            std::vector<std::string> state_sequence;
        };
        class Result {
            private:
                std::vector<float> mu_vals;
                std::vector<float> path_lengths;
                std::vector<Plan> plans;
            public:
                const Plan* getPlan(float mu_max) const;
                const Plan* getPlan(unsigned ind) const;
                std::vector<std::pair<float, float>> getParetoFront() const;
                void pushParetoPoint(float mu, float path_length, const Plan& plan);
        };
    private:
        bool success;
        Result pr_front;
    public:
        bool search(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::function<float(std::vector<int>)>& setToMu);
        Result* getResult() const;
};

