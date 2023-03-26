#pragma once

#include<string>
#include<vector>
#include<filesystem>
#include<iostream>
#include<fstream>

#include "orderedPlanner.h"

namespace MatlabDemoFiles {

const std::string PATH_TO_PLAN_FILES = "/matlab_scripts/preference_planning_demos/plan_files/";
const std::string DEFAULT_PREF_PLAN_FILE_NAME = "plan.txt";
const std::string PATH_TO_PLOT_FILES = "/matlab_scripts/preference_planning_demos/plot_files/";
const std::string DEFAULT_PARETO_PLOT_FILE_NAME = "pareto_front.txt";

std::filesystem::path getPath(const std::string* filepath, const std::string& default_path);

class GridRobot {
    private:
        static void appendLOI(const std::vector<std::string>* loi, std::ofstream& open_plan_file);
    public:
        static void writeSinglePlan(const std::vector<std::string>& action_sequence, const std::string* filepath, bool overwrite = true);
        static void writeSinglePlan(const std::vector<std::string>& action_sequence, const std::filesystem::path& filepath, bool overwrite = true);
        static void writeSinglePlan(const std::vector<std::string>& action_sequence, std::ofstream& open_plan_file);
        static void writeFlexibilityPlanList(const OrderedPlanner::Result& result, const std::string* filepath, const std::vector<std::string>* xtra_info = nullptr);
        static void writeFlexibilityPlanList(const std::vector<std::pair<std::vector<std::string>, float>>& plan_and_flexibility_list, const std::string* filepath, const std::vector<std::string>* xtra_info = nullptr);
};

namespace ParetoFront {
    void writeFlexibilityParetoFront(const OrderedPlanner::Result& result, const std::string* filepath);
};

}