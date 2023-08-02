#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/TransitionSystem.h"

namespace GF {
namespace DiscreteModel {

    struct ManipulatorModelProperties {
        std::vector<std::string> locations;
        std::vector<std::string> objects;
        
        std::string init_ee_location = s_stow;
        std::map<std::string, std::string> init_obj_locations;

        bool include_stow = true;
        
        // If an init_obj_locations[i] equals this, its location is set to 'ee' and holding is set to 'T'
        inline static const std::string s_ee_obj_location = "ee";
        inline static const std::string s_stow = "stow";

        std::vector<std::string> getInitStateVars() const {
            bool holding = false;
            std::vector<std::string> vars;
            vars.reserve(objects.size() + 2);
            ASSERT(include_stow || init_ee_location != s_stow, "Init ee location is 'stow', but the stow location is not included");
            vars.push_back(init_ee_location);
            for (const auto& obj : objects) {
                const std::string& loc = init_obj_locations.at(obj);
                vars.push_back(loc);

                if (loc == s_ee_obj_location) {
                    if (holding) {
                        ASSERT(false, "Duplicate 'ee' object location in initial state");
                    } else {
                        holding = true;
                    }
                }
            }
            vars.push_back(holding ? "T" : "F");
            return vars;
        }
    };

    class Manipulator {
        public:
            static std::shared_ptr<TransitionSystem> generate(const ManipulatorModelProperties& model_props);
            static State makeInitState(const ManipulatorModelProperties& model_props, const std::shared_ptr<TransitionSystem>& ts);

        private:
            //static ConvertedProperties convertProperties(const ManipulatorModelProperties& model_props);
            static std::string templateToLabel(std::string label_template, uint32_t num);

    };
}
}