#include<functional>
#include<list>
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
            std::vector<std::string> action_sequence;
        };
        class Result {
            public:
                struct ParetoPoint {float mu; float path_length; Plan plan;};
            private:
                std::list<ParetoPoint> pareto_front; // x: mu, y: pathlengths
            public:
                const Plan* getPlan(float mu_max) const;
                const Plan* getPlan(unsigned ind) const;
                const std::list<ParetoPoint>* getParetoFront() const;
                bool addParetoPoint(float mu, float path_length, const Plan& plan);
                void printParetoFront();
        };
    private:
        bool success;
        Result pr_front;
    public:
        bool search(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::function<float(std::vector<int>)>& setToMu);
        Result* getResult() const;
};

