#pragma once

#include <cmath>

#include <Eigen/Core>

#include "TaskPlanner.h"

#include "TrajectoryDistributionEstimator.h"

namespace PRL {

template <std::size_t N>
class GaussianEFE {
    public:
        using ModelDistribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;
        //using TrajectoryDistribution = TP::Stats::Distributions::FixedMultivariateNormal<TP::Stats::Distributions::FixedNormalInverseWishart<N>::uniqueElements()>;

    public:
        static float calculate(const TrajectoryDistributionUpdaters<N>& traj_updaters, const ModelDistribution& pref_dist, uint32_t n_samples, float* information_gain = nullptr) {
            const auto& parameters_mvn = traj_updaters.getConvolutedEstimateMVN();
            //LOG("prior cov: \n" << parameters_mvn.Sigma);
            ModelDistribution ceq_obs_dist = getCEQObservationDistribution(traj_updaters);
            //LOG("preference likelihood: " << preferenceLikelihood(pref_dist, ceq_obs_dist));
            //LOG("prior entropy: " << parameters_mvn.entropy());
            //LOG("exp post entropy: " << expectedPosteriorEntropy(traj_updaters, n_samples));
            float info_gain = parameters_mvn.entropy() + expectedPosteriorEntropy(traj_updaters, n_samples);
            if (information_gain)
                *information_gain = info_gain;
            return preferenceLikelihood(pref_dist, ceq_obs_dist) - info_gain;
        }

        static ModelDistribution getCEQObservationDistribution(const TrajectoryDistributionUpdaters<N>& prior_updaters) {
            const auto& parameters_mvn = prior_updaters.getConvolutedEstimateMVN();
            Eigen::Matrix<float, TP::Stats::Distributions::FixedNormalInverseWishart<N>::uniqueElements(), 1> parameters_mean = TP::Stats::E(parameters_mvn);
            //LOG("parameters dist mean: \n" << parameters_mean << "\n");
            Eigen::Matrix<float, N, 1> mean_of_mean;
            Eigen::Matrix<float, ModelDistribution::uniqueCovarianceElements(), 1> mean_of_covariance;
            for (std::size_t i = 0; i < TP::Stats::Distributions::FixedNormalInverseWishart<N>::uniqueElements(); ++i) {
                if (i < N) 
                    mean_of_mean(i) = parameters_mean(i);
                else
                    mean_of_covariance(i - N) = parameters_mean(i);
            }
            ModelDistribution dist;
            dist.mu = mean_of_mean;
            dist.setSigmaFromUniqueElementVector(mean_of_covariance);
            return dist;
        }

    private:

        static float preferenceLikelihood(const ModelDistribution& pref_dist, const ModelDistribution& ceq_obs_dist) {
            float sum = std::log(std::pow(std::sqrt(M_2_PI), N) * pref_dist.Sigma.determinant());
            Eigen::Matrix<float, N, N> Sigma_ev_inv = pref_dist.Sigma.inverse();
            
            // First term
            sum += 0.5f * pref_dist.mu.transpose() * Sigma_ev_inv * pref_dist.mu;

            // Second term
            sum += - pref_dist.mu.transpose() * Sigma_ev_inv * ceq_obs_dist.mu;

            // Third term
            sum += 0.5f * ((Sigma_ev_inv * ceq_obs_dist.Sigma).trace() + ceq_obs_dist.mu.transpose() * Sigma_ev_inv * ceq_obs_dist.mu);
            return sum;
        }

        static float expectedPosteriorEntropy(const TrajectoryDistributionUpdaters<N>& prior_updaters, uint32_t n_samples) {
            std::vector<TP::Stats::Distributions::FixedMultivariateNormal<N>> estimate_dists = prior_updaters.getIndividualEstimateDistributions();
            const std::vector<TP::Stats::MultivariateGaussianUpdater<N>*>& updaters = prior_updaters.getIndividualUpdaters();
            
            // Make sampler for each individual dist, since we need to sample each (s,a)
            std::vector<TP::Stats::Distributions::FixedMultivariateNormalSampler<N>> samplers;
            samplers.reserve(estimate_dists.size());
            for (const auto& dist : estimate_dists) {
                samplers.emplace_back(dist);
            }

            float unnormalized_entropy = 0.0f;

            // Sample the expected entropy
            for (uint32_t s = 0; s < n_samples; ++s) {
                // Convoluted posterior estimate
                TrajectoryDistributionConvolver<N> posterior_convolver;
                auto sampler_it = samplers.begin();
                for (const auto& updater : updaters) {
                    Eigen::Matrix<float, N, 1> obs = TP::RNG::mvnrand(*sampler_it++);
                    //LOG(" sampled observation: " << obs.transpose());
                    TP::Stats::Distributions::FixedNormalInverseWishart<N> posterior = updater->tempPosterior(obs);
                    posterior_convolver.add(posterior);
                }
                
                unnormalized_entropy += posterior_convolver.getConvolutedEstimateMVN().entropy();
            }

            return unnormalized_entropy / static_cast<float>(n_samples);
        }
};

template <std::size_t N>
class CertaintyEquivalenceGaussianEFE {
    public:
        using Distribution = TP::Stats::Distributions::FixedMultivariateNormal<N>;

    public:
        static float calculate(const Distribution& input_dist, const Distribution& p_ev_dist) {
            return kld(input_dist, p_ev_dist) + entropy(input_dist.Sigma, true);
        }

    private:
        static float kld(const Distribution& lhs, const Distribution& rhs) {
            float det_rhs_covariance = rhs.Sigma.determinant();
            Eigen::Matrix<float, N, N> inv_rhs_covariance = rhs.Sigma.inverse();
            Eigen::Matrix<float, N, N> inv_rhs_cov_by_lhs_cov = inv_rhs_covariance * lhs.Sigma;
            Eigen::Matrix<float, N, 1> mean_diff = rhs.mu - lhs.mu;
            
            // Compute determinant and cache it for entropy calculation
            t_det_input_sigma_cache.det = lhs.Sigma.determinant();
            t_det_input_sigma_cache.cached = true;

            return 0.5f * (
                std::log(rhs.Sigma.determinant() / t_det_input_sigma_cache.det) 
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