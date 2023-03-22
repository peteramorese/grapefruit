#include<iostream>
#include<string>
#include<sstream>
#include<memory>
#include<queue>

#include "tools/Logging.h"
#include "tools/Test.h"
#include "tools/Random.h"
#include "core/Graph.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "graph_search/NAMOAStar.h"
#include "graph_search/BOAStar.h"

#include "tools/ArgParser.h"

using namespace TP;
using namespace TP::GraphSearch;

struct Edge {

    public:
        Edge(uint32_t cost_1, uint32_t cost_2) : cv({cost_1, cost_2}) {}
        bool operator==(const Edge& other) const {return cv == other.cv;}

    public:

        Containers::FixedArray<2, uint32_t> cv;
        static Containers::FixedArray<2, uint32_t> edgeToCostVector(const Edge& edge) {return edge.cv;}
        static std::string cvToStr(const Containers::FixedArray<2, uint32_t>& cv) {return "(" + std::to_string(cv[0]) + ", " + std::to_string(cv[1]) + ")";}
        static std::string edgeToStr(const Edge& edge) {return "cost: " + cvToStr(edge.cv);}
};

class RandomGraphGenerator {
    public:
        RandomGraphGenerator(uint32_t size, uint32_t max_edges_per_node, uint32_t max_cost = 10)
            : m_size(size)
            , m_max_edges_per_node(max_edges_per_node)
            , m_max_cost(max_cost)
        {}

        std::shared_ptr<Graph<Edge>> generate() {
            std::shared_ptr<Graph<Edge>> graph(std::make_shared<Graph<Edge>>(true, true, &Edge::edgeToStr));
            std::queue<Node> q;
            q.push(newNode());
            while (graph->size() < m_size) {
                Node src = q.front();
                q.pop();

                uint32_t n_edges = RNG::srandi(0, m_max_edges_per_node);
                for (uint32_t i = 0; i < n_edges; ++i) {
                    Node dst = newNode();
                    graph->connect(src, dst, Edge(randomCost(), randomCost()));
                    q.push(dst);
                }
            }
            return graph;
        }    

        inline Node newNode() {
            Node new_node = RNG::srandi(0, m_size);
            m_nodes_in_graph.push_back(new_node);
            return new_node;
        }

        inline uint32_t randomCost() const {
            return RNG::srandi(0, m_max_cost);
        }

        inline std::pair<Node, Node> getRandomInitAndGoal() const {
            Node init = m_nodes_in_graph[RNG::srandi(0, m_nodes_in_graph.size() - 1)];
            Node goal = m_nodes_in_graph[RNG::srandi(0, m_nodes_in_graph.size() - 1)];
            while (init == goal) {goal = m_nodes_in_graph[RNG::srandi(0, m_nodes_in_graph.size() - 1)];}
            return {init, goal};
        }

    private:
        uint32_t m_size, m_max_edges_per_node, m_max_cost;
        std::vector<Node> m_nodes_in_graph;
};

int main(int argc, char* argv[]) {

	ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v');
    
	uint32_t size = parser.parseAsUnsignedInt("size", 100);
	uint32_t max_edges_per_node = parser.parseAsUnsignedInt("max-edges-per-node", 10);
	uint32_t trials = parser.parseAsUnsignedInt("trials", 1);
	uint32_t seed = parser.parseAsUnsignedInt("seed", 0);

    ASSERT(size, "Size must be greater than zero");
    ASSERT(max_edges_per_node, "Max number of edges per node must be greater than zero");

    RNG::seed(seed);

    if (verbose) LOG("Generating random graph...");

    RandomGraphGenerator random_graph_generator(size, max_edges_per_node);
    std::shared_ptr<Graph<Edge>> graph = random_graph_generator.generate();

    if (verbose) graph->print();

    auto[start, goal] = random_graph_generator.getRandomInitAndGoal();
 
    MOQuantitativeGraphSearchProblem<Graph<Edge>, Containers::FixedArray<2, uint32_t>, SearchDirection::Forward> problem(graph, {start}, {goal}, &Edge::edgeToCostVector);

    if (verbose) LOG("Searching BOA*...");
    auto boa_result = BOAStar<Containers::FixedArray<2, uint32_t>, decltype(problem)>::search(problem);
    if (verbose) LOG("Searching NAMOA*...");
    auto namoa_result = NAMOAStar<Containers::FixedArray<2, uint32_t>, decltype(problem)>::search(problem);
    if (verbose) LOG("Done.");


    if (boa_result.success) {
        TEST_ASSERT_FATAL(namoa_result.success, "BOA* found solutions when NAMOA* did not");
    } else if (namoa_result.success) {
        TEST_ASSERT_FATAL(boa_result.success, "NAMOA* found solutions when BOA* did not");
    }

    bool pf_size_equal = false;
    if (boa_result.solution_set.size() == namoa_result.solution_set.size()) {
        pf_size_equal = true;
    } else {
        TEST_ASSERT(false, "Did not find the same number of pareto points");
    }
    
    if (pf_size_equal) {
        auto boa_it = boa_result.solution_set.begin();
        auto namoa_it = namoa_result.solution_set.begin();
        uint32_t pt = 0;
        for (; boa_it != boa_result.solution_set.end();) {
            const auto& boa_path_cost = boa_it->path_cost;
            const auto& namoa_path_cost = boa_it->path_cost;

            if (verbose) {
                LOG("Pareto point " << pt++);
                LOG("   BOA* Path cost: " << Edge::cvToStr(boa_path_cost));
                LOG("   NAMOA* Path cost: " << Edge::cvToStr(namoa_path_cost));
            }

            TEST_ASSERT(boa_path_cost == namoa_path_cost, "Path costs are not equal BOA*: " << Edge::cvToStr(boa_path_cost) << " NAMOA*: " << Edge::cvToStr(namoa_path_cost));

            ++boa_it;
            ++namoa_it;
        }

    }

	return 0;
}
