#pragma once

#include "Grapefruit.h"
#include "EFE.h"

namespace PRL {

class FakePlan {
    public:
        FakePlan(uint32_t n_samples) : m_n_samples(n_samples) {}

        void deserialize(const GF::Deserializer& dszr) {
            const YAML::Node& node = dszr.get();
            uint32_t plan_length = node["plan_length"].as<uint32_t>();
            m_updaters.reserve(plan_length);

            // Preference distribution
            YAML::Node pref_dist_node = node["p_ev"];
            m_pref_dist.mu(0) = pref_dist_node["mu_0"].as<float>();
            m_pref_dist.mu(1) = pref_dist_node["mu_1"].as<float>();
            m_pref_dist.Sigma(0, 0) = pref_dist_node["Sigma_0_0"].as<float>();
            m_pref_dist.Sigma(1, 0) = pref_dist_node["Sigma_0_1"].as<float>();
            m_pref_dist.Sigma(0, 1) = pref_dist_node["Sigma_0_1"].as<float>();
            m_pref_dist.Sigma(1, 1) = pref_dist_node["Sigma_1_1"].as<float>();

            // Plan distributions
            for (uint32_t i = 0; i < plan_length; ++i) {
                YAML::Node dist_node = node["dist_" + std::to_string(i)];
                GF::Stats::MultivariateGaussianUpdater<2> updater;

                Eigen::Matrix<float, 2, 1> mu; 
                Eigen::Matrix<float, 2, 2> Lambda; 
                mu(0) = dist_node["lambda_n_0"].as<float>();
                mu(1) = dist_node["lambda_n_1"].as<float>();
                Lambda(0, 0) = dist_node["Lambda_0_0"].as<float>();
                Lambda(1, 0) = dist_node["Lambda_0_1"].as<float>();
                Lambda(0, 1) = dist_node["Lambda_0_1"].as<float>();
                Lambda(1, 1) = dist_node["Lambda_1_1"].as<float>();

                float kappa = dist_node["kappa_n"].as<float>();
                float nu = dist_node["nu"].as<float>();

                m_updaters.emplace_back(mu, Lambda, kappa, nu);
            }

            for (const auto& updater : m_updaters) 
                m_traj_updaters.add(updater);
        }

        float calculateEFE() const {
            return GaussianEFE<2>::calculate(m_traj_updaters, m_pref_dist, m_n_samples);
        }
    private:
        uint32_t m_n_samples;
        std::vector<GF::Stats::MultivariateGaussianUpdater<2>> m_updaters;
        TrajectoryDistributionUpdaters<2> m_traj_updaters;
        GF::Stats::Distributions::FixedMultivariateNormal<2> m_pref_dist;
};


}