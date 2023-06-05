#pragma once

#include "ParetoSelector.h"

#include <cmath>

#include "statistics/Random.h"

namespace TP {

template <class COST_VECTOR_T>
typename std::size_t ParetoSelector<COST_VECTOR_T>::uniformRandom(const ParetoFront<COST_VECTOR_T>& pf) {
    return RNG::randi(0, pf.size());
}

template <class COST_VECTOR_T>
typename std::size_t ParetoSelector<COST_VECTOR_T>::TOPSIS(const ParetoFront<COST_VECTOR_T>& pf) {
    constexpr std::size_t dim = COST_VECTOR_T::size(); 
    COST_VECTOR_T worst_soln;

    //LOG("Beginning topsis w " << pf.size() <<" points");

    //auto printcv = [](const COST_VECTOR_T& cv) {
    //    LOG(" [0]: " << cv[0] << " [1]: " << cv[1]);
    //};

    for (const auto& pt : pf) {

        //LOG("path cost: ");
        //printcv(path_cost);

        worst_soln.forEachWithI(
            [&pt] <typename T, uint32_t I> (T& e) {
            if (pt.template get<I>() > e)
                e = pt.template get<I>();
            return true;
        });
    }

    //LOG("\nworst soln: ");
    //printcv(worst_soln);

    float min_similarity = 0.0f;
    //typename std::list<GraphSearch::PathSolution<NODE_T, EDGE_STORAGE_T, COST_VECTOR_T>>::const_iterator min_it = pf.begin();
    std::size_t min_ind = 0;
    for (std::size_t i = 0; i < pf.size(); ++i) {
        float d_worst, d_best;
        auto findDistances = [&] <typename T, uint32_t I> (T& e) {
            d_best += std::pow(pf[i].template get<I>(), 2);
            d_worst += std::pow((worst_soln.template get<I>() - pf[i].template get<I>()), 2);
            return true;
        };
            
        worst_soln.forEachWithI(findDistances);
        d_worst = std::sqrt(d_worst);
        d_best = std::sqrt(d_best);

        float similarity = -d_worst / (d_worst + d_best);
        if (similarity < min_similarity) {
            min_similarity = similarity;
            min_ind = i;
        }
    }
    //PAUSE;
    return min_ind;
}

template <class COST_VECTOR_T>
typename std::size_t ParetoSelector<COST_VECTOR_T>::scalarWeights(const ParetoFront<COST_VECTOR_T>& pf, Containers::FixedArray<COST_VECTOR_T::size(), float> weights) {
    auto norm = [] (COST_VECTOR_T& cv) {
        float norm = 0.0f;
        cv.forEachWithI(
            [&norm] <typename T, uint32_t I> (T& e) {
            norm += static_cast<float>(e) * static_cast<float>(e);
            return true;
        });
        norm = std::sqrt(norm);
        cv.forEachWithI(
            [&norm] <typename T, uint32_t I> (T& e) {
            e /= norm;
            return true;
        });
    };

    norm(weights);

    // Minimize the angle between the normalized pareto vector and the normalized weight vector
    float max_weighted_sum = 0.0f;
    std::size_t max_ind = 0;
    for (std::size_t i = 0; i < pf.size(); ++i) {
        COST_VECTOR_T normalized_path_cost = pf[i]; // copy it out to normalize it
        norm(normalized_path_cost);
        float weighted_sum = 0.0f;
        auto weightSoln = [&weighted_sum, &weights] <typename T, uint32_t I> (const T& e) {
            weighted_sum += weights.template get<I>() * e;
            return true;
        };
        normalized_path_cost.forEachWithI(weightSoln);
        if (weighted_sum > max_weighted_sum) {
            max_weighted_sum = weighted_sum;
            max_ind = i;
        }
    }
    return max_ind;
}

}