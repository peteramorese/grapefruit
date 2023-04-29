#pragma once

#include <cmath>
#include <math.h>

#include <Eigen/Dense>

#include "statistics/StatTools.h"
#include "tools/Logging.h"

#define _USE_MATH_DEFINES

namespace TP {
namespace Stats {
namespace Distributions {
    
struct Normal {
    public:
        float mu = 0.0f;
        float sigma_2 = 0.0f;

    public:
        inline float std() const {return std::sqrt(sigma_2);}

        float pdf(const float& x) const {
            float stdev = std();
            float adj = (x - mu) / stdev;
            return 1.0f / (stdev * std::sqrt(M_2_PI)) *std::exp(-0.5f * adj * adj);
        }

};


struct MultivariateNormal {
    public:
        Eigen::VectorXf mu;
        Eigen::MatrixXf covariance;

    public:
        MultivariateNormal(uint32_t N) : mu(N), covariance(N, N) {}

        float pdf(const Eigen::VectorXf& x) const {
            // TODO
        }

};

template <std::size_t N>
struct FixedMultivariateNormal {
    public:
        Eigen::Matrix<float, N, 1> mu = Eigen::Matrix<float, N, 1>{};
        Eigen::Matrix<float, N, N> covariance = Eigen::Matrix<float, N, N>{};

    public:
        float pdf(const Eigen::Matrix<float, N, 1>& x) const {
            // TODO
        }
};


} // namespace Distributions

// Expectation and variance overloads
inline static float E(const Distributions::Normal& p) {return p.mu;}
inline static float var(const Distributions::Normal& p) {return p.mu;}
inline static const Eigen::VectorXf& E(const Distributions::MultivariateNormal& p) {return p.mu;}
inline static const Eigen::MatrixXf& cov(const Distributions::MultivariateNormal& p) {return p.covariance;}

} // namespace Stats
} // namespace TP
