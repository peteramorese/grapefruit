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
        Normal() = default;
        Normal(float mu_, float sigma_2_) 
            : mu(mu_)
            , sigma_2(sigma_2_)
        {}

        inline float std() const {return std::sqrt(sigma_2);}

        float pdf(const float& x) const {
            float stdev = std();
            float adj = (x - mu) / stdev;
            return 1.0f / (stdev * std::sqrt(M_2_PI)) *std::exp(-0.5f * adj * adj);
        }

        void convolveWith(const Normal& other) {
            mu += other.mu;
            sigma_2 += other.sigma_2;
        }

};

struct MultivariateNormal {
    public:
        Eigen::VectorXf mu;
        Eigen::MatrixXf Sigma;

    public:
        MultivariateNormal(uint32_t N) : mu(N), Sigma(N, N) {}

        float pdf(const Eigen::VectorXf& x) const {
            // TODO
            return 0.0f;
        }

};

template <std::size_t N>
struct FixedMultivariateNormal {
    public:
        Eigen::Matrix<float, N, 1> mu = Eigen::Matrix<float, N, 1>::Zero();
        Eigen::Matrix<float, N, N> Sigma = Eigen::Matrix<float, N, N>::Zero();

    public:
        FixedMultivariateNormal() = default;
        FixedMultivariateNormal(const Eigen::Matrix<float, N, 1>& mu_, const Eigen::Matrix<float, N, N>& Sigma_)
            : mu(mu_)
            , Sigma(Sigma_)
        {}

        float pdf(const Eigen::Matrix<float, N, 1>& x) const {
            // TODO
            return 0.0f;
        }

        void convolveWith(const FixedMultivariateNormal<N>& other) {
            mu += other.mu;
            Sigma += other.Sigma;
        }

        float entropy() const {
            constexpr float d = static_cast<float>(N);
            return 0.5f * (std::log(Sigma.determinant()) + d * (1.0f + std::log(M_2_PI)));
        }

        static constexpr std::size_t uniqueCovarianceElements() {return N * (N + 1u) / 2;}

        void setSigmaFromUniqueElementVector(const Eigen::Matrix<float, uniqueCovarianceElements(), 1>& Sigma_) {
            std::size_t row = 0;
            std::size_t col = 0;
            for (std::size_t i = 0; i < uniqueCovarianceElements(); ++i) {
                Sigma(row, col) = Sigma_(i);
                if (col > row) 
                    Sigma(col, row) = Sigma_(i);
                if (col == N - 1) {
                    ++row;
                    col = row;
                } else {
                    ++col;
                }
            }
        }
};

template <std::size_t N>
class FixedMultivariateNormalSampler {
    public:
        FixedMultivariateNormalSampler(const FixedMultivariateNormal<N>& dist) 
        {
            resetDist(dist);
        }

        const FixedMultivariateNormal<N>& dist() const {return m_dist;}
        const Eigen::Matrix<float, N, 1>& mean() const {return m_dist.mu;}
        const Eigen::Matrix<float, N, N>& transform() const {return m_transform;}

        void resetDist(const FixedMultivariateNormal<N>& dist) {
            m_dist = dist;
            Eigen::SelfAdjointEigenSolver<Eigen::Matrix<float, N, N>> solver(dist.Sigma);
            m_transform = solver.eigenvectors() * solver.eigenvalues().cwiseSqrt().asDiagonal();
        }

    private:
        FixedMultivariateNormal<N> m_dist;
        Eigen::Matrix<float, N, N> m_transform;
};

} // namespace Distributions

// Expectation and variance overloads
inline static float E(const Distributions::Normal& p) {return p.mu;}
inline static float var(const Distributions::Normal& p) {return p.sigma_2;}
inline static Distributions::Normal convolve(const Distributions::Normal& lhs, const Distributions::Normal& rhs) {return Distributions::Normal(lhs.mu + rhs.mu, lhs.sigma_2 + rhs.sigma_2);}

inline static const Eigen::VectorXf& E(const Distributions::MultivariateNormal& p) {return p.mu;}
inline static const Eigen::MatrixXf& cov(const Distributions::MultivariateNormal& p) {return p.Sigma;}

template <std::size_t N>
inline static const Eigen::Matrix<float, N, 1>& E(const Distributions::FixedMultivariateNormal<N>& p) {return p.mu;}
template <std::size_t N>
inline static const Eigen::Matrix<float, N, N>& var(const Distributions::FixedMultivariateNormal<N>& p) {return p.Sigma;}
//inline static Distributions::MultivariateNormal convolve(const Distributions::MultivariateNormal& lhs, const Distributions::MultivariateNormal& rhs) {
//    return Distributions::Normal(lhs.mu + rhs.mu, lhs.covariance + rhs.covariance);
//}

} // namespace Stats
} // namespace TP
