#pragma once

#include<string>
#include<vector>
#include<filesystem>
#include<iostream>
#include<fstream>

#include "orderedPlanner.h"

namespace MatlabDemoFiles {

const std::string PATH_TO_PREF_PLAN_FILES = "/matlab_scripts/preference_planning_demos/plan_files/";
const std::string DEFAULT_PLAN_FILE_NAME = "plan.txt";

class GridRobot {
    private:
        static std::filesystem::path getPath(const std::string* filepath, const std::string& default_path);
    public:
        static void writeSinglePlan(const std::vector<std::string>& action_sequence, const std::string* filepath, bool overwrite = true);
        static void writeSinglePlan(const std::vector<std::string>& action_sequence, const std::filesystem::path& filepath, bool overwrite = true);
        static void writeSinglePlan(const std::vector<std::string>& action_sequence, std::ofstream& open_plan_file);
        static void writeFlexibilityPlanList(const OrderedPlanner::Result& result, const std::string* filepath);
        static void writeFlexibilityPlanList(const std::vector<std::pair<std::vector<std::string>, float>>& plan_and_flexibility_list, const std::string* filepath);
};

namespace ParetoFront {
    static void writeFlexibilityParetoFront(const OrderedPlanner::Result& result, const std::string* filepath);
};

}