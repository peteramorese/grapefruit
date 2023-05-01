#pragma once

#include <cmath>

namespace TP {
namespace ML {

class UCB {
    public:
        UCB() = delete;
        UCB(float confidence) 
            : m_confidence(confidence)
            , m_n(0u)
        {}

        inline void pull() { ++m_n; }
        inline float get(float exploitation_estimate, uint32_t k) const { return exploitation_estimate + explorationBonus(k); }
        inline float explorationBonus(uint32_t k) const {
            return m_confidence * std::sqrt(std::log(k) / m_n);
        }
    private:
        float m_confidence;
        uint32_t m_n;

};

}
}