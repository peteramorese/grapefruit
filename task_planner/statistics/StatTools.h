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
        SampleSet(const T& preset_avg) 
            : m_preset_avg(preset_avg)
            , m_avg(preset_avg) 
        {}
        
        inline const T& avg() const {return m_avg;}
        inline std::size_t size() const {return m_sample_set.size();}
        inline const std::vector<T>& getSamples() const {return m_sample_set;}
        inline void clear() {m_sample_set.clear(); m_avg = 0.0f;}

        inline void add(const T& sample) {
            float n = static_cast<float>(m_sample_set.size());
            m_avg = (n * m_avg + sample) / (n + 1.0f);
            m_sample_set.push_back(sample);
        }

        inline void add(T&& sample) {
            float n = static_cast<float>(m_sample_set.size());
            m_avg = (n * m_avg + sample) / (n + 1.0f);
            m_sample_set.push_back(std::move(sample));
        }

        inline void pop_back() {
            float n = static_cast<float>(m_sample_set.size());
            m_avg = (m_sample_set.size() > 1) ? (n * m_avg - m_sample_set.back()) / (n - 1.0f) : m_preset_avg;
            m_sample_set.pop_back();
        }
    private:
        T m_preset_avg = T{};
        std::vector<T> m_sample_set;
        T m_avg = T{};
};

// Immutable wrapper for including a posterior sample
template <typename T>
class PosteriorSampleSet {
    public:
        PosteriorSampleSet(const SampleSet<T>* sample_set, const T& posterior_sample) 
            : m_sample_set(sample_set)
            , m_posterior_sample(posterior_sample)
        {}

        inline std::size_t size() const {return m_sample_set->size() + 1;}
        const std::vector<T>& getPriorSamples() const {return m_sample_set->getSamples();}
        const T& getPosteriorSample() const {return m_posterior_sample;}
        T avg() const {
            float n = static_cast<float>(m_sample_set->size());
            return (n * m_sample_set->avg() + m_posterior_sample) / (n + 1.0f);
        }
        
    private:
        const SampleSet<T>* const m_sample_set;
        const T m_posterior_sample;
};


}
}
