#pragma once

#include <vector>
#include <list>
#include <tuple>

#include "tools/Logging.h"

// Forward declarations for non member functions
namespace GF {
namespace Containers {
    template <class T>
    class SizedArray;
}
}

template <typename T>
static GF::Containers::SizedArray<T> operator+(const GF::Containers::SizedArray<T>& lhs, const GF::Containers::SizedArray<T>& rhs);

namespace GF {
namespace Containers {

    template <class T>
    class RandomAccessList {
        public:
            RandomAccessList() = default;
            RandomAccessList(RandomAccessList&&) = default;
            RandomAccessList(const RandomAccessList& other) {
                m_list = other.m_list;
                m_access_ptrs.reserve(m_list.size());
                for (auto it = m_list.begin(); it != m_list.end(); ++it) {
                    m_access_ptrs.push_back(&(*it));
                }
            }

            void operator=(const RandomAccessList& other) {
                m_list = other.m_list;
                m_access_ptrs.reserve(m_list.size());
                for (auto it = m_list.begin(); it != m_list.end(); ++it) {
                    m_access_ptrs.push_back(&(*it));
                }
            }

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

            typename std::list<T>::iterator begin() {return m_list.begin();}
            typename std::list<T>::iterator end() {return m_list.end();}
            typename std::list<T>::const_iterator begin() const {return m_list.cbegin();}
            typename std::list<T>::const_iterator end() const {return m_list.cend();}

            friend void swap(RandomAccessList& lhs, RandomAccessList& rhs) {
                std::swap(lhs.m_list, rhs.m_list);
                std::swap(lhs.m_access_ptrs, rhs.m_access_ptrs);
            }
        private:
            std::list<T> m_list;
            std::vector<T*> m_access_ptrs;
    };

    template <class T>
    class SizedArray {
        public:
            SizedArray() = delete;
            SizedArray(std::size_t size) : m_size(size) {
                m_array = (m_size) ? new T[m_size] : nullptr;
            }
            SizedArray(std::size_t size, const T& fill_val) : m_size(size) {
                m_array = (m_size) ? new T[m_size] : nullptr;
                for (std::size_t i=0; i < m_size; ++i) {
                    m_array[i] = T{};
                }
            }
            SizedArray(std::initializer_list<T> list) : m_size(list.size()) {
                m_array = (m_size) ? new T[m_size] : nullptr;
                std::size_t i = 0;
                for (auto it = list.begin(); it != list.end(); ++it) {
                    m_array[i++] = *it;
                }
            }
            SizedArray(const SizedArray& other) : m_size(other.m_size) {
                m_array = (m_size) ? new T[m_size] : nullptr;
                for (std::size_t i=0; i < m_size; ++i) {
                    m_array[i] = other.m_array[i];
                }
            }
            SizedArray(SizedArray&& other) : m_array(other.m_array), m_size(other.m_size) {
                other.m_array = nullptr;
                other.m_size = 0;
            }
            ~SizedArray() {
                safeDelete();
            }

            inline T& operator[](std::size_t i) {return m_array[i];}
            inline const T& operator[](std::size_t i) const {return m_array[i];}

            inline T& back() {return m_array[m_size - 1];}
            inline const T& back() const {return m_array[m_size - 1];}

            inline std::size_t size() const {return m_size;}

            std::vector<T> toVector() const {
                std::vector<T> v(m_size);
                for (uint32_t i=0; i<size(); ++i) v[i] = operator[](i);
                return v;
            }

            // Pseudo iterators
            T* begin() {return m_array;}
            const T* begin() const {return m_array;}
            T* end() {return m_array + m_size;}
            const T* end() const {return m_array + m_size;}

            void operator=(const SizedArray& other) {
                if (size() != other.size()) {
                    safeDelete();
                    m_size = other.m_size;

                    m_array = (m_size) ? new T[m_size] : nullptr;
                    m_array = new T[m_size];
                }
                for (std::size_t i=0; i < size(); ++i) m_array[i] = other.m_array[i];
            }
            void operator=(SizedArray&& other) {m_array = other.m_array; m_size = other.m_size;}

        private:
            inline void safeDelete() {
                if (m_array) 
                    delete[] m_array;
            }

        private:
            T* m_array;
            std::size_t m_size;
            friend SizedArray operator+<T>(const SizedArray& lhs, const SizedArray& rhs);
    };

} // namespace Containers
} // namespace GF


//TODO: move these out of ns
// Sized Array non-member operators
template <typename T>
static GF::Containers::SizedArray<T> operator+(const GF::Containers::SizedArray<T>& lhs, const GF::Containers::SizedArray<T>& rhs) {
    ASSERT(lhs.size() == rhs.size(), "Operand sizes do not match");
    GF::Containers::SizedArray<T> ret_sa(lhs.size());
    for (std::size_t i=0; i < lhs.size(); ++i) ret_sa[i] = lhs[i] + rhs[i];
    return ret_sa;
}