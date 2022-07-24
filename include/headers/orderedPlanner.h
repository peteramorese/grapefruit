#include<functional>
#include<list>
#include<unordered_map>
#include<queue>
#include<memory>
#include "transitionSystem.h"
#include "graph.h"

class SymbolicMethods {
    public:
        struct ConnectedNodes {std::vector<int> nodes; std::vector<WL*> data;};
        static std::vector<int> getGraphSizes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas);
        static std::vector<int> post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set);
        static std::vector<int> pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set);
        static std::vector<int> post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
        static std::vector<int> pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
        static ConnectedNodes postNodes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
        static ConnectedNodes preNodes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
};

class OrderedPlanner {
    public:
        struct Plan {
            std::vector<const State*> state_sequence;
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
                void printParetoFront() const;
        };
    private:
        struct Node {
            Node(); 
            Node(int ind_, float cost_, float f_cost_, const std::vector<float>& cost_set_); 
            int ind; 
            float cost; 
            float f_cost; 
            std::vector<float> cost_set;
        };
        TransitionSystem<State>* ts_ptr;
        const bool verbose;
        bool success;
        Result result;
        std::vector<int> graph_sizes;
        std::unique_ptr<Node> newNode();
        std::unique_ptr<Node> newNode(const Node& node);
        std::unique_ptr<Node> newNode(const std::vector<int>& inds, float cost, float f_cost, const std::vector<float>& cost_set);
        int newNode(const std::vector<int>& inds, float cost, float f_cost, const std::vector<float>& cost_set, std::unordered_map<int, std::unique_ptr<Node>>& node_map);
        Node* newNode(const Node& node, std::unordered_map<int, std::unique_ptr<Node>>& node_map);
        //std::vector<float> addCostSets(const std::vector<float>& set_1, const std::vector<float> set_2);
        Plan extractPlan(int p_acc, int p_init, const std::unordered_map<int, std::pair<int, std::string>>& parents);
    public:
        OrderedPlanner(bool verbose_ = false);
        bool search(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::function<float(const std::vector<float>&)>& setToMu, bool use_heuristic);
        const Result* getResult() const;
};

