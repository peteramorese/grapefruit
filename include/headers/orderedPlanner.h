#pragma once

#include<functional>
#include<list>
#include<unordered_map>
#include<queue>
#include<memory>
#include "transitionSystem.h"
#include "graph.h"
#include "benchmark.h"

namespace SymbolicMethods {
    struct ConnectedNodes {std::vector<int> nodes; std::vector<WL*> data;};
    std::vector<int> getGraphSizes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas);
    std::vector<int> post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set);
    std::vector<int> pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set);
    std::vector<int> post(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
    std::vector<int> pre(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
    ConnectedNodes postNodes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
    ConnectedNodes preNodes(TransitionSystem<State>& ts, const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& set, const std::function<bool(int)>& inclMe);
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
                static const unsigned NEGLECTED = 0;
                static const unsigned ADDED = 1;
                static const unsigned UPDATED = 2;
                unsigned iterations;
                const Plan* getPlan(float mu_max) const;
                const Plan* getPlan(unsigned ind = 0) const;
                const std::list<ParetoPoint>* getParetoFront() const;
                unsigned addParetoPoint(float mu, float path_length, const Plan& plan);
                void printParetoFront() const;
                void clear();
        };
        struct CostToGoal {
            CostToGoal(unsigned sz = 0);
            void resize(unsigned sz);
            void clear();
            std::vector<float> cost;
            std::vector<bool> reachable;
            bool success;
        };
    private:
        // Member types:
        struct Node {
            Node(); 
            Node(int ind_, float cost_, float f_cost_, float mu, const std::vector<float>& cost_set_); 
            int ind; 
            float cost; 
            float f_cost; 
            float mu; 
            std::vector<float> cost_set;
        };
        struct ParentNode {
            int par_ind;
            std::string par_action;
        };
        struct VisitedNode {
            VisitedNode();
            VisitedNode(int ind_,  float mu_max_);
            bool operator==(const VisitedNode& vn_in) const;
            int ind;
            float mu_max;
        };
        friend std::hash<VisitedNode>; // Hash fxn
        using gsz = const std::vector<int>&;

        // Member variables:
        TransitionSystem<State>& ts;
        const bool verbose;
        bool success;
        Result result;
        std::pair<bool, std::vector<CostToGoal>> heuristic;
        // Benchmark:
        const std::string* bm_filepath;
        Benchmark bm;
        
        // Member functions:
        std::unique_ptr<Node> newNode();
        std::unique_ptr<Node> newNode(const Node& node);
        std::unique_ptr<Node> newNode(gsz graph_sizes, const std::vector<int>& inds, float cost, float f_cost, float mu_, const std::vector<float>& cost_set);
        int newNode(gsz graph_sizes, const std::vector<int>& inds, float cost, float f_cost, float mu, const std::vector<float>& cost_set, std::unordered_map<int, std::unique_ptr<Node>>& node_map);
        Node* newNode(const Node& node, std::unordered_map<int, std::unique_ptr<Node>>& node_map);
        Node* pruneBranch(std::unordered_map<VisitedNode, bool>& visited, std::unordered_map<int, bool>& seen, std::unordered_map<int, ParentNode>& parents, std::unordered_map<int, std::unique_ptr<Node>>& node_map, int curr_node, float mu_max, float prev_mu_max);
        static bool allAccepting(gsz graph_sizes, int p, const std::vector<DFA_EVAL*>& dfas);
        Plan extractPlan(gsz graph_sizes, int p_acc, int p_init, const std::unordered_map<int, ParentNode>& parents);
        bool generateHeuristic(const std::vector<DFA_EVAL*>& dfas);
        float getH(gsz graph_sizes, unsigned p) const;
    public:
        OrderedPlanner(TransitionSystem<State>& ts_, bool verbose_ = false, const std::string* bm_filepath_ = nullptr);
        bool search(const std::vector<DFA_EVAL*>& dfas, const std::function<float(const std::vector<float>&)>& setToMu, bool use_heuristic, bool single_query = false, float mu_sq = -1.0f);
        std::vector<int> BFS(const std::vector<DFA_EVAL*>& dfas);
        void searchBackwards(const std::vector<DFA_EVAL*>& dfas, const std::vector<int>& root_acc_nodes, CostToGoal& cost_to_goal);
        const Result* getResult() const;
};

