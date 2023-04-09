#pragma once

#include <cmath>
#include <math.h>

#include <Eigen/Core>

#define _USE_MATH_DEFINES

namespace TP {
namespace Distributions {
    
struct Gaussian {
    float mean = 0.0f;
    float variance = 0.0f;

    inline float std() const {return std::sqrt(variance);}

    float pdf(float x) const {
        float stdev = std();
        float adj = (x - mean) / stdev;
        return 1.0f / (stdev * std::sqrt(M_2_PI)) *std::exp(-0.5f * adj * adj);
    }

};

struct MultivariateGuassian {
    MultivariateGuassian(uint32_t rank) : mean(rank), covariance(rank, rank) {}

    Eigen::VectorXf mean;
    Eigen::MatrixXf covariance;
};

template <std::size_t N>
struct FixedMultivariateGuassian {
    Eigen::Matrix<float, N, 1> mean = Eigen::Matrix<float, N, 1>{};
    Eigen::Matrix<float, N, N> covariance = Eigen::Matrix<float, N, N>{};
};

}
}