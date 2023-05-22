#pragma once

#include <vector>

namespace TP {
namespace Stats {

template <typename T>
class SampleSet {
    public:
        typedef T type;
    public:
        SampleSet() = default;
        
        inline const T& avg() const {return m_avg;}
        inline uint32_t size() const {return m_sample_set.size();}
        inline const std::vector<T>& getSamples() const {return m_sample_set;}
        inline void clear() {m_sample_set.clear(); m_avg = 0.0f;}

        inline void add(const T& sample) {
            float n = static_cast<float>(m_sample_set.size());
            m_avg = (n * m_avg + sample) / (n + 1.0f);
            m_sample_set.push_back(sample);
        }
    private:
        std::vector<T> m_sample_set;
        T m_avg = 0.0f;
};

}
}
