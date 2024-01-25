#include "GridWorldAgent.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace GF {
namespace DiscreteModel {

    std::shared_ptr<TransitionSystem> GridWorldAgent::generate(const GridWorldAgentProperties& model_props) {
        ConvertedProperties converted_props;

        ASSERT(model_props.init_coordinate_x < model_props.n_x, "Init x coordinate (" << model_props.init_coordinate_x << ") exceeds the x-grid-dimension");
        ASSERT(model_props.init_coordinate_y < model_props.n_y, "Init y coordinate (" << model_props.init_coordinate_y << ") exceeds the y-grid-dimension");

        bool standard_propositions = model_props.environment.regions.empty();


        /////////////////   State Space   /////////////////

        std::shared_ptr<StateSpace> ss_grid_agent = std::make_shared<StateSpace>(2);;

        std::vector<std::string> x_labels(model_props.n_x);
        std::vector<std::string> y_labels(model_props.n_y);
        for (int i=0; i<model_props.n_x; ++i) {
            x_labels[i] = "x" + std::to_string(i);
        }
        for (int i=0; i<model_props.n_y; ++i) {
            y_labels[i] = "y" + std::to_string(i);
        }

        ss_grid_agent->setDimension(0, s_x_coord_label, x_labels); 
        ss_grid_agent->setDimension(1, s_y_coord_label, y_labels); 

        /////////////////   Initial State   /////////////////

        State init_state(ss_grid_agent.get());	
        init_state[s_x_coord_label] = x_labels[model_props.init_coordinate_x];
        init_state[s_y_coord_label] = y_labels[model_props.init_coordinate_y];

        std::shared_ptr<TransitionSystem> ts = std::make_shared<TransitionSystem>(ss_grid_agent);


        for (uint32_t i=0; i<model_props.n_x; ++i) { // x
            for (uint32_t j=0; j<model_props.n_y; ++j) { // y
                if (model_props.environment.inObstacle(i, j))
                    continue;
                State src_state(ss_grid_agent.get());

                src_state = {x_labels[i], y_labels[j]};

                bool stay_put = false;
                for (uint8_t dir=0; dir<4; ++dir) {
                    State dst_state = src_state;
                    switch (dir) {
                        case 0: // left 
                            if (i > 0) {
                                if (model_props.environment.inObstacle(i - 1, j))
                                    continue;
                                dst_state[s_x_coord_label] = x_labels[i - 1];
                                float cost = (model_props.cost_map) ? model_props.cost_map->operator()(i, j, i-1, j) : 1.0f;
                                ts->connect(src_state, dst_state, TransitionSystemLabel(cost, "left"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, src_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }
                            continue;
                        case 1: // right
                            if (i < (model_props.n_x - 1)) {
                                if (model_props.environment.inObstacle(i + 1, j))
                                    continue;
                                dst_state[s_x_coord_label] = x_labels[i + 1];
                                float cost = (model_props.cost_map) ? model_props.cost_map->operator()(i, j, i+1, j) : 1.0f;
                                ts->connect(src_state, dst_state, TransitionSystemLabel(cost, "right"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, src_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }

                            continue;
                        case 2: // down
                            if (j > 0) {
                                if (model_props.environment.inObstacle(i, j - 1))
                                    continue;
                                dst_state[s_y_coord_label] = y_labels[j - 1];
                                float cost = (model_props.cost_map) ? model_props.cost_map->operator()(i, j, i, j-1) : 1.0f;
                                ts->connect(src_state, dst_state, TransitionSystemLabel(cost, "down"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, dst_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }

                            continue;
                        case 3: // up
                            if (j < (model_props.n_y - 1)) {
                                if (model_props.environment.inObstacle(i, j + 1))
                                    continue;
                                dst_state[s_y_coord_label] = y_labels[j + 1];
                                float cost = (model_props.cost_map) ? model_props.cost_map->operator()(i, j, i, j+1) : 1.0f;
                                ts->connect(src_state, dst_state, TransitionSystemLabel(cost, "up"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, dst_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }
                            continue;
                    }
                }

                if (standard_propositions) {
                    Condition ap;
                    ap.addCondition(ConditionArg::Label, s_x_coord_label, ConditionOperator::Equals, ConditionArg::Variable, x_labels[i]);
                    ap.addCondition(ConditionArg::Label, s_y_coord_label, ConditionOperator::Equals, ConditionArg::Variable, y_labels[j]);
                    ap.setName(templateToLabel(model_props.coord_label_template, i, j));

                    ts->addProposition(ap);
                }
            }
        }

        if (!standard_propositions) {
            for (const auto& region : model_props.environment.regions) {
                for (uint32_t i = region.lower_left_x; i <= region.upper_right_x; ++i) {
                    for (uint32_t j = region.lower_left_y; j <= region.upper_right_y; ++j) {
                        Condition ap;
                        ap.addCondition(ConditionArg::Label, s_x_coord_label, ConditionOperator::Equals, ConditionArg::Variable, x_labels[i]);
                        ap.addCondition(ConditionArg::Label, s_y_coord_label, ConditionOperator::Equals, ConditionArg::Variable, y_labels[j]);
                        LOG("cell " << x_labels[i] << ", " << y_labels[j] << " label: " << region.proposition);
                        ap.setName(region.proposition);

                        ts->addProposition(ap);
                    }
                }
            }
        }

        return ts;
    }

    State GridWorldAgent::makeInitState(const GridWorldAgentProperties& model_props, const std::shared_ptr<TransitionSystem>& ts) {
        State init_state(ts->getStateSpace().lock().get());
        init_state[s_x_coord_label] = "x" + std::to_string(model_props.init_coordinate_x);
        init_state[s_y_coord_label] = "y" + std::to_string(model_props.init_coordinate_y);
        return init_state;
    }

    std::string GridWorldAgent::templateToLabel(std::string label_template, uint32_t x, uint32_t y) {
        uint32_t i = 0;
        bool on_x = true;
        bool on_y = false;
        while (i < label_template.size()) {
            if (label_template[i] == GridWorldAgentProperties::s_delimeter) {
                uint32_t num = (on_x) ? x : y;
                label_template.replace(i, 1, std::to_string(num));
                if (on_x) {
                    on_x = false;
                    on_y = true;
                } else {
                    ASSERT(on_y, "Too many delimeters in label template: " << label_template);
                    on_y = false;
                }
            }
            ++i;
        }
        return label_template;
    }

    void GridWorldAgent::serializeConfig(const GridWorldAgentProperties& model_props, const std::string& filepath) {

        YAML::Emitter out;

        out << YAML::BeginMap;

        out << YAML::Key << "Grid X" << YAML::Value << model_props.n_x;
        out << YAML::Key << "Grid Y" << YAML::Value << model_props.n_y;

        out << YAML::Key << "Init X Coord" << YAML::Value << model_props.init_coordinate_x;
        out << YAML::Key << "Init Y Coord" << YAML::Value << model_props.init_coordinate_y;

        out << YAML::Key << "Coord Label Template" << YAML::Value << model_props.coord_label_template;

        if (!model_props.environment.regions.empty()) {
            out << YAML::Key << "N Regions" << YAML::Value << model_props.environment.regions.size();

            out << YAML::Key << "Regions Labels" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& region : model_props.environment.regions) {
                out << region.label;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "Lower Left Cells X" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& region : model_props.environment.regions) {
                out << region.lower_left_x;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "Lower Left Cells Y" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& region : model_props.environment.regions) {
                out << region.lower_left_y;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "Upper Right Cells X" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& region : model_props.environment.regions) {
                out << region.upper_right_x;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "Upper Right Cells Y" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& region : model_props.environment.regions) {
                out << region.upper_right_y;
            }
            out << YAML::EndSeq;

            out << YAML::Key << "Region Colors" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& region : model_props.environment.regions) {
                out << region.color;
            }
            out << YAML::EndSeq;

        }

        out << YAML::EndMap;
        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    GridWorldAgentProperties GridWorldAgent::deserializeConfig(const std::string& filepath) {
        GridWorldAgentProperties props;
        YAML::Node data;
        try {
            data = YAML::LoadFile(filepath);

            props.n_x = data["Grid X"].as<uint32_t>();
            props.n_y = data["Grid Y"].as<uint32_t>();
            props.init_coordinate_x = data["Init X Coord"].as<uint32_t>();
            props.init_coordinate_y = data["Init Y Coord"].as<uint32_t>();
            props.coord_label_template = data["Coord Label Template"].as<std::string>();

            if (data["Region Labels"]) {
                for (const auto& label : data["Region Labels"].as<std::vector<std::string>>()) {
                    ASSERT(data[label], "Region: " << label << " was specified in 'Region Labels', but data not found");
                    YAML::Node region_node = data[label];
                    std::vector<uint32_t> bounds = region_node["Bounds"].as<std::vector<uint32_t>>();
                    ASSERT(bounds.size() == 4, "Must specify four bounds (x_lower, x_upper, y_lower, y_upper)");
                    std::string color = region_node["Color"].as<std::string>();

                    std::string proposition = label;
                    if (region_node["Proposition"]) // proposition is overriden if the key exists
                        proposition = region_node["Proposition"].as<std::string>();

                    RectangleGridWorldRegion region{label, proposition, bounds[0], bounds[1], bounds[2], bounds[3], color};
                    props.environment.regions.push_back(std::move(region));
                }
            }
            if (data["Obstacles"]) {
                YAML::Node obstacles_node = data["Obstacles"];
                for (YAML::const_iterator it = obstacles_node.begin(); it != obstacles_node.end(); ++it) {
                    std::string label = it->first.as<std::string>();
                    //YAML::Node bounds_node = it->second;
                    std::vector<uint32_t> bounds = it->second["Bounds"].as<std::vector<uint32_t>>();
                    ASSERT(bounds.size() == 4, "Must specify four bounds (x_lower, x_upper, y_lower, y_upper)");
                    Obstacle obstacle{label, bounds[0], bounds[1], bounds[2], bounds[3]};
                    props.environment.obstacles.push_back(std::move(obstacle));
                }
            }

        } catch (YAML::ParserException e) {
            ERROR("Failed to load file" << filepath << " ("<< e.what() <<")");
        }
        return props;
    }

}
}