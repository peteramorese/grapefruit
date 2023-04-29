#pragma once

#include <vector>

namespace TP {
namespace Stats {

class SampleSet {
    public:
        SampleSet() = default;
        
        inline float avg() const {return m_avg;}
        inline uint32_t size() const {return m_sample_set.size();}
        inline const std::vector<float>& getSamples() const {return m_sample_set;}
        inline void clear() {m_sample_set.clear(); m_avg = 0.0f;}

        inline void add(float sample) {
            float n = static_cast<float>(m_sample_set.size());
            m_avg = (n * m_avg + sample) / (n + 1.0f);
            m_sample_set.push_back(sample);
        }
    private:
        std::vector<float> m_sample_set;
        float m_avg = 0.0f;
};

}
}
