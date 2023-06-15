#pragma once

#include <vector>
#include "tools/Algorithms.h"

namespace TP {

template <class COST_VECTOR_T>
class ParetoFront {
    public:
        inline void reserve(std::size_t sz) const noexcept {m_pf.reserve(sz);}
        inline std::size_t size() const noexcept {return m_pf.size();}

        inline void push_back(const COST_VECTOR_T& cv) {
            m_pf.push_back(cv);
        }

        inline void push_back(COST_VECTOR_T&& cv) {
            m_pf.push_back(std::move(cv));
        }

        inline COST_VECTOR_T& operator[](std::size_t i) {return m_pf[i];}
        inline const COST_VECTOR_T& operator[](std::size_t i) const {return m_pf[i];}

        typename Algorithms::Sorting::Permutation sort() {
            auto lexComparison = [](const COST_VECTOR_T& lhs, const COST_VECTOR_T& rhs) -> bool {
                return lhs.lexicographicLess(rhs);
            };
            Algorithms::Sorting::Permutation perm = Algorithms::Sorting::getSortedPermutation(m_pf, lexComparison);
            Algorithms::Sorting::applyPermutation(m_pf, perm);
            return perm;
        }

        inline typename std::vector<COST_VECTOR_T>::const_iterator begin() const {return m_pf.begin();}
        inline typename std::vector<COST_VECTOR_T>::const_iterator end() const {return m_pf.end();}

        // Pareto regret (min epsilon such that the point is non dominated by any point on the Pareto front)
        float regret(const COST_VECTOR_T& point) const {

            float epsilon = 0.0f;
            COST_VECTOR_T p_best = point;
            for (const auto& cv : m_pf) {
                bool dominates = false;
                float epsilon_i_min = 0.0f;

                auto getEpsilon = [&]<typename T, uint32_t I>(const T& e){
                    float epsilon_i = point.template get<I>() - e;
                    dominates = epsilon_i < 0.0f;
                    if (epsilon_i < epsilon_i_min || I == 0) {
                        epsilon_i_min = epsilon_i;
                    }
                    return !dominates;
                };

                cv.forEachWithI(getEpsilon);

                // If the input point dominates the Pareto point, disregard it
                if (dominates)
                    continue;

                // Create a new point that is moved epsilon distance (on the Pareto front)
                // to check if it is dominated 
                COST_VECTOR_T p_test = point;
                p_test.forEach([&]<typename T>(T& e){e -= epsilon_i_min; return true;});

                if (p_test.dominates(p_best) == TP::Containers::ArrayComparison::Dominates) {
                    p_best = p_test;
                    epsilon = epsilon_i_min;
                }
                
            }
            return epsilon;
        }

    private:
        std::vector<COST_VECTOR_T> m_pf;
};
}