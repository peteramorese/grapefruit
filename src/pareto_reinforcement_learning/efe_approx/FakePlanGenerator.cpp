#include <Grapefruit.h>

#include "Misc.h"

using namespace PRL;
constexpr uint64_t N = 2;

int main(int argc, char* argv[]) {

	GF::ArgParser parser(argc, argv);

	auto n_truth_samples = parser.parse<uint32_t>("n-truth-samples", 'n', "Number of samples from the true distribution");
	auto m = parser.parse<uint32_t>("plan-length", 'm', "Length of plan");
	auto file_prefix = parser.parse<std::string>("file_prefix", "fakeplan", "Length of plan");
	auto config_filepath = parser.parse<std::string>("config-filepath", "config.yaml", "Config containing the preference distribution");

    parser.enableHelp();

    GF::Serializer szr(file_prefix.get() + ".yaml");
    YAML::Emitter& out = szr.get();

    GF::Stats::Distributions::FixedMultivariateNormal<2> p_ev = deserializePreferenceDist<2>(config_filepath.get());
    out << YAML::Key << "p_ev" << YAML::Value << YAML::BeginMap;

    out << YAML::Key << "mu_0" << YAML::Value << p_ev.mu(0);
    out << YAML::Key << "mu_1" << YAML::Value << p_ev.mu(1);
    out << YAML::Key << "Sigma_0_0" << YAML::Value << p_ev.Sigma(0, 0);
    out << YAML::Key << "Sigma_0_1" << YAML::Value << p_ev.Sigma(0, 1);
    out << YAML::Key << "Sigma_1_1" << YAML::Value << p_ev.Sigma(1, 1);

    out << YAML::EndMap;

    out << YAML::Key << "plan_length" << YAML::Value << m.get();

    std::vector<GF::Stats::MultivariateGaussianUpdater<2>> updaters(m.get());

    uint32_t action_i = 0u;
    for (auto& updater : updaters) {
        // Make a random truth distribution
        Eigen::Matrix<float, N, 1> mu; 
        Eigen::Matrix<float, N, N> Sigma;
        mu(0) = GF::RNG::randf(0.1f, 20.0f);
        mu(1) = GF::RNG::randf(0.1f, 20.0f);
        Sigma(0, 0) = GF::RNG::randf(0.01f, 5.0f);
        Sigma(0, 1) = GF::RNG::randf(0.01f, 5.0f);
        Sigma(1, 0) = Sigma(0, 1);
        Sigma(1, 1) = GF::RNG::randf(0.01f, 5.0f);
        // Make positive semi def
        Sigma = Sigma * Sigma;


        GF::Stats::Distributions::FixedMultivariateNormal<2> dist(mu, Sigma);
        GF::Stats::Distributions::FixedMultivariateNormalSampler<2> sampler(dist);
        for (uint32_t i = 0; i < n_truth_samples.get(); ++i) {
            Eigen::Matrix<float, N, 1> sample = GF::RNG::mvnrand(sampler);
            updater.update(sample);
        }
        GF::Stats::Distributions::FixedNormalInverseWishart<N> posterior = updater.dist();

        out << YAML::Key << "dist_" + std::to_string(action_i++) << YAML::Value << YAML::BeginMap;

        out << YAML::Key << "kappa_n" << YAML::Value << posterior.kappa;
        out << YAML::Key << "Lambda_0_0" << YAML::Value << posterior.Lambda(0,0);
        out << YAML::Key << "Lambda_0_1" << YAML::Value << posterior.Lambda(0,1);
        out << YAML::Key << "Lambda_1_1" << YAML::Value << posterior.Lambda(1,1);
        out << YAML::Key << "lambda_n_0" << YAML::Value << posterior.mu(0);
        out << YAML::Key << "lambda_n_1" << YAML::Value << posterior.mu(1);
        out << YAML::Key << "nu" << YAML::Value << posterior.nu;

        out << YAML::EndMap;

    }

    szr.done();

    return 0;
}