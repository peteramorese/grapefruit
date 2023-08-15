#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/TransitionSystem.h"
#include "models/ModelProperties.h"

namespace GF {
namespace DiscreteModel {

    struct ManipulatorModelProperties : public TransitionSystemModelProperties {
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

        /// @brief Properties are the simplest way to compare the equivalence of generated transtion systems
        virtual bool isEqual(const TransitionSystemModelProperties& other_) const override {
            auto other = dynamic_cast<const ManipulatorModelProperties&>(other_);
            return other.locations == locations
                && other.objects == objects
                && other.init_ee_location == init_ee_location
                && other.init_obj_locations == init_obj_locations
                && other.include_stow == include_stow;
        }
        virtual void serialize(GF::Serializer& szr) const override;
        virtual void deserialize(const GF::Deserializer& dszr) override;
    };


    class Manipulator {
        public:
            static std::shared_ptr<TransitionSystem> generate(const ManipulatorModelProperties& model_props);
            static State makeInitState(const ManipulatorModelProperties& model_props, const std::shared_ptr<TransitionSystem>& ts);
    };
}
}