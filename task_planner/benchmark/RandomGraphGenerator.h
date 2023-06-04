#pragma once

#include<memory>
#include<queue>

#include "core/Graph.h"
#include "statistics/Random.h"

namespace TP {

class RandomGraphGenerator {
    public:
        RandomGraphGenerator(uint32_t size, uint32_t max_edges_per_node, uint32_t max_cost = 10)
            : m_size(size)
            , m_max_edges_per_node(max_edges_per_node)
            , m_max_cost(max_cost)
            , m_back_node(0)
        {}

        // Takes in lambda that creates a random edge with a maximum cost
        template <class EDGE_T, typename LAM>
        std::shared_ptr<Graph<EDGE_T>> generate(LAM createRandomEdge) {
            reset();
            std::shared_ptr<Graph<EDGE_T>> graph(std::make_shared<Graph<EDGE_T>>());
            std::queue<Node> q;
            q.push(newNode());
            while (m_back_node < m_size) {
                Node src = q.front();
                q.pop();

                uint32_t n_edges = RNG::srandi(1, m_max_edges_per_node);
                for (uint32_t i = 0; i < n_edges; ++i) {
                    Node dst = newNode();
                    graph->connect(src, dst, createRandomEdge(0, m_max_cost));
                    q.push(dst);
                }
            }
            return graph;
        }    

        inline void reset() {
            m_back_node = 0;
        }

        inline Node newNode() {
            return RNG::srandi(0, ++m_back_node);
        }

        inline std::pair<Node, Node> getRandomInitAndGoal() const {
            Node init = RNG::srandi(0, m_back_node);
            Node goal = RNG::srandi(0, m_back_node);
            while (init == goal) {goal = RNG::srandi(0, m_back_node);}
            return {init, goal};
        }

        inline Node getRandomGoal() const {
            Node goal = RNG::srandi(0, m_back_node);
            while (goal == 0) {goal = RNG::srandi(0, m_back_node);}
            return goal;
        }

    private:
        const uint32_t m_size, m_max_edges_per_node, m_max_cost;
        Node m_back_node;
};

}