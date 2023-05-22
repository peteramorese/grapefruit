#pragma once

#include <cmath>
#include <math.h>

#include "statistics/StatTools.h"
#include "tools/Logging.h"

#define _USE_MATH_DEFINES

namespace TP {
namespace Stats {
namespace Distributions {
    
struct Gamma {
    public:
        uint32_t alpha = 1;
        float beta = 1.0f;

    public:
        Gamma() = default;
        Gamma(uint32_t alpha_, float beta_)
            : alpha(alpha_)
            , beta(beta_)
        {}

        float pdf(float x) const {
            ASSERT(x >= 0.0f, "Input 'x' must be geq 0");
            ASSERT(alpha >= 1, "'alpha' cannot be zero");
            return std::pow(x, alpha - 1) * std::exp(-beta * x) * std::pow(beta, alpha) / tgamma(alpha);
        }
};

struct LocationScaleT {
    public:
        float nu = 0;
        float mu = 0.0f;
        float sigma_2 = 0.0f;

    public:
        LocationScaleT() = default;
        LocationScaleT(float nu_, float mu_, float sigma_2_) 
            : nu(nu_)
            , mu(mu_)
            , sigma_2(sigma_2_)
        {}

        inline float normalizationConstant() const {
            return tgamma(0.5f * (nu + 1.0f)) / (tgamma(0.5f * nu) * std::sqrt(nu * M_PI * sigma_2));
        }

        float pdf(float x) const {
            return normalizationConstant() * std::pow(1.0f + std::pow(x - mu, 2) / (nu * sigma_2), -(0.5f * (nu + 1.0f)));
        }
};

template <std::size_t N>
struct FixedMultivariateT {
    public:
        float nu = 0;
        Eigen::Matrix<float, N, 1> mu = Eigen::Matrix<float, N, 1>::Zero();
        Eigen::Matrix<float, N, N> Sigma = Eigen::Matrix<float, N, N>::Zero();
    public:
        FixedMultivariateT(float nu_, const Eigen::Matrix<float, N, 1>& mu_, const Eigen::Matrix<float, N, N>& sigma_)
            : nu(nu_)
            , mu(mu_)
            , Sigma(sigma_)
        {}

        // TODO normalizationConstant()
        // TODO pdf()

};

struct FixedWishart {
    // TODO
};

template <std::size_t N>
struct FixedInverseWishart {
    public:
        float nu = static_cast<float>(N);
        Eigen::Matrix<float, N, N> Psi = Eigen::Matrix<float, N, N>::Identity();
    public:
        FixedInverseWishart(float nu_, const Eigen::Matrix<float, N, N>& Psi_)
            : nu(nu_)
            , Psi(Psi_)
        {}

        static constexpr std::size_t uniqueElements() {return N * (N - 1u) / 2;}

        // TODO normalizationConstant()
        // TODO pdf()
};

} // namespace Distributions

// Expectation and variance overloads
// Gamma
inline static float E(const Distributions::Gamma& p) {ASSERT(p.beta > 0.0f, "Singular value"); return static_cast<float>(p.alpha) / p.beta;}
inline static float var(const Distributions::Gamma& p) {ASSERT(p.beta > 0.0f, "Singular value"); return static_cast<float>(p.alpha) / (p.beta * p.beta);}

// LocationScaleT
inline static float E(const Distributions::LocationScaleT& p) {ASSERT(p.nu > 1.0f, "DOF must be greater than 1.0"); return p.mu;}
inline static float var(const Distributions::LocationScaleT& p) {ASSERT(p.nu > 2.0f, "DOF must be greater than 2.0"); return p.nu * p.sigma_2 / (p.nu - 2.0f);} 

// FixedMultivariateT
template <std::size_t N>
inline static const Eigen::Matrix<float, N, 1>& E(const Distributions::FixedMultivariateT<N>& p) {ASSERT(p.nu > 1.0f, "DOF must be greater than 1.0"); return p.mu;}
template <std::size_t N>
inline static Eigen::Matrix<float, N, N> var(const Distributions::FixedMultivariateT<N>& p) {ASSERT(p.nu > 2.0f, "DOF must be greater than 2.0"); return p.Sigma * (p.nu / (p.nu - 2.0f));}

// FixedInverseWishart
template <std::size_t N>
inline static Eigen::Matrix<float, N, N> E(const Distributions::FixedInverseWishart<N>& p) {
    ASSERT(p.nu > static_cast<float>(N + 1u), "DOF must be greater than N + 1"); 
    return p.Psi * (1.0f / (p.nu - static_cast<float>(N) - 1.0f));
}
template <std::size_t N>
static Eigen::Matrix<float, Distributions::FixedInverseWishart<N>::uniqueElements(), Distributions::FixedInverseWishart<N>::uniqueElements()> var(const Distributions::FixedInverseWishart<N>& p) {
    constexpr uint32_t uniqueN = Distributions::FixedInverseWishart<N>::uniqueElements();
    float N_f = static_cast<float>(N);
    ASSERT(p.nu > static_cast<float>(N + 1u), "DOF must be creater than N (+ 1) <- check wikipedia");
    
    /*
    Unique elements correspond to the upper triangular elements iterating over columns then rows, i.e.

    [sig11 * sig11, sig11 * sig12, sig11 * sig22]
    [sig12 * sig11, sig12 * sig12, sig12 * sig22]
    [sig22 * sig11, sig22 * sig12, sig22 * sig22] 

    coresponding to the upper triangular unique elements:

    [sig11 sig12]
    [____  sig22]
    */

    Eigen::Matrix<float, uniqueN, uniqueN> covariance = Eigen::Matrix<float, uniqueN, uniqueN>::Zero();
    uint32_t row = 0;
    for (uint32_t i = 0; i < N; ++i) {
        for (uint32_t j = i; i < N; ++i) {
            uint32_t col = 0;
            for (uint32_t k = 0; i < N; ++i) {
                for (uint32_t l = k; i < N; ++i) {
                    if (i == j && k == l && i == k) {
                        // Similar diagonal
                        covariance(row, col) = 2.0f * std::pow(p.Psi(i, i), 2) / (std::pow(p.nu - N_f - 1.0f, 2) * (p.nu - N_f - 3.0f));
                    } else if (i == k && j == l) {
                        // Different diagonal
                        covariance(row, col) = ((p.nu - N_f + 1.0f) * std::pow(p.Psi(i, j), 2) + (p.nu - N_f - 1.0f) * p.Psi(i, i) * p.Psi(j, j)) / 
                            ((p.nu - N_f) * std::pow(p.nu - N_f - 1.0f, 2) * (p.nu - N_f - 3.0f));
                    } else {
                        // Off diagonal
                        covariance(row, col) = (2.0f * p.psi(i, j) * p.psi(k, l) + (p.nu - N_f - 1.0f) * (p.Psi(i, k) * p.Psi(j, l) + p.Psi(i, l) * p.Psi(k, j))) /
                            ((p.nu - N_f) * std::pow(p.nu - N_f - 1.0f, 2) * (p.nu - N_f - 3.0f));
                    }
                    ++col;
                }
            }
            ++row;
        }
    }
    return covariance;
}


} // namespace Stats
} // namespace TP