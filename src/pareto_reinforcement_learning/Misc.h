#pragma once

#include "Grapefruit.h"

namespace PRL {

template <uint64_t N>
GF::Stats::Distributions::FixedMultivariateNormal<N> deserializePreferenceDist(const std::string& config_filepath) {
    YAML::Node data;
    try {
        data = YAML::LoadFile(config_filepath);

        ASSERT(data["PRL Preference"], "Missing PRL Preference distribution");

        YAML::Node pref_node = data["PRL Preference"]; 
        std::vector<float> mean = pref_node["Mean"].as<std::vector<float>>();
        std::vector<float> minimal_covariance = pref_node["Covariance"].as<std::vector<float>>();
        ASSERT(mean.size() == N, "Mean (" << mean.size() <<") dimension does not match compile-time dimension (" << N << ")");
        ASSERT(minimal_covariance.size() == GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 
            "Mean (" << minimal_covariance.size() <<") dimension does not match compile-time minimal covariance dimension (" << GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements() << ")");

        // Convert to Eigen
        Eigen::Matrix<float, N, 1> mean_converted;
        Eigen::Matrix<float, GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 1> minimal_cov_converted;
        for (uint32_t i = 0; i < N; ++i)
            mean_converted(i) = mean[i];

        for (uint32_t i = 0; i < GF::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(); ++i)
            minimal_cov_converted(i) = minimal_covariance[i];
        
        GF::Stats::Distributions::FixedMultivariateNormal<N> dist;
        dist.mu = mean_converted;
        dist.setSigmaFromUniqueElementVector(minimal_cov_converted);
        ASSERT(GF_IS_COV_POS_SEMI_DEF(dist.Sigma), "Preference Covariance: \n" << dist.Sigma <<"\nis not positive semi-definite");
        return dist;
    } catch (YAML::ParserException e) {
        ERROR("Failed to load file" << config_filepath << " ("<< e.what() <<")");
        return GF::Stats::Distributions::FixedMultivariateNormal<N>();
    }
}

template <uint64_t N>
std::pair<bool, Eigen::Matrix<float, N, 1>> deserializeDefaultMean(const std::string& config_filepath) {
    YAML::Node data;
    try {
        data = YAML::LoadFile(config_filepath);

        if (!data["Default Transition Estimate Mean"]) 
            return std::make_pair(false, Eigen::Matrix<float, N, 1>());

        YAML::Node pref_node = data["PRL Preference"]; 
        std::vector<float> mean = data["Default Transition Estimate Mean"].as<std::vector<float>>();

        // Convert to Eigen
        Eigen::Matrix<float, N, 1> mean_converted;
        for (uint32_t i = 0; i < N; ++i)
            mean_converted(i) = mean[i];

        LOG("Default mean: " << mean_converted(0) << ", " << mean_converted(1));
        return std::make_pair(true, mean_converted);
    } catch (YAML::ParserException e) {
        ERROR("Failed to load file" << config_filepath << " ("<< e.what() <<")");
        return std::make_pair(false, Eigen::Matrix<float, N, 1>());
    }
}

}
