#pragma once

#include <Eigen/Dense>

#include "tools/Containers.h"

namespace GF {

template <typename T, std::size_t N>
static void toColMatrix(const Containers::FixedArray<N, T>& in, Eigen::Matrix<T, N, 1>& out) {
    for (uint32_t i = 0; i < N; ++i) {
        out(i) = in[i];
    }
}

template <typename T, std::size_t N>
static void toRowMatrix(const Containers::FixedArray<N, T>& in, Eigen::Matrix<T, 1, N>& out) {
    for (uint32_t i = 0; i < N; ++i) {
        out(i) = in[i];
    }
}

template <typename T, std::size_t N>
static void fromColMatrix(const Eigen::Matrix<T, N, 1>& in, Containers::FixedArray<N, T>& out) {
    for (uint32_t i = 0; i < N; ++i) {
        out[i] = in(i);
    }
}

template <typename T, std::size_t N>
static void fromRowMatrix(const Eigen::Matrix<T, 1, N>& in, Containers::FixedArray<N, T>& out) {
    for (uint32_t i = 0; i < N; ++i) {
        out[i] = in(i);
    }
}

}