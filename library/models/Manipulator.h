#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/TransitionSystem.h"

namespace TP {
namespace DiscreteModel {

    struct ManipulatorModelProperties {
        uint32_t n_locations;
        uint32_t n_objects;
        
        int32_t init_ee_location = s_stow;
        std::vector<int32_t> init_obj_locations;

        std::string location_label_template = "L#";
        std::string object_location_label_template = "obj_#_loc";
        bool include_stow = true;
        
        // Template delimeter
        inline static const char s_delimeter = '#';
        
        // If an init_obj_locations[i] equals this, its location is set to 'ee' and holding is set to 'T'
        inline static const int32_t s_ee_obj_location = -1;
        inline static const int32_t s_stow = -2;
    };

    class Manipulator {
        private:
            struct ConvertedProperties {
                std::vector<std::string> locations;
                std::vector<std::string> objects;
                std::vector<std::string> init_state_vars;
            };
        public:
            static std::shared_ptr<TransitionSystem> generate(const ManipulatorModelProperties& model_props);
            static State makeInitState(const ManipulatorModelProperties& model_props, const std::shared_ptr<TransitionSystem>& ts);

        private:
            static ConvertedProperties convertProperties(const ManipulatorModelProperties& model_props);
            static std::string templateToLabel(std::string label_template, uint32_t num);

    };
}
}