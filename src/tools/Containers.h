#pragma once

#include <list>

namespace TP {
namespace Containers {

    template <class T>
    class RandomAccessList {
        public:
            void push_back(const T& value) {
                m_list.push_back(value);
                m_access_ptrs.push_back(&m_list.back());
            }

            T& operator[](uint32_t i) {return *m_access_ptrs[i];}
            const T& operator[](uint32_t i) const {return *m_access_ptrs[i];}

            void clear() {m_list.clear(); m_access_ptrs.clear();}
            void resize(std::size_t count) {
                auto list_it = std::prev(m_list.end());
                m_list.resize(count);
                m_access_ptrs.reserve(count);
                while (m_access_ptrs.size() < m_access_ptrs.capacity()) {
                    m_access_ptrs.insert(m_access_ptrs.end(), &(*(++list_it)));
                }
            }
            std::size_t size() const {return m_list.size();}

            std::list<T>::iterator begin() {return m_list.begin();}
            std::list<T>::iterator end() {return m_list.end();}
            std::list<T>::const_iterator begin() const {return m_list.cbegin();}
            std::list<T>::const_iterator end() const {return m_list.cend();}
        private:
            std::list<T> m_list;
            std::vector<T*> m_access_ptrs;
    };

    template<class T>
    class SizedArray {
        public:
            SizedArray(std::size_t size) : m_array(new T[size]), m_size(size) {}
            SizedArray(std::size_t size, const T& fill_val) : m_array(new T[size]), m_size(size) {
                for (std::size_t i=0; i < m_size; ++i) {
                    m_array[i] = T{};
                }
            }
            SizedArray(const SizedArray& other) : m_array(new T[other.m_size]), m_size(other.m_size) {
                for (std::size_t i=0; i < m_size; ++i) {
                    m_array[i] = other.m_array[i];
                }
            }
            ~SizedArray() {delete[] m_array;}

            T& operator[](std::size_t i) {return m_array[i];}
            const T& operator[](std::size_t i) const {return m_array[i];}

            T& back() {return m_array[m_size - 1];}
            const T& back() const {return m_array[m_size - 1];}

            std::size_t size() const {return m_size;}
        private:
            T* m_array;
            const std::size_t m_size;
    };

} // namespace Containers
} // namespace TP