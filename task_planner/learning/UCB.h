#pragma once

#include <cmath>

#include "tools/Misc.h"

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
        inline float getReward(float exploitation_estimate, uint32_t k) const { return exploitation_estimate + explorationBonus(k); }
        inline float getCost(float exploitation_estimate, uint32_t k) const { return exploitation_estimate - explorationBonus(k); }
        inline float getRectifiedCost(float exploitation_estimate, uint32_t k) const { return max(exploitation_estimate - explorationBonus(k), 0.0f); }
        inline float explorationBonus(uint32_t k) const {
            return m_confidence * std::sqrt(std::log(k + 1) / static_cast<float>(m_n + 1));
        }
    protected:
        float m_confidence;
        uint32_t m_n;

};

}
}