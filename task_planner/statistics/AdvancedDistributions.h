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
        FixedInverseWishart() = default;
        FixedInverseWishart(float nu_, const Eigen::Matrix<float, N, N>& Psi_)
            : nu(nu_)
            , Psi(Psi_)
        {}

        // TODO normalizationConstant()
        // TODO pdf()
};

template <std::size_t N>
struct MinimalFixedInverseWishart {
    
    /*
    Unique elements correspond to the upper triangular elements iterating over columns then rows, i.e.

    Psi_minimal = [sig11 sig12 sig13 sig22 sig23 sig33]^T

    coresponding to the upper triangular unique elements of Psi:

    [sig11 sig12 sig13]
    [____  sig22 sig23]
    [____  _____ sig33]
    */

    private:
        inline static constexpr std::size_t N_minimal = N * (N + 1u) / 2;
        static_assert(N >= 2, "Use Gamma distribution for 1D scalar variance");

    public:
        float nu = static_cast<float>(N);
        Eigen::Matrix<float, N_minimal, 1> Psi_minimal;

    public:
        MinimalFixedInverseWishart() {
            std::size_t diff = 0;
            std::size_t next_diff = N;
            for (std::size_t i = 0; i < N_minimal; ++i) {
                if (diff == next_diff || i == 0) {
                    Psi_minimal(i, 0) = 1.0f; // diagonal
                    --next_diff;
                    diff = 0;
                } else {
                    Psi_minimal(i, 0) = 0.0f; // off diagonal
                }
                ++diff;
            }
        }
        MinimalFixedInverseWishart(float nu_, const Eigen::Matrix<float, N, N>& Psi_)
            : nu(nu_)
        {
            std::size_t i_minimal = 0;
            for (std::size_t i = 0; i < N; ++i) {
                for (std::size_t j = i; j < N; ++j) {
                    Psi_minimal(i_minimal++, 1) = Psi_(i, j);
                }
            }
        }

        // Order switched to differentiate from full-Psi ctor
        MinimalFixedInverseWishart(const Eigen::Matrix<float, N_minimal, 1>& Psi_minimal_, float nu_)
            : nu(nu_)
            , Psi_minimal(Psi_minimal_)
        {}

        static constexpr std::size_t uniqueElements() {return N_minimal;}
};


} // namespace Distributions

// Conversion between minimal and full Wishart representations
template <std::size_t N>
Distributions::FixedInverseWishart<N> minimalWishartToWishart(const Distributions::MinimalFixedInverseWishart<N> minimal_ws) {
    Eigen::Matrix<float, N, N> Psi;
    std::size_t i_minimal = 0;
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = i; j < N; ++j) {
            Psi(i, j) = minimal_ws.Psi_minimal(i_minimal++);
        }
    }
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = i + 1; j < N; ++j) {
            Psi(j, i) = Psi(i, j);
        }
    }
    return Distributions::FixedInverseWishart<N>(minimal_ws.nu, Psi);
}

template <std::size_t N>
Distributions::MinimalFixedInverseWishart<N> wishartToMinimalWishart(const Distributions::FixedInverseWishart<N> ws) {
    Eigen::Matrix<float, Distributions::MinimalFixedInverseWishart<N>::uniqueElements(), 1> Psi_minimal;
    std::size_t i_minimal = 0;
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = i; j < N; ++j) {
            Psi_minimal(i_minimal++) = ws.Psi(i, j);
        }
    }
    return Distributions::MinimalFixedInverseWishart<N>(Psi_minimal, ws.nu);
}

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

// TODO: FixedInverseWishart
template <std::size_t N>
inline static Eigen::Matrix<float, N, N> E(const Distributions::FixedInverseWishart<N>& p) {
    ASSERT(p.nu > static_cast<float>(N + 1u), "DOF must be greater than N + 1"); 
    return p.Psi * (1.0f / (p.nu - static_cast<float>(N) - 1.0f));
}
template <std::size_t N> 
static Eigen::Matrix<float, N * N, N * N> var(const Distributions::FixedInverseWishart<N>& p) {
    float N_f = static_cast<float>(N);
    ASSERT(p.nu >= static_cast<float>(N), "DOF must be geq than N");
    Eigen::Matrix<float, N * N, N * N> covariance = Eigen::Matrix<float, N * N, N * N>::Zero();
    LOG("sz: " << N * N << " rows: " << covariance.rows() << " cols: " << covariance.cols());
    
    /*
    Rows and columns are ordered acording to the order read from right to left over each row, i.e.

    [sig11 * sig11, sig11 * sig12, sig11 * sig21, sig11 * sig22]
    [sig12 * sig11, sig12 * sig12, sig12 * sig21, sig12 * sig22]
    [sig21 * sig11, sig21 * sig12, sig21 * sig21, sig21 * sig22] 
    [sig22 * sig11, sig22 * sig12, sig22 * sig21, sig22 * sig22] 

    coresponding to the upper triangular unique elements:

    [sig11 sig12]
    [sig21 sig22]
    */

    std::size_t row = 0;
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            std::size_t col = 0;
            for (std::size_t k = 0; k < N; ++k) {
                for (std::size_t l = 0; l < N; ++l) {
                    //LOG("element r: " << row << " c: " << col << " = sig" << i << j << "*sig" << k << l);
                    if (i == j && k == l && i == k) {
                        // Similar diagonal
                        covariance(row, col) = 2.0f * std::pow(p.Psi(i, i), 2) / (std::pow(p.nu - N_f - 1.0f, 2) * (p.nu - N_f - 3.0f));
                    } else if (i == k && j == l) {
                        // Different diagonal
                        covariance(row, col) = ((p.nu - N_f + 1.0f) * std::pow(p.Psi(i, j), 2) + (p.nu - N_f - 1.0f) * p.Psi(i, i) * p.Psi(j, j)) / 
                            ((p.nu - N_f) * std::pow(p.nu - N_f - 1.0f, 2) * (p.nu - N_f - 3.0f));
                    } else {
                        // Off diagonal
                        covariance(row, col) = (2.0f * p.Psi(i, j) * p.Psi(k, l) + (p.nu - N_f - 1.0f) * (p.Psi(i, k) * p.Psi(j, l) + p.Psi(i, l) * p.Psi(k, j))) /
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

// MinimalFixedInverseWishart
template <std::size_t N>
inline static Eigen::Matrix<float, Distributions::FixedInverseWishart<N>::uniqueElements(), 1> E(const Distributions::MinimalFixedInverseWishart<N>& p) {
    ASSERT(p.nu > static_cast<float>(N + 1u), "DOF must be greater than N + 1"); 
    return p.Psi_minimal * (1.0f / (p.nu - static_cast<float>(N) - 1.0f));
}
template <std::size_t N> 
static Eigen::Matrix<float, Distributions::MinimalFixedInverseWishart<N>::uniqueElements(), Distributions::MinimalFixedInverseWishart<N>::uniqueElements()> var(const Distributions::MinimalFixedInverseWishart<N>& p) {
    constexpr uint32_t uniqueN = Distributions::MinimalFixedInverseWishart<N>::uniqueElements();
    float N_f = static_cast<float>(N);
    ASSERT(p.nu > static_cast<float>(N + 1u), "DOF must be creater than N (+ 1) <- check wikipedia");
    Eigen::Matrix<float, uniqueN, uniqueN> covariance = Eigen::Matrix<float, uniqueN, uniqueN>::Zero();
    Distributions::FixedInverseWishart<N> p_full = minimalWishartToWishart(p);
    
    /*
    Unique elements correspond to the upper triangular elements iterating over columns then rows, i.e.

    covariance = 
        [sig11 * sig11, sig11 * sig12, sig11 * sig22]
        [sig12 * sig11, sig12 * sig12, sig12 * sig22]
        [sig22 * sig11, sig22 * sig12, sig22 * sig22] 

    coresponding to the upper triangular unique elements:

    [sig11 sig12]
    [____  sig22]
    */

    std::size_t row = 0;
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = i; j < N; ++j) {
            std::size_t col = 0;
            for (std::size_t k = 0; k < N; ++k) {
                for (std::size_t l = k; l < N; ++l) {
                    if (i == j && k == l && i == k) {
                        // Similar diagonal
                        covariance(row, col) = 2.0f * std::pow(p_full.Psi(i, i), 2) / (std::pow(p.nu - N_f - 1.0f, 2) * (p.nu - N_f - 3.0f));
                    } else if (i == k && j == l) {
                        // Different diagonal
                        covariance(row, col) = ((p.nu - N_f + 1.0f) * std::pow(p_full.Psi(i, j), 2) + (p.nu - N_f - 1.0f) * p_full.Psi(i, i) * p_full.Psi(j, j)) / 
                            ((p.nu - N_f) * std::pow(p.nu - N_f - 1.0f, 2) * (p.nu - N_f - 3.0f));
                    } else {
                        // Off diagonal
                        covariance(row, col) = (2.0f * p_full.Psi(i, j) * p_full.Psi(k, l) + (p.nu - N_f - 1.0f) * (p_full.Psi(i, k) * p_full.Psi(j, l) + p_full.Psi(i, l) * p_full.Psi(k, j))) /
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