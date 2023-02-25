#include "GridWorldAgent.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace TP {
namespace DiscreteModel {

    std::shared_ptr<TransitionSystem> GridWorldAgent::generate(const GridWorldAgentProperties& model_props) {
        ConvertedProperties converted_props;

        ASSERT(model_props.init_coordinate_x < model_props.n_x, "Init x coordinate (" << model_props.init_coordinate_x << ") exceeds the x-grid-dimension");
        ASSERT(model_props.init_coordinate_y < model_props.n_y, "Init y coordinate (" << model_props.init_coordinate_y << ") exceeds the y-grid-dimension");


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
                State src_state(ss_grid_agent.get());

                src_state = {x_labels[i], y_labels[j]};

                bool stay_put = false;
                for (uint8_t dir=0; dir<4; ++dir) {
                    State dst_state = src_state;
                    switch (dir) {
                        case 0: // left 
                            if (i > 0) {
                                dst_state[s_x_coord_label] = x_labels[i - 1];
                                ts->connect(src_state, dst_state, TransitionSystemLabel(1.0f, "left"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, src_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }
                            continue;
                        case 1: // right
                            if (i < (model_props.n_x - 1)) {
                                dst_state[s_x_coord_label] = x_labels[i + 1];
                                ts->connect(src_state, dst_state, TransitionSystemLabel(1.0f, "right"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, src_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }

                            continue;
                        case 2: // down
                            if (j > 0) {
                                dst_state[s_y_coord_label] = y_labels[j - 1];
                                ts->connect(src_state, dst_state, TransitionSystemLabel(1.0f, "down"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, dst_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }

                            continue;
                        case 3: // up
                            if (j < (model_props.n_y - 1)) {
                                dst_state[s_y_coord_label] = y_labels[j + 1];
                                ts->connect(src_state, dst_state, TransitionSystemLabel(1.0f, "up"));
                            } else {
                                if (!stay_put) {
                                    ts->connect(src_state, dst_state, TransitionSystemLabel(0.0f, "stay"));
                                    stay_put = true;
                                } 
                            }
                            continue;
                    }
                }

                Condition ap;
                ap.addCondition(ConditionArg::Label, s_x_coord_label, ConditionOperator::Equals, ConditionArg::Variable, x_labels[i]);
                ap.addCondition(ConditionArg::Label, s_y_coord_label, ConditionOperator::Equals, ConditionArg::Variable, y_labels[j]);
                ap.setName(templateToLabel(model_props.coord_label_template, i, j));

                ts->addProposition(ap);
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

        out << YAML::EndMap;
        std::ofstream fout(filepath);
        fout << out.c_str();
    }
}
}