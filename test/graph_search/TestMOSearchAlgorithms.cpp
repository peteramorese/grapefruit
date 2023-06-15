#include<iostream>
#include<string>
#include<sstream>
#include<memory>
#include<queue>

#include "tools/Logging.h"
#include "tools/Test.h"
#include "statistics/Random.h"
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
        Edge(uint32_t cost_1, uint32_t cost_2) : cv(cost_1, cost_2) {}
        bool operator==(const Edge& other) const {return cv == other.cv;}

    public:

        Containers::TypeGenericArray<uint32_t, uint32_t> cv;
        static Containers::TypeGenericArray<uint32_t, uint32_t> edgeToCostVector(const Edge& edge) {return edge.cv;}
        static std::string cvToStr(const Containers::TypeGenericArray<uint32_t, uint32_t>& cv) {return "(" + std::to_string(cv.template get<0>()) + ", " + std::to_string(cv.template get<1>()) + ")";}
        static std::string edgeToStr(const Edge& edge) {return "cost: " + cvToStr(edge.cv);}
};

int main(int argc, char* argv[]) {

	ArgParser parser(argc, argv);

	bool verbose = parser.parse<void>('v', "Run in verbose mode");

    bool boa_only = parser.parse<void>("boa-only", "Analyze BOA* only");
    bool namoa_only = parser.parse<void>("namoa-only", "Analyze NAMOA* only");
    ASSERT(!boa_only || !namoa_only, "Cannot specify boa only and namoa only");
    
	auto size = parser.parse<uint32_t>("size", 's', 100, "Random graph size");
	auto max_edges_per_node = parser.parse<uint32_t>("max-edges-per-node", 5, "Max number of edges to extend from");
	auto trials = parser.parse<uint32_t>("trials", 't', 1, "Number of randomized trials to run");
    auto seed_arg = parser.parse<uint32_t>("seed", 0, "Specify a certain seed for generating graph");

	uint32_t seed = (seed_arg) ? seed_arg.get() : RNG::randi(0, INT32_MAX);

	parser.enableHelp();

    ASSERT(size.get() > 0, "Size must be greater than zero");
    ASSERT(max_edges_per_node.get() > 0, "Max number of edges per node must be greater than zero");

    RNG::seed(seed);

    if (verbose) LOG("Generating random graph...");

    RandomGraphGenerator random_graph_generator(size.get(), max_edges_per_node.get());

    auto createRandomEdge = [](uint32_t min, uint32_t max) {
        LOG("min: " << min << " max: " << max);
        return Edge(RNG::srandi(min, max), RNG::srandi(min, max));
    };

    for (uint32_t trial=0; trial<trials.get(); ++trial) {

        std::shared_ptr<Graph<Edge>> graph = random_graph_generator.generate<Edge>(createRandomEdge);

        if (verbose) {
            LOG("Done.");
            graph->print(&Edge::edgeToStr);
        }

        //auto[start, goal] = random_graph_generator.getRandomInitAndGoal();
        Node start = 0;
        Node goal = random_graph_generator.getRandomGoal();
        if (verbose) LOG("Starting node: " << start << ", goal node: " << goal);
    
        MOQuantitativeGraphSearchProblem<Graph<Edge>, Containers::TypeGenericArray<uint32_t, uint32_t>, SearchDirection::Forward> problem(graph, {start}, {goal}, &Edge::edgeToCostVector);

        if (verbose) LOG("Searching BOA*...");
        auto boa_result = BOAStar<Containers::TypeGenericArray<uint32_t, uint32_t>, decltype(problem)>::search(problem);
        if (verbose) LOG("Done.");
        if (boa_only) {
            if (verbose) {
                for (uint32_t pt = 0; pt < boa_result.pf.size(); ++pt) {
                    LOG("Pareto point " << pt);
                    LOG("   BOA* Path cost: " << Edge::cvToStr(boa_result.pf[pt]));
                }
            }
            continue;
        }
        if (verbose) LOG("Searching NAMOA*...");
        auto namoa_result = NAMOAStar<Containers::TypeGenericArray<uint32_t, uint32_t>, decltype(problem)>::search(problem);
        if (verbose) LOG("Done.");
        if (namoa_only) {
            if (verbose) {
                uint32_t pt = 0;
                for (uint32_t pt = 0; pt < namoa_result.pf.size(); ++pt) {
                    LOG("Pareto point " << pt);
                    LOG("   NAMOA* Path cost: " << Edge::cvToStr(namoa_result.pf[pt]));
                }
            }
            continue;
        }


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
                const auto& boa_path_cost = boa_result.pf[pt];
                const auto& namoa_path_cost = namoa_result.pf[pt];

                if (verbose) {
                    LOG("Pareto point " << pt++);
                    LOG("   BOA* Path cost: " << Edge::cvToStr(boa_path_cost));
                    LOG("   NAMOA* Path cost: " << Edge::cvToStr(namoa_path_cost));
                }

                TEST_ASSERT(boa_path_cost == namoa_path_cost, "Path costs are not equal BOA*: " << Edge::cvToStr(boa_path_cost) << " NAMOA*: " << Edge::cvToStr(namoa_path_cost));

            }
        }
    }

	return 0;
}
