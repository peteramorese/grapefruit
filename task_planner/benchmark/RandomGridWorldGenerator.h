#pragma once

#include "models/GridWorldAgent.h"

namespace TP {
namespace DiscreteModel {

struct RandomGridWorldProperties {

};

class RandomGridWorldGenerator {
    public:
        static RandomGridWorldProperties deserializeConfig(const std::string& filepath);
        static GridWorldAgentProperties generateProps(const GridWorldAgentProperties& random_model_props);
    private:
};

}
}