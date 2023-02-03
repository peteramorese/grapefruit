#pragma once

namespace Containers {

    template<class T>
    class SizedArray {
        public:
            SizedArray(std::size_t size) : m_array(new T[size]), m_size(size) {}
            SizedArray(const SizedArray& other) : m_array(new T[other.m_size]), m_size(other.m_size) {
                for (std::size_t i=0; i < m_size; ++i) {
                    m_array[i] = other.m_array[i];
                }
            }
            ~SizedArray() {delete[] m_array;}
            T& operator[](std::size_t i) {return m_array[i];}
            const T& operator[](std::size_t i) const {return m_array[i];}
            std::size_t size() const {return m_size;}
        private:
            T* m_array;
            const std::size_t m_size;
    };

}