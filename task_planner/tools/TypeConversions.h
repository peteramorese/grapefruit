#pragma once

#include <Eigen/Dense>

#include "tools/Containers.h"

namespace TP {

template <typename T, uint32_t N>
static void convert(const Containers::FixedArray<N, T>& in, Eigen::Matrix<T, N, 1>& out) {
    for (uint32_t i = 0; i < N; ++i) {
        out(i, 1) = in[i];
    }
}

template <typename T, uint32_t N>
static void convert(const Containers::FixedArray<N, T>& in, Eigen::Matrix<T, 1, N>& out) {
    for (uint32_t i = 0; i < N; ++i) {
        out(1, i) = in[i];
    }
}


}