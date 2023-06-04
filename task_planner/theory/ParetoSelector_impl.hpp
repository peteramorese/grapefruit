#pragma once

#include "ParetoSelector.h"

#include <cmath>

#include "statistics/Random.h"

namespace TP {

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator ParetoSelector<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>::uniformRandom(const ParetoFront& pf) {
    uint32_t rand_ind = RNG::randi(0, pf.solution_set.size());
    return std::next(pf.solution_set.begin(), rand_ind);
}

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator ParetoSelector<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>::TOPSIS(const ParetoFront& pf) {
    constexpr std::size_t dim = COST_VECTOR_T::size(); 
    COST_VECTOR_T ideal_soln;

    for (const auto& soln : pf.solution_set) {
        const COST_VECTOR_T& path_cost = soln.path_cost;
        ideal_soln.forEachWithI(
            [&path_cost] <typename T, uint32_t I> (T& e) {
            if (path_cost.template get<I>() > e)
                e =  path_cost.template get<I>();
            return true;
        });
    }

    float min_similarity = 0.0f;
    typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator min_it = pf.solution_set.begin();
    for (auto soln_it = pf.solution_set.begin(); soln_it != pf.solution_set.end(); ++soln_it) {
        float d_worst, d_best;
        auto findDistances = [&] <typename T, uint32_t I> (T& e) {
            d_worst += std::pow(soln_it->path_cost.template get<I>(), 2);
            d_best += std::pow((ideal_soln.template get<I>() - soln_it->path_cost.template get<I>()), 2);
            return true;
        };
            
        ideal_soln.forEachWithI(findDistances);
        d_worst = std::sqrt(d_worst);
        d_best = std::sqrt(d_best);

        float similarity = -d_worst / (d_worst + d_best);
        if (similarity < min_similarity) {
            min_similarity = similarity;
            min_it = soln_it;
        }
    }
    return min_it;
}

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator ParetoSelector<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>::scalarWeights(const ParetoFront& pf, const Containers::FixedArray<COST_VECTOR_T::size(), float>& weights) {
    float min_weighted_sum = 0.0f;
    typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator min_it = pf.solution_set.begin();
    for (auto soln_it = pf.solution_set.begin(); soln_it != pf.solution_set.end(); ++soln_it) {
        float weighted_sum = 0.0f;
        auto weightSoln = [&weighted_sum, &weights] <typename T, uint32_t I> (const T& e) {
            weighted_sum -= weights.template get<I>() * e;
            return true;
        };
        soln_it->path_cost.forEachWithI(weightSoln);
        if (weighted_sum < min_weighted_sum) {
            min_weighted_sum = weighted_sum;
            min_it = soln_it;
        }
    }
    return min_it;
}

}