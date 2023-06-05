#pragma once

#include "ParetoSelector.h"

#include <cmath>

#include "statistics/Random.h"

namespace TP {

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator ParetoSelector<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>::uniformRandom(const ParetoFront& pf) {
    int rand_ind = RNG::randi(0, pf.size());
    return std::next(pf.begin(), rand_ind);
}

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator ParetoSelector<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>::TOPSIS(const ParetoFront& pf) {
    constexpr std::size_t dim = COST_VECTOR_T::size(); 
    COST_VECTOR_T worst_soln;

    //LOG("Beginning topsis w " << pf.size() <<" points");

    //auto printcv = [](const COST_VECTOR_T& cv) {
    //    LOG(" [0]: " << cv[0] << " [1]: " << cv[1]);
    //};

    for (const auto& soln : pf) {
        const COST_VECTOR_T& path_cost = soln.path_cost;

        //LOG("path cost: ");
        //printcv(path_cost);

        worst_soln.forEachWithI(
            [&path_cost] <typename T, uint32_t I> (T& e) {
            if (path_cost.template get<I>() > e)
                e =  path_cost.template get<I>();
            return true;
        });
    }

    //LOG("\nworst soln: ");
    //printcv(worst_soln);

    float min_similarity = 0.0f;
    typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator min_it = pf.begin();
    for (auto soln_it = pf.begin(); soln_it != pf.end(); ++soln_it) {
        float d_worst, d_best;
        auto findDistances = [&] <typename T, uint32_t I> (T& e) {
            d_best += std::pow(soln_it->path_cost.template get<I>(), 2);
            d_worst += std::pow((worst_soln.template get<I>() - soln_it->path_cost.template get<I>()), 2);
            return true;
        };
            
        worst_soln.forEachWithI(findDistances);
        d_worst = std::sqrt(d_worst);
        d_best = std::sqrt(d_best);

        float similarity = -d_worst / (d_worst + d_best);
        if (similarity < min_similarity) {
            min_similarity = similarity;
            min_it = soln_it;
        }
    }
    //PAUSE;
    return min_it;
}

template <class NODE_T, class EDGE_STORAGE_T, class COST_VECTOR_T>
typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator ParetoSelector<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>::scalarWeights(const ParetoFront& pf, const Containers::FixedArray<COST_VECTOR_T::size(), float>& weights) {
    // Maximize
    float max_weighted_sum = 0.0f;
    typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator max_it = pf.begin();
    for (auto soln_it = pf.begin(); soln_it != pf.end(); ++soln_it) {
        float weighted_sum = 0.0f;
        auto weightSoln = [&weighted_sum, &weights] <typename T, uint32_t I> (const T& e) {
            LOG("element: " << e);
            weighted_sum += weights.template get<I>() * e;
            return true;
        };
        soln_it->path_cost.forEachWithI(weightSoln);
        LOG("weighted sum: " << weighted_sum);
        if (weighted_sum > max_weighted_sum) {
            LOG("--selected");
            max_weighted_sum = weighted_sum;
            max_it = soln_it;
        }
    }
    return max_it;
}

}