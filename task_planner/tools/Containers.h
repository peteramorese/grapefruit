#pragma once

#include <vector>
#include <list>
#include <tuple>

#include "tools/Logging.h"

// Forward declarations for non member functions
namespace TP {
namespace Containers {
    template <class T>
    class SizedArray;

    template <class... T_ARGS>
    class TypeGenericArray;

    template <std::size_t M, class T>
    class FixedArray;
}
}

template <typename T>
static TP::Containers::SizedArray<T> operator+(const TP::Containers::SizedArray<T>& lhs, const TP::Containers::SizedArray<T>& rhs);

template <class... T_ARGS>
static TP::Containers::TypeGenericArray<T_ARGS...> operator+(const TP::Containers::TypeGenericArray<T_ARGS...>& lhs, const TP::Containers::TypeGenericArray<T_ARGS...>& rhs);

template <std::size_t M, class T>
static TP::Containers::FixedArray<M, T> operator+(const TP::Containers::FixedArray<M, T>& lhs, const TP::Containers::FixedArray<M, T>& rhs);


namespace TP {
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

            std::list<T>::iterator begin() {return m_list.begin();}
            std::list<T>::iterator end() {return m_list.end();}
            std::list<T>::const_iterator begin() const {return m_list.cbegin();}
            std::list<T>::const_iterator end() const {return m_list.cend();}

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

    enum class ArrayComparison {
        Dominates,
        DoesNotDominate,
        Equal
    };

    /* 
    Compile-time sized array wrapper around std::array.

    Method signature is made to match TypeGenericArray (except compare)
    */
    template<std::size_t M, class T>
    struct FixedArray {
        public:
            FixedArray() {
                for (T& v : m_values) 
                    v = T{};
            }
            FixedArray(const std::array<T, M>& values_) : m_values(values_) {}
            FixedArray(const FixedArray& other) = default;

            static constexpr std::size_t size() {return M;}

            T& operator[](std::size_t i) {return m_values[i];}

            const T& operator[](std::size_t i) const {return m_values[i];}
            
            template <uint32_t I>
            T& get() {return m_values[I];}

            template <uint32_t I>
            const T& get() const {return m_values[I];}

            // Elementwise access. Uses type-generic templated lambda ([]<typename T>(T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEach(LAMBDA_T onElement) {
                for (std::size_t i=0; i < M; ++i) if (!onElement(m_values[i])) {return;};
            }

            // Elementwise access. Uses type-generic templated lambda ([]<typename T>(const T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEach(LAMBDA_T onElement) const {
                for (std::size_t i=0; i < M; ++i) if (!onElement(m_values[i])) {return;};
            }

            // Elementwise access. Uses type-generic templated lambda ([]<typename T, uint32_t I>(T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEachWithI(LAMBDA_T onElement) {
                _forEachWithI<0, LAMBDA_T>(onElement);
            }

            // Elementwise access. Uses type-generic templated lambda ([]<typename T, uint32_t I>(const T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEachWithI(LAMBDA_T onElement) const {
                _forEachWithI_c<0, LAMBDA_T>(onElement);
            }

            bool operator==(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) {
                    if (m_values[i] != other.m_values[i]) return false;
                }
                return true;
            }

            bool operator<(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(m_values[i] < other.m_values[i])) {return false;};
                return true;
            }

            bool operator>(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(m_values[i] > other.m_values[i])) {return false;};
                return true;
            }

            bool operator<=(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(m_values[i] <= other.m_values[i])) {return false;};
                return true;
            }

            bool operator>=(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(m_values[i] >= other.m_values[i])) {return false;};
                return true;
            }

            // 'Dominates' operator
            ArrayComparison dominates(const FixedArray& other) const {
                bool equal = true;
                for (std::size_t i=0; i < M; ++i) {
                    if (m_values[i] > other.m_values[i]) {
                        return ArrayComparison::DoesNotDominate;
                    } else {
                        if (equal && m_values[i] < other.m_values[i]) equal = false;
                    }
                }
                if (equal) return ArrayComparison::Equal;
                return ArrayComparison::Dominates;
            }

            // Lexicographic less than
            bool lexicographicLess(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) {
                    if (m_values[i] < other.m_values[i]) {
                        return true;
                    } else if (m_values[i] > other.m_values[i]) {
                        return false;
                    }
                }
                return false;
            }

            // Lexicographic less than
            bool lexicographicGreater(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) {
                    if (m_values[i] < other.m_values[i]) {
                        return false;
                    } else if (m_values[i] > other.m_values[i]) {
                        return true;
                    }
                }
                return false;
            }

            // Element-wise addition
            void operator+=(const FixedArray& other) {for (std::size_t i=0; i < M; ++i) m_values[i] += other.m_values[i];}

            void operator=(const FixedArray& other) {for (std::size_t i=0; i < M; ++i) m_values[i] = other.m_values[i];}

            std::array<T, M>::iterator begin() {return m_values.begin();}
            std::array<T, M>::iterator end() {return m_values.end();}
            std::array<T, M>::const_iterator begin() const {return m_values.begin();}
            std::array<T, M>::const_iterator end() const {return m_values.end();}
        private:

            template<uint32_t I, typename LAMBDA_T>
            bool _forEachWithI(LAMBDA_T compareElement) {
                if (compareElement.template operator()<T, I>(std::get<I>(m_values))) {
                    if constexpr (I != (M - 1)) {
                        return _forEachWithI<I + 1, LAMBDA_T>(compareElement);
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

            template<uint32_t I, typename LAMBDA_T>
            bool _forEachWithI_c(LAMBDA_T compareElement) const {
                if (compareElement.template operator()<T, I>(std::get<I>(m_values))) {
                    if constexpr (I != (M - 1)) {
                        return _forEachWithI_c<I + 1, LAMBDA_T>(compareElement);
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

        private:
            std::array<T, M> m_values = std::array<T, M>();
            friend FixedArray operator+<M, T>(const FixedArray& lhs, const FixedArray& rhs);
    };


    /* 
    Complile-time sized type-generic array wrapper around std::tuple.

    Method signature is made to match FixedArray (except compare)
    */
    template <class... T_ARGS>
    class TypeGenericArray {
        protected:
            using tuple_t = std::tuple<T_ARGS...>;

        public:
            TypeGenericArray() = default; // value initializes each element
            TypeGenericArray(T_ARGS&&... elements) : m_elements(std::forward<T_ARGS>(elements)...) {}
            TypeGenericArray(const T_ARGS&... elements) : m_elements(elements...) {}
            TypeGenericArray(const TypeGenericArray&) = default;
            TypeGenericArray(TypeGenericArray&&) = default;

            static constexpr uint32_t size() {return sizeof...(T_ARGS);}

            template <uint32_t I>
            inline std::tuple_element<I, tuple_t>::type& get() {return std::get<I>(m_elements);}

            template <uint32_t I>
            inline const std::tuple_element<I, tuple_t>::type& get() const {return std::get<I>(m_elements);}

            // Elementwise access. Uses type-generic templated lambda ([]<typename T>(T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEach(LAMBDA_T onElement) {
                _forEach<0, LAMBDA_T>(onElement);
            }

            // Elementwise access. Uses type-generic templated lambda ([]<typename T>(const T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEach(LAMBDA_T onElement) const {
                _forEach_c<0, LAMBDA_T>(onElement);
            }

            // Elementwise access. Uses type-generic templated lambda ([]<typename T, uint32_t I>(T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEachWithI(LAMBDA_T onElement) {
                _forEachWithI<0, LAMBDA_T>(onElement);
            }

            // Elementwise access. Uses type-generic templated lambda ([]<typename T, uint32_t I>(const T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            void forEachWithI(LAMBDA_T onElement) const {
                _forEachWithI_c<0, LAMBDA_T>(onElement);
            }

            // Element-wise conjunctive comparison. Uses type-generic templated lambda ([]<typename T, uint32_t I>(const T& element))
            // Return true to continue, false to exit early
            template <typename LAMBDA_T>
            bool compare(const TypeGenericArray& other, LAMBDA_T compareElement) const {
                return _forEachWithI<0>(compareElement);
            }

            // Implemented comparison operators
            bool operator==(const TypeGenericArray& other) const {
                auto equals = [&other]<typename T, uint32_t I>(const T& element) {
                    return other.template get<I>() == element;
                };
                return _forEachWithI_c<0>(equals);
            }

            bool operator<(const TypeGenericArray& other) const {
                auto less_than = [&other]<typename T, uint32_t I>(const T& element) {
                    return element < other.template get<I>();
                };
                return _forEachWithI_c<0>(less_than);
            }

            bool operator>(const TypeGenericArray& other) const {
                auto greater_than = [&other]<typename T, uint32_t I>(const T& element) {
                    return element > other.template get<I>();
                };
                return _forEachWithI_c<0>(greater_than);
            }

            bool operator<=(const TypeGenericArray& other) const {
                auto leq = [&other]<typename T, uint32_t I>(const T& element) {
                    return element <= other.template get<I>();
                };
                return _forEachWithI_c<0>(leq);
            }

            bool operator>=(const TypeGenericArray& other) const {
                auto geq = [&other]<typename T, uint32_t I>(const T& element) {
                    return element >= other.template get<I>();
                };
                return _forEachWithI_c<0>(geq);
            }

            // 'Dominates' operator
            ArrayComparison dominates(const TypeGenericArray& other) const {
                bool equal = true;
                auto dominates_ = [&other, &equal]<typename T, uint32_t I>(const T& element) {
                    if (other.template get<I>() < element) {
                        // return false and stop iterating
                        return false;
                    } else {
                        if (equal && element < other.template get<I>()) equal = false;
                    }
                    // Continue
                    return true;
                };
                if (_forEachWithI_c<0>(dominates_)) {
                    if (equal) return ArrayComparison::Equal;
                    return ArrayComparison::Dominates;
                } 
                return ArrayComparison::DoesNotDominate;
            }

            // Lexicographic less than
            bool lexicographicLess(const TypeGenericArray& other) const {
                bool result = true;
                auto lexicographicLess_ = [&other, &result]<typename T, uint32_t I>(const T& element) {
                    if (element < other.template get<I>()) {
                        return false; // return true
                    } else if (other.template get<I>() < element) {
                        result = false; // return false;
                        return false;
                    }
                    // Continue
                    return true;
                };
                _forEachWithI_c<0>(lexicographicLess_);
                return result;
            }

            // Lexicographic greater than
            bool lexicographicGreater(const TypeGenericArray& other) const {
                bool result = true;
                auto lexicographicGreater_ = [&other, &result]<typename T, uint32_t I>(const T& element) {
                    if (element < other.template get<I>()) {
                        return false; // return true
                    } else if (other.template get<I>() < element) {
                        result = false; // return false;
                        return false;
                    }
                    // Continue
                    return true;
                };
                _forEachWithI_c<0>(lexicographicGreater_);
                return result;
            }

            void operator+=(const TypeGenericArray& other) {
                auto peq = [&other]<typename T, uint32_t I>(T& element) {
                    element += other.template get<I>();
                    return true;
                };
                _forEachWithI<0>(peq);
            }

            void operator=(const TypeGenericArray& other) {
                m_elements = other.m_elements;
            }

            void operator=(TypeGenericArray&& other) {
                m_elements = other.m_elements;
            }
        protected:

            template<uint32_t I, typename LAMBDA_T>
            bool _forEach(LAMBDA_T onElement) {
                if (onElement.template operator()<typename std::tuple_element<I, tuple_t>::type>(std::get<I>(m_elements))) {
                    if constexpr (I != (sizeof...(T_ARGS) - 1)) {
                        return _forEach<I + 1, LAMBDA_T>(onElement);
                    }  else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

            template<uint32_t I, typename LAMBDA_T>
            bool _forEach_c(LAMBDA_T onElement) const {
                if (onElement.template operator()<typename std::tuple_element<I, tuple_t>::type>(std::get<I>(m_elements))) {
                    if constexpr (I != (sizeof...(T_ARGS) - 1)) {
                        return _forEach_c<I + 1, LAMBDA_T>(onElement);
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

            template<uint32_t I, typename LAMBDA_T>
            bool _forEachWithI(LAMBDA_T compareElement) {
                if (compareElement.template operator()<typename std::tuple_element<I, tuple_t>::type, I>(std::get<I>(m_elements))) {
                    if constexpr (I != (sizeof...(T_ARGS) - 1)) {
                        return _forEachWithI<I + 1, LAMBDA_T>(compareElement);
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

            template<uint32_t I, typename LAMBDA_T>
            bool _forEachWithI_c(LAMBDA_T compareElement) const {
                if (compareElement.template operator()<typename std::tuple_element<I, tuple_t>::type, I>(std::get<I>(m_elements))) {
                    if constexpr (I != (sizeof...(T_ARGS) - 1)) {
                        return _forEachWithI_c<I + 1, LAMBDA_T>(compareElement);
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

        protected:
            std::tuple<T_ARGS...> m_elements;
            friend TypeGenericArray operator+<T_ARGS...>(const TypeGenericArray& lhs, const TypeGenericArray& rhs);
            
    };


} // namespace Containers
} // namespace TP


//TODO: move these out of ns
// Sized Array non-member operators
template <typename T>
static TP::Containers::SizedArray<T> operator+(const TP::Containers::SizedArray<T>& lhs, const TP::Containers::SizedArray<T>& rhs) {
    ASSERT(lhs.size() == rhs.size(), "Operand sizes do not match");
    TP::Containers::SizedArray<T> ret_sa(lhs.size());
    for (std::size_t i=0; i < lhs.size(); ++i) ret_sa[i] = lhs[i] + rhs[i];
    return ret_sa;
}

// Fixed Array non-member operators
template<std::size_t M, class T>
static TP::Containers::FixedArray<M, T> operator+(const TP::Containers::FixedArray<M, T>& lhs, const TP::Containers::FixedArray<M, T>& rhs) {
    TP::Containers::FixedArray<M, T> ret_fa;
    for (std::size_t i=0; i < M; ++i) ret_fa.m_values[i] = lhs.m_values[i] + rhs.m_values[i];
    return ret_fa;
}

// Type Generic Array non-member operators
template <class... T_ARGS>
static TP::Containers::TypeGenericArray<T_ARGS...> operator+(const TP::Containers::TypeGenericArray<T_ARGS...>& lhs, const TP::Containers::TypeGenericArray<T_ARGS...>& rhs) {
    TP::Containers::TypeGenericArray<T_ARGS...> ret_tga;
    auto add = [&ret_tga, &rhs]<typename T, uint32_t I>(const T& element) {
        return ret_tga.template get<I>() = element + rhs.template get<I>() ;
    };
    lhs.forEachWithI(add);
    return ret_tga;
}
