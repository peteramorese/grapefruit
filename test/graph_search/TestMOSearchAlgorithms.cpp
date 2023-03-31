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
#include "benchmark/RandomGraphGenerator.h"

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

int main(int argc, char* argv[]) {

	ArgParser parser(argc, argv);

	bool verbose = parser.hasFlag('v');
    
	uint32_t size = parser.parse<uint32_t>("size", 100);
	uint32_t max_edges_per_node = parser.parse<uint32_t>("max-edges-per-node", 5);
	uint32_t trials = parser.parse<uint32_t>("trials", 1);

	uint32_t seed;
    if (parser.hasKey("seed")) {
        seed = parser.parse<uint32_t>("seed", 0);
    } else {
        seed = RNG::randi(0, UINT32_MAX);
    }

    ASSERT(size, "Size must be greater than zero");
    ASSERT(max_edges_per_node, "Max number of edges per node must be greater than zero");

    RNG::seed(seed);

    if (verbose) LOG("Generating random graph...");

    RandomGraphGenerator random_graph_generator(size, max_edges_per_node);

    auto createRandomEdge = [](uint32_t min, uint32_t max) {
        return Edge(RNG::srandi(min, max), RNG::srandi(min, max));
    };

    for (uint32_t trial=0; trial<trials; ++trial) {

        LOG("b4 generate");
        std::shared_ptr<Graph<Edge>> graph = random_graph_generator.generate<Edge>(createRandomEdge);
        LOG("af generate");

        if (verbose) {
            LOG("Done.");
            graph->print(&Edge::edgeToStr);
        }

        //auto[start, goal] = random_graph_generator.getRandomInitAndGoal();
        Node start = 0;
        Node goal = random_graph_generator.getRandomGoal();
        if (verbose) LOG("Starting node: " << start << ", goal node: " << goal);
    
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
        } else {
            if (verbose) LOG("Neither algorithm found any solutions");
            continue;
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
    }

	return 0;
}
