#pragma once

#include <set>
#include <unordered_set>

#include "core/Graph.h"

#include "graph_search/SearchProblem.h"
#include "graph_search/MultiObjectiveSearchProblem.h"
#include "graph_search/BOAStar.h"

#include "tools/Logging.h"


namespace TP {


namespace GraphSearch {

    using NormType = double;

    template <class COST_VECTOR_T, class COMBINED_COST_T = NormType>
    struct ObjectiveL1Norm {
        typedef COMBINED_COST_T combined_cost_t;
        static COMBINED_COST_T norm(const COST_VECTOR_T& cv) {
            //Containers::FixedArray<COST_VECTOR_T::size(), COMBINED_COST_T> input;
            COMBINED_COST_T cumulative_norm = COMBINED_COST_T{};
            auto extract_combined_cost = [&cumulative_norm]<typename T>(const T& element) {
                cumulative_norm += static_cast<COMBINED_COST_T>(element);
                return true;
            };

            cv.forEach(extract_combined_cost);
            return cumulative_norm;
        }
    };


    template <class COST_VECTOR_T, class SEARCH_PROBLEM_T, class HEURISTIC_T = ZeroHeuristic<Node, COST_VECTOR_T>, typename EDGE_STORAGE_T = typename SEARCH_PROBLEM_T::edge_t, class OBJECTIVE_NORM_T = ObjectiveL1Norm<COST_VECTOR_T>>
    class NAMOAStar {
        public:
            using GraphNode = SEARCH_PROBLEM_T::node_t;
            typedef SEARCH_PROBLEM_T::edge_t edge_t;
            typedef SEARCH_PROBLEM_T::cost_t cost_t;
            using EnumeratedNode = SEARCH_PROBLEM_T::node_t;

        private:
            struct OpenSetElement;

            using CostMapItem = typename NonDominatedCostMap<COST_VECTOR_T>::Item;

            struct OpenSetSortedElement {
                OpenSetSortedElement(GraphNode node_, const CostMapItem* g_score_) 
                    : node(node_)
                    , g_score(g_score_)
                    {}

                OpenSetSortedElement(GraphNode node_, const CostMapItem* g_score_, COST_VECTOR_T&& f_score_) 
                    : node(node_)
                    , g_score(g_score_)
                    , f_score(std::move(f_score_))
                    , cached_norm(OBJECTIVE_NORM_T::norm(f_score))
                    {}

                // Used as a tie to the unsorted_set
                GraphNode node;
                const CostMapItem* g_score;

                // Sorted values
                COST_VECTOR_T f_score; // f_score (g_score + h)
                OBJECTIVE_NORM_T::combined_cost_t cached_norm; // Cache the norm calculation

                bool operator<(const OpenSetSortedElement& other) const {
                    return cached_norm < other.cached_norm;
                }
            };

            struct OpenSetCheckElement {
                OpenSetCheckElement(OpenSetCheckElement&&) = default;
                OpenSetCheckElement(GraphNode node_, const CostMapItem* g_score_) 
                    : node(node_), g_score(g_score_) {}

                bool operator==(const OpenSetCheckElement& other) const {
                    return node == other.node && g_score == other.g_score;
                }

                void tie(std::set<OpenSetSortedElement>::iterator tie) const {sorted_element_tie = tie;}

                GraphNode node;
                const CostMapItem* g_score;
                mutable std::set<OpenSetSortedElement>::iterator sorted_element_tie;
            };
            

            struct OpenSetCheckElementHash {
                std::size_t operator()(const OpenSetCheckElement& e) const {
                    return std::hash<GraphNode>{}(e.node) ^ std::hash<const CostMapItem*>{}(e.g_score);
                }
            };

            class OpenSet {
                public:
                    bool insert(GraphNode node, const CostMapItem* g_score, COST_VECTOR_T&& f_score) {
                        LOG("inserting node: " << node << " ptr: " << g_score);
                        print();
                        auto check_it = check_set.find(OpenSetCheckElement(node, g_score));
                        if (check_it == check_set.end()) { // If not found, insert
                            // Insert
                            auto[check_it, check_inserted] = check_set.emplace(node, g_score);
                            LOG("check inserted: " << check_inserted);
                            auto sorted_it = sorted_set.emplace(node, g_score, std::move(f_score));

                            // Tie the check element to the sorted element
                            check_it->tie(sorted_it);
                            return true;
                        } else {
                            return false;
                        }
                    }

                    void erase(GraphNode node, const CostMapItem* g_score) {
                        LOG("erasing node: " << node << " ptr: " << g_score);
                        auto check_it = check_set.find(OpenSetCheckElement(node, g_score));
                        ASSERT(check_it != check_set.end(), "Element not found in open check set");

                        auto erased = sorted_set.erase(check_it->sorted_element_tie);
                        check_set.erase(check_it);
                    }

                    std::pair<GraphNode, const CostMapItem*> pop() {
                        LOG("b4 pop");
                        auto it = sorted_set.begin();
                        GraphNode node = it->node;
                        const CostMapItem* g_score = it->g_score;
                        LOG("popping node: " << node << " ptr: " << g_score);
                        erase(node, g_score);
                        LOG("af pop");
                        return {node, g_score};
                    }

                    void eraseDominated(const COST_VECTOR_T& test_cv) {
                        auto check_it = check_set.begin();
                        while (check_it != check_set.end()) {
                            if (test_cv.dominates(check_it->g_score->cv) == Containers::ArrayComparison::Dominates) {
                                auto erased = sorted_set.erase(check_it->sorted_element_tie);
                                check_it = check_set.erase(check_it);
                            } else {
                                ++check_it;
                            }
                        }
                    }

                    inline bool empty() const {return sorted_set.empty();}

                    // TODO remove
                    void print() const {
                        LOG("check set: ");
                        for (auto it = check_set.begin(); it != check_set.end(); ++it) LOG(" node: " << it->node << " g_score: " << it->g_score);
                        LOG("sorted set: ");
                        for (auto item : sorted_set) LOG(" node: " << item.node << " g_score: " << item.g_score);
                    }
                private:
                    std::unordered_set<OpenSetCheckElement, OpenSetCheckElementHash> check_set;
                    std::multiset<OpenSetSortedElement> sorted_set;
            };
        public:
            using CostMap = NonDominatedCostMap<COST_VECTOR_T>;
            using NAMOASearchResult = MultiObjectiveSearchResult<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>;

        public:
            static NAMOASearchResult search(const SEARCH_PROBLEM_T& problem);

        private:
            static void extractPaths(std::vector<std::pair<GraphNode, const CostMapItem*>>& goal_set, NAMOASearchResult& result, const SEARCH_PROBLEM_T& problem);
            static void extractPath(const GraphNode& goal_node, PathSolution<GraphNode, EDGE_STORAGE_T, COST_VECTOR_T>& path_solution, const PathEnumeratedNodeMap<GraphNode, EnumeratedNode, SearchGraphEdge<COST_VECTOR_T, EDGE_STORAGE_T>>& node_map);
    };

}
}



#include "NAMOAStar_impl.hpp"