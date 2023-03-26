#pragma once

#include <tuple>
#include <string>
#include <vector>
#include <memory>

#include "core/TransitionSystem.h"

namespace TP {
namespace DiscreteModel {

    struct RectangleGridWorldRegion {
        RectangleGridWorldRegion(const std::string& label_, uint32_t lower_left_x_, uint32_t lower_left_y_, uint32_t upper_right_x_, uint32_t upper_right_y_, const std::string& color_ = std::string()) 
            : label(label_)
            , lower_left_x(lower_left_x_)
            , lower_left_y(lower_left_y_)
            , upper_right_x(upper_right_x_) 
            , upper_right_y(upper_right_y_) 
            , color(color_) {}
        std::string label;
        uint32_t lower_left_x;
        uint32_t lower_left_y;
        uint32_t upper_right_x;
        uint32_t upper_right_y;
        std::string color = "orange";
    };

    struct GridWorldEnvironment {
        std::vector<RectangleGridWorldRegion> regions;
        void addRegion(const std::string& label, uint32_t lower_left_cell_x, uint32_t lower_left_cell_y, uint32_t upper_right_cell_x, uint32_t upper_right_cell_y) {
            regions.emplace_back(label, lower_left_cell_x, lower_left_cell_y, upper_right_cell_x, upper_right_cell_y);
        }
        void addRegion(const std::string& label, uint32_t lower_left_cell_x, uint32_t lower_left_cell_y, uint32_t upper_right_cell_x, uint32_t upper_right_cell_y, const std::string& color) {
            regions.emplace_back(label, lower_left_cell_x, lower_left_cell_y, upper_right_cell_x, upper_right_cell_y, color);
        }
        bool empty() const {return regions.empty();}
    };

    struct GridWorldAgentProperties {
        uint32_t n_x;
        uint32_t n_y;
        
        uint32_t init_coordinate_x;
        uint32_t init_coordinate_y;

        std::string coord_label_template = "x_#_y_#";

        GridWorldEnvironment environment;
        
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
            static GridWorldAgentProperties deserializeConfig(const std::string& filepath);

        private:
            static std::string templateToLabel(std::string label_template, uint32_t x, uint32_t y);
    };
}
}