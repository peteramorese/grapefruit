#pragma once

#include<memory>
#include<queue>

#include "core/Graph.h"

template <class EDGE_T>
class RandomGraphGenerator {
    public:
        RandomGraphGenerator(uint32_t size, uint32_t max_edges_per_node, uint32_t max_cost = 10)
            : m_size(size)
            , m_max_edges_per_node(max_edges_per_node)
            , m_max_cost(max_cost)
            , m_back_node(0)
        {}

        // Takes in lambda that creates a random edge with a maximum cost
        template <typename LAM>
        std::shared_ptr<Graph<EDGE_T>> generate(LAM createRandomEdge, Graph<EDGE_T>::EdgeToStrFunction edgeToStr = nullptr) {
            std::shared_ptr<Graph<EDGE_T>> graph(std::make_shared<Graph<Edge>>(true, true, edgeToStr));
            std::queue<Node> q;
            q.push(newNode());
            //while (graph->size() < m_size) {
            while (m_back_node < m_size) {
                Node src = q.front();
                q.pop();

                uint32_t n_edges = RNG::srandi(0, m_max_edges_per_node);
                for (uint32_t i = 0; i < n_edges; ++i) {
                    Node dst = newNode();
                    graph->connect(src, dst, createRandomEdge(0, m_max_cost));
                    q.push(dst);
                }
            }
            return graph;
        }    

        inline Node newNode() {
            return RNG::srandi(0, ++m_back_node);
        }

        inline uint32_t randomCost() const {
            return RNG::srandi(0, m_max_cost);
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
        uint32_t m_size, m_max_edges_per_node, m_max_cost;
        Node m_back_node;
};
