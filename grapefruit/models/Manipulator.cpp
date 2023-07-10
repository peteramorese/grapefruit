#include "Manipulator.h"

namespace GF {
namespace DiscreteModel {

    Manipulator::ConvertedProperties Manipulator::convertProperties(const ManipulatorModelProperties& model_props) {
        ConvertedProperties converted_props;

        // Set up
        ASSERT(model_props.n_locations > model_props.n_objects, "Number of locations must be greater than number of objects");
        ASSERT(model_props.init_obj_locations.size() == model_props.n_objects, "Number of init obj locations must match the number of objects");

        converted_props.locations.resize(model_props.n_locations);
        auto& locations = converted_props.locations;
        for (uint32_t i=0; i<model_props.n_locations; ++i) locations[i] = templateToLabel(model_props.location_label_template, i);

        converted_props.objects.resize(model_props.n_objects);
        auto& objects = converted_props.objects;
        for (uint32_t i=0; i<model_props.n_objects; ++i) objects[i] = templateToLabel(model_props.object_location_label_template, i);

        // Init state
        converted_props.init_state_vars.resize(model_props.n_objects + 2);
        auto& init_state_vars = converted_props.init_state_vars;

        if (model_props.init_ee_location >= 0) {
            init_state_vars[0] = locations[model_props.init_ee_location];
        } else if (model_props.init_ee_location == ManipulatorModelProperties::s_stow) {
            init_state_vars[0] = "stow";
        } else {
            ASSERT(false, "Unrecognized init ee location");
        }

        bool holding = false;
        for (uint32_t i=0; i<model_props.n_objects; ++i) {
            if (model_props.init_obj_locations[i] >= 0) {
                init_state_vars[i + 1] = locations[model_props.init_obj_locations[i]];
            } else if (model_props.init_obj_locations[i] == ManipulatorModelProperties::s_ee_obj_location) {
                if (holding) ASSERT(false, "More than one init obj location is 'ee'");
                init_state_vars[i + 1] = "ee";
                holding = true;
            } else {
                ASSERT(false, "Unrecognized init obj location");
            }
        }
        init_state_vars.back() = (holding) ? "T" : "F";

        return converted_props;
    }

    State Manipulator::makeInitState(const ManipulatorModelProperties& model_props, const std::shared_ptr<TransitionSystem>& ts) {
        return State(ts->getStateSpace().lock().get(), Manipulator::convertProperties(model_props).init_state_vars);
    }

    std::shared_ptr<TransitionSystem> Manipulator::generate(const ManipulatorModelProperties& model_props) {


        auto[locations, objects, init_state_vars] = Manipulator::convertProperties(model_props);


        /////////////////   State Space   /////////////////

        std::shared_ptr<StateSpace> ss_manipulator = std::make_shared<StateSpace>(objects.size() + 2);

        std::vector<std::string> ee_locations = locations;
        if (model_props.include_stow) ee_locations.push_back("stow");

        std::vector<std::string> obj_locations = locations;
        obj_locations.push_back("ee");
        
        // Create state space:
        ss_manipulator->setDimension(0, "ee_loc", ee_locations); // end effector locations
        uint32_t i = 1;
        for (const auto& obj : objects) ss_manipulator->setDimension(i++, obj, obj_locations); 
        ss_manipulator->setDimension(i, "holding", {"T", "F"}); // end effector is holding an object

        // Add a label group denoting the obj locations
        ss_manipulator->addGroup("obj_locations", objects);

        // Add a domain capturing only the locations that an object can be dropped in ('locations')
        ss_manipulator->addDomain("drop_locs", locations);

        TransitionSystemProperties props(ss_manipulator);

        /////////////////   TransitionConditions   /////////////////

        props.conditions.reserve(4);
        {
        TransitionCondition cond("grasp", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

        // Pre: "End effector is not holding and the variable in 'ee_loc' is found in group 'obj_locations'"
        cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); 
        cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg");

        // Post: "End effector is holding and the label found from 'ArgFind' in the precondition assumes the variable 'ee'"
        cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); 
        cond.addCondition(ConditionType::Post, ConditionArg::Variable, "ee", ConditionOperator::Equals, ConditionArg::ArgLabel, "", "arg");

        props.conditions.push_back(cond);
        }
        {
        TransitionCondition cond("transport", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

        // Pre: "End effector is holding and ee_loc variable does not match any in obj_locations"
        cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T");
        cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg_1", ConditionLogical::Negate);
        cond.addCondition(ConditionType::Pre, ConditionArg::Label, "ee_loc", ConditionOperator::ArgFind, ConditionArg::None, "", "arg_2");

        // Post: "End effector is holding and the arg variable stored in 'arg2' (the ee_loc) does not equal the new variable stored in ee_loc"
        cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T");
        cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgVariable, "", "arg_2", ConditionLogical::Negate); // ee_loc has changed
        cond.addCondition(ConditionType::Post, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "na", ConditionLogical::Negate); // ee_loc is not in an object's location

        props.conditions.push_back(cond);
        }
        {
        TransitionCondition cond("release", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

        // Pre: "End effector is holding and ee_loc var is not found among obj_locations and 'ee' is found among obj_locations"
        cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "T"); 
        cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Label, "ee_loc", "arg_1", ConditionLogical::Negate);
        cond.addCondition(ConditionType::Pre, ConditionArg::Group, "obj_locations", ConditionOperator::ArgFind, ConditionArg::Variable, "ee", "arg_2");

        // Post: "End effector is no longer holding, the arg label found in 'arg_2' equals the variable in ee_loc"
        cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F"); 
        cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgLabel, "", "arg_2"); 

        // Force ee_loc to stay the same
        cond.forceExclusionComparison("ee_loc");

        props.conditions.push_back(cond);
        }
        {
        TransitionCondition cond("transit", 1.0f, ConditionJunction::Conjunction, ConditionJunction::Conjunction); // Each sub-condition is conjoined

        // Pre: "End effector is not holding"
        cond.addCondition(ConditionType::Pre, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F");
        cond.addCondition(ConditionType::Pre, ConditionArg::Label, "ee_loc", ConditionOperator::ArgFind, ConditionArg::None, "", "arg");

        // Post: "End effector is not holding and the arg variable stored in 'arg' (the ee_loc) does not equal the new variable stored in ee_loc"
        cond.addCondition(ConditionType::Post, ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F");
        cond.addCondition(ConditionType::Post, ConditionArg::Label, "ee_loc", ConditionOperator::Equals, ConditionArg::ArgVariable, "", "arg", ConditionLogical::Negate); // ee_loc has changed

        props.conditions.push_back(cond);
        }

        /////////////////   Initial State   /////////////////

        props.init_state = init_state_vars;

        /////////////////   Propositions   /////////////////

        uint32_t num_propositions = locations.size() * objects.size();
        props.propositions.reserve(num_propositions);
        for (uint32_t i = 0; i < locations.size(); ++i) {
            for (uint32_t j = 0; j < objects.size(); ++j) {
                Condition prop;
                prop.addCondition(ConditionArg::Label, objects[j], ConditionOperator::Equals, ConditionArg::Variable, locations[i]);
                prop.addCondition(ConditionArg::Label, "holding", ConditionOperator::Equals, ConditionArg::Variable, "F");
                prop.setName(objects[j] + "_" + locations[i]);
                props.propositions.push_back(prop);
            }
        }

        // Generate
        return TransitionSystemGenerator::generate(props);
    }
    
    std::string Manipulator::templateToLabel(std::string label_template, uint32_t num) {
        uint32_t i = 0;
        while (i < label_template.size()) {
            if (label_template[i] == ManipulatorModelProperties::s_delimeter) {
                label_template.replace(i, 1, std::to_string(num));
            }
            ++i;
        }
        return label_template;
    }

}
}
