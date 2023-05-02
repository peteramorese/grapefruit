#pragma once

#include <tuple>
#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "core/TransitionSystem.h"

namespace TP {
namespace DiscreteModel {

    struct RectangleGridWorldRegion {
        RectangleGridWorldRegion(const std::string& label_, uint32_t lower_left_x_, uint32_t lower_left_y_, uint32_t upper_right_x_, uint32_t upper_right_y_, const std::string& color_ = std::string(), float exit_cost = 1.0f) 
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
        float exit_cost = 1.0f;
    };

    struct GridWorldEnvironment {
        std::vector<RectangleGridWorldRegion> regions;
        void addRegion(const std::string& label, uint32_t lower_left_cell_x, uint32_t lower_left_cell_y, uint32_t upper_right_cell_x, uint32_t upper_right_cell_y) {
            regions.emplace_back(label, lower_left_cell_x, lower_left_cell_y, upper_right_cell_x, upper_right_cell_y);
        }
        void addRegion(const std::string& label, uint32_t lower_left_cell_x, uint32_t lower_left_cell_y, uint32_t upper_right_cell_x, uint32_t upper_right_cell_y, const std::string& color) {
            regions.emplace_back(label, lower_left_cell_x, lower_left_cell_y, upper_right_cell_x, upper_right_cell_y, color);
        }
        void addRegion(const std::string& label, uint32_t lower_left_cell_x, uint32_t lower_left_cell_y, uint32_t upper_right_cell_x, uint32_t upper_right_cell_y, const std::string& color, float exit_cost) {
            regions.emplace_back(label, lower_left_cell_x, lower_left_cell_y, upper_right_cell_x, upper_right_cell_y, color, exit_cost);
        }
        bool empty() const {return regions.empty();}
    };

    //class GridWorldCostMap {
    //    public:
    //        enum Direction {
    //            Left = 0,
    //            Right, Down, Up
    //        };

    //        GridWorldCostMap(uint32_t n_x, uint32_t n_y) 
    //            : cost_map(n_x, std::vector<float>(n_y))
    //        {}

    //        std::pair<uint32_t, uint32_t> size() const {
    //            if (cost_map.size() == 0u || cost_map.begin()->size() == 0u) return std::make_pair(0u, 0u);
    //            return std::make_pair(cost_map.size(), cost_map.begin()->size())
    //        };

    //        virtual float& operator()(uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y) {
    //            if (src_x != dst_x) {
    //                return (src_x > dst_x) ? cost_map[src_x][src_y][Direction::Left] : cost_map[src_x][src_y][Direction::Right];
    //            } else if (src_y != dst_y) {
    //                return (src_y > dst_y) ? cost_map[src_x][src_y][Direction::Down] : cost_map[src_x][src_y][Direction::Up];
    //            }
    //        }

    //    private:
    //        std::vector<std::vector<std::array<float, 4>>> cost_map;
    //};

    template <typename COST_T>
    class BadCellCostMap {

        public:
            BadCellCostMap(uint32_t n_x, uint32_t n_y, const COST_T& default_exit_cost) 
                : cell_exit_costs(n_x, std::vector<COST_T>(n_y, default_exit_cost))
            {}

            COST_T& get(uint32_t x, uint32_t y) {
                return m_cell_exit_costs[x][y];
            }

            COST_T& operator()(uint32_t src_x, uint32_t src_y, uint32_t dst_x, uint32_t dst_y) {
                return get(src_x, src_y);
            }
        private:
            std::vector<std::vector<COST_T>> m_cell_exit_costs;
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

        std::shared_ptr<BadCellCostMap<float>> cost_map;
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