#pragma once

#include "TaskPlanner.h"

namespace PRL {

template <uint64_t N>
TP::Stats::Distributions::FixedMultivariateNormal<N> deserializePreferenceDist(const std::string& config_filepath) {
    YAML::Node data;
    try {
        data = YAML::LoadFile(config_filepath);

        ASSERT(data["PRL Preference"], "Missing PRL Preference distribution");

        YAML::Node pref_node = data["PRL Preference"]; 
        std::vector<float> mean = pref_node["Mean"].as<std::vector<float>>();
        std::vector<float> minimal_covariance = pref_node["Covariance"].as<std::vector<float>>();
        ASSERT(mean.size() == N, "Mean (" << mean.size() <<") dimension does not match compile-time dimension (" << N << ")");
        ASSERT(minimal_covariance.size() == TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 
            "Mean (" << minimal_covariance.size() <<") dimension does not match compile-time minimal covariance dimension (" << TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements() << ")");

        // Convert to Eigen
        Eigen::Matrix<float, N, 1> mean_converted;
        Eigen::Matrix<float, TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(), 1> minimal_cov_converted;
        for (uint32_t i = 0; i < N; ++i)
            mean_converted(i) = mean[i];

        for (uint32_t i = 0; i < TP::Stats::Distributions::FixedMultivariateNormal<N>::uniqueCovarianceElements(); ++i)
            minimal_cov_converted(i) = minimal_covariance[i];
        
        TP::Stats::Distributions::FixedMultivariateNormal<N> dist;
        dist.mu = mean_converted;
        dist.setSigmaFromUniqueElementVector(minimal_cov_converted);
        ASSERT(TP::isCovariancePositiveSemiDef(dist.Sigma), "Preference Covariance: \n" << dist.Sigma <<"\nis not positive semi-definite");
        return dist;
    } catch (YAML::ParserException e) {
        ERROR("Failed to load file" << config_filepath << " ("<< e.what() <<")");
        return TP::Stats::Distributions::FixedMultivariateNormal<N>();
    }
}

}
