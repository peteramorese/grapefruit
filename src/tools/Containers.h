#pragma once

#include <vector>
#include <list>
#include <tuple>

#include "tools/Logging.h"

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

    template <class T>
    class SizedArray {
        public:
            SizedArray() = delete;
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
            SizedArray(SizedArray&& other) : m_array(other.m_array), m_size(other.m_size) {}
            ~SizedArray() {delete[] m_array;}

            T& operator[](std::size_t i) {return m_array[i];}
            const T& operator[](std::size_t i) const {return m_array[i];}

            T& back() {return m_array[m_size - 1];}
            const T& back() const {return m_array[m_size - 1];}

            std::size_t size() const {return m_size;}
        private:
            T* m_array;
            std::size_t m_size;
    };

    // Method signature is made to match TypeGenericArray (except compare)
    template<std::size_t M, class T>
    struct FixedArray {
        public:
            FixedArray() = default;
            FixedArray(const std::array<T, M>& values_) : values(values_) {}
            FixedArray(const FixedArray& other) = default;

            T& operator[](std::size_t i) {return values[i];}

            const T& operator[](std::size_t i) const {return values[i];}
            
            template <uint32_t I>
            T& get() {return values[I];}

            template <uint32_t I>
            const T& get() const {return values[I];}

            // Elementwise access. Uses type-generic templated lambda ([]<typename T>(const T& element))
            template <typename LAMBDA_T>
            void forEach(LAMBDA_T onElement) const {
                for (std::size_t i=0; i < M; ++i) if (!onElement(values[i])) {return;};
            }

            bool operator==(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) {
                    if (values[i] != other.values[i]) return false;
                }
                return true;
            }

            bool operator<(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(values[i] < other.values[i])) {return false;};
                return true;
            }

            bool operator>(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(values[i] > other.values[i])) {return false;};
                return true;
            }

            bool operator<=(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(values[i] <= other.values[i])) {return false;};
                return true;
            }

            bool operator>=(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) if (!(values[i] >= other.values[i])) {return false;};
                return true;
            }

            // 'Dominates' operator
            bool dominates(const FixedArray& other) const {
                bool equal = true;
                for (std::size_t i=0; i < M; ++i) {
                    if (values[i] > other.values[i]) {
                        return false;
                    } else {
                        if (equal && values[i] < other.values[i]) equal = false;
                    }
                }
                return !equal;
            }

            // Lexicographic less than
            bool lexicographicLess(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) {
                    if (values[i] < other.values[i]) {
                        return true;
                    } else if (values[i] > other.values[i]) {
                        return false;
                    }
                }
                return false;
            }

            // Lexicographic less than
            bool lexicographicGreater(const FixedArray& other) const {
                for (std::size_t i=0; i < M; ++i) {
                    if (values[i] < other.values[i]) {
                        return false;
                    } else if (values[i] > other.values[i]) {
                        return true;
                    }
                }
                return false;
            }

            // Element-wise addition
            void operator+=(const FixedArray& other) {for (std::size_t i=0; i < M; ++i) values[i] += other.values[i];}

        private:
            std::array<T, M> values = std::array<T, M>();
            friend FixedArray operator+<M, T>(const FixedArray& lhs, const FixedArray& rhs);
    };

    // Cost vector non-member operators
    template<std::size_t M, class T>
    static FixedArray<M, T> operator+(const FixedArray<M, T>& lhs, const FixedArray<M, T>& rhs) {
        FixedArray<M, T> ret_cv;
        for (std::size_t i=0; i < M; ++i) ret_cv.values[i] = lhs.values[i] + rhs.values[i];
        return ret_cv;
    }


    // Method signature is made to match FixedArray (except compare)
    template <class... T_ARGS>
    class TypeGenericArray {
        protected:
            using tuple_t = std::tuple<T_ARGS...>;

        public:
            TypeGenericArray() = delete;
            TypeGenericArray(T_ARGS&&... elements_) : elements(std::forward<T_ARGS>(elements_)...) {}
            TypeGenericArray(const TypeGenericArray&) = default;
            TypeGenericArray(TypeGenericArray&&) = default;

            constexpr uint32_t size() const {return sizeof...(T_ARGS);}

            template <uint32_t I>
            inline std::tuple_element<I, tuple_t>::type& get() {return std::get<I>(elements);}

            template <uint32_t I>
            inline const std::tuple_element<I, tuple_t>::type& get() const {return std::get<I>(elements);}

            // Elementwise access. Uses type-generic templated lambda ([]<typename T>(const T& element))
            template <typename LAMBDA_T>
            void forEach(LAMBDA_T onElement) const {
                _forEach<0, LAMBDA_T>(onElement);
            }

            // Element-wise conjunctive comparison. Uses type-generic templated lambda ([]<typename T, uint32_t I>(const T& element))
            template <typename LAMBDA_T>
            bool compare(const TypeGenericArray& other, LAMBDA_T compareElement) const {
                return _compare<0>(compareElement);
            }

            // Implemented comparison operators
            bool operator==(const TypeGenericArray& other) const {
                auto equals = [&other]<typename T, uint32_t I>(const T& element) {
                    return other.template get<I>() == element;
                };
                return _compare<0>(equals);
            }

            bool operator<(const TypeGenericArray& other) const {
                auto less_than = [&other]<typename T, uint32_t I>(const T& element) {
                    return other.template get<I>() < element;
                };
                return _compare<0>(less_than);
            }

            bool operator>(const TypeGenericArray& other) const {
                auto greater_than = [&other]<typename T, uint32_t I>(const T& element) {
                    return other.template get<I>() > element;
                };
                return _compare<0>(greater_than);
            }

            bool operator<=(const TypeGenericArray& other) const {
                auto leq = [&other]<typename T, uint32_t I>(const T& element) {
                    return other.template get<I>() <= element;
                };
                return _compare<0>(leq);
            }

            bool operator>=(const TypeGenericArray& other) const {
                auto geq = [&other]<typename T, uint32_t I>(const T& element) {
                    return other.template get<I>() >= element;
                };
                return _compare<0>(geq);
            }

            // 'Dominates' operator
            bool dominates(const TypeGenericArray& other) const {
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
                return (_compare<0>(dominates_)) ? !equal : false;
            }

            // Lexicographic less than
            bool lexicographicLess(const TypeGenericArray& other) {
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
                _compare<0>(lexicographicLess_);
                return result;
            }

            // Lexicographic greater than
            bool lexicographicGreater(const TypeGenericArray& other) {
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
                _compare<0>(lexicographicGreater_);
                return result;
            }

        protected:

            template<uint32_t I, typename LAMBDA_T>
            bool _compare(LAMBDA_T compareElement) const {
                if (compareElement.template operator()<typename std::tuple_element<I, tuple_t>::type, I>(std::get<I>(elements))) {
                    if constexpr (I != (sizeof...(T_ARGS) - 1)) {
                        return _compare<I + 1, LAMBDA_T>(compareElement);
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

            template<uint32_t I, typename LAMBDA_T>
            bool _forEach(LAMBDA_T onElement) const {
                if (onElement.template operator()<typename std::tuple_element<I, tuple_t>::type>(std::get<I>(elements))) {
                    if constexpr (I != (sizeof...(T_ARGS) - 1)) {
                        return _forEach<I + 1, LAMBDA_T>(onElement);
                    }  else {
                        return true;
                    }
                } else {
                    return false;
                }
            }

            std::tuple<T_ARGS...> elements;
    };

} // namespace Containers
} // namespace TP