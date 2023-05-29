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
        std::string label;
        std::string proposition;
        uint32_t lower_left_x;
        uint32_t upper_right_x;
        uint32_t lower_left_y;
        uint32_t upper_right_y;
        std::string color;
    };

    struct Obstacle {
        std::string label;
        uint32_t lower_left_x;
        uint32_t upper_right_x;
        uint32_t lower_left_y;
        uint32_t upper_right_y;
        inline bool within(uint32_t x, uint32_t y) const {
            return x >= lower_left_x 
                && x <= upper_right_x 
                && y >= lower_left_y
                && y <= upper_right_y;
        }
    };

    struct GridWorldEnvironment {
        std::vector<RectangleGridWorldRegion> regions;
        std::vector<Obstacle> obstacles;
        bool inObstacle(uint32_t x, uint32_t y) const {
            for (const auto& obstacle : obstacles) {
                if (obstacle.within(x, y))
                    return true;
            }
            return false;
        }
        // TODO cost map
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
                : m_cell_exit_costs(n_x, std::vector<COST_T>(n_y, default_exit_cost))
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