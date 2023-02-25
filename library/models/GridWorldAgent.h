#pragma once

#include <tuple>
#include <string>
#include <vector>
#include <memory>

#include "core/TransitionSystem.h"

namespace TP {
namespace DiscreteModel {

    struct GridWorldAgentProperties {
        uint32_t n_x;
        uint32_t n_y;
        
        uint32_t init_coordinate_x;
        uint32_t init_coordinate_y;

        std::string coord_label_template = "x_#_y_#";
        
        // Template delimeter
        inline static const char s_delimeter = '#';
    };

    class GridWorldAgent {
        public:
            inline static const std::string s_x_coord_label = "x_coord";
            inline static const std::string s_y_coord_label = "y_coord";

        private:
            struct ConvertedProperties {
                std::vector<std::string> locations;
                std::vector<std::string> init_state_vars;
            };
        public:
            static std::shared_ptr<TransitionSystem> generate(const GridWorldAgentProperties& model_props);
            static State makeInitState(const GridWorldAgentProperties& model_props, const std::shared_ptr<TransitionSystem>& ts);
            static void serializeConfig(const GridWorldAgentProperties& model_props, const std::string& filepath);

        private:
            static std::string templateToLabel(std::string label_template, uint32_t x, uint32_t y);
    };
}
}