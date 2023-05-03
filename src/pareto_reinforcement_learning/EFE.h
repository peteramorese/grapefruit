#pragma once

#include <cmath>

#include <Eigen/Core>

#include "TaskPlanner.h"


namespace PRL {

template <std::size_t N>
class GuassianEFE {
    public:
        using Distribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;

    public:
        static float calculate(const Distribution& input_dist, const Distribution& p_ev_dist) {
            return kld(input_dist, p_ev_dist) + entropy(input_dist.covariance, true);
        }

    private:
        static float kld(const Distribution& lhs, const Distribution& rhs) {
            float det_rhs_covariance = rhs.covariance.determinant();
            Eigen::Matrix<float, N, N> inv_rhs_covariance = rhs.covariance.inverse();
            Eigen::Matrix<float, N, N> inv_rhs_cov_by_lhs_cov = inv_rhs_covariance * lhs.covariance;
            Eigen::Matrix<float, N, 1> mean_diff = rhs.mu - lhs.mu;
            
            // Compute determinant and cache it for entropy calculation
            t_det_input_sigma_cache.det = lhs.covariance.determinant();
            t_det_input_sigma_cache.cached = true;

            //LOG("input det: " << t_det_input_sigma_cache.det);
            //LOG("rhs det: " << rhs.covariance.determinant());
            //LOG("inv trace: " << inv_rhs_cov_by_lhs_cov.trace());
            //LOG("First term: " << std::log(rhs.covariance.determinant() / t_det_input_sigma_cache.det));
            //LOG("Fourth term: " << mean_diff.transpose() * inv_rhs_covariance * mean_diff);
            //Eigen::IOFormat OctaveFmt(Eigen::StreamPrecision, 0, ", ", ";\n", "", "", "[", "]");
            //LOG("rhs_covariance: \n" << rhs.covariance.format(OctaveFmt));
            //LOG("inv_rhs_covariance: \n" << inv_rhs_covariance.format(OctaveFmt));
            return 0.5f * (
                std::log(rhs.covariance.determinant() / t_det_input_sigma_cache.det) 
                - static_cast<float>(N)
                + inv_rhs_cov_by_lhs_cov.trace()
                + mean_diff.transpose() * inv_rhs_covariance * mean_diff
            );
        }

        static float entropy(const Eigen::Matrix<float, N, N>& covariance, bool use_cache = true) {
            return 0.5f * (
                std::log((use_cache && t_det_input_sigma_cache.cached) ? t_det_input_sigma_cache.det : covariance.determinant())
                + static_cast<float>(N) * (1.0f + std::log(M_2_PI))
            );
        }

    private:
        struct InputSigmaDeterminantCache {
            float det = 0.0f;
            bool cached = false;
        };

    private:
        inline static InputSigmaDeterminantCache t_det_input_sigma_cache;
};

}