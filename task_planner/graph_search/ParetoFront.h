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

    private:
        std::vector<COST_VECTOR_T> m_pf;
};
}