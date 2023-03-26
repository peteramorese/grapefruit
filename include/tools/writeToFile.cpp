#include "writeToFile.h"


std::filesystem::path MatlabDemoFiles::getPath(const std::string* filepath, const std::string& default_path) {
    if (filepath) {
        return *filepath;
    } else {
        return std::filesystem::current_path().parent_path().parent_path().concat(default_path);
    }

}

void MatlabDemoFiles::GridRobot::writeSinglePlan(const std::vector<std::string>& action_sequence, const std::string* filepath, bool overwrite) {
    std::filesystem::path p = getPath(filepath, PATH_TO_PLAN_FILES);
	std::ofstream plan_file;
	if (overwrite) {
        plan_file.open(p);
        plan_file<<"Type: single_plan\n";
    } else {
        plan_file.open(p, std::ios_base::app);
    }
	for (const auto& action : action_sequence) {
			plan_file<<"  "<<action<<'\n';
	}
	plan_file.close();
}

void MatlabDemoFiles::GridRobot::writeSinglePlan(const std::vector<std::string>& action_sequence, const std::filesystem::path& filepath, bool overwrite) {
	std::ofstream plan_file;
	if (overwrite) {
        plan_file.open(filepath);
        plan_file<<"Type: single_plan\n";
    } else {
        plan_file.open(filepath, std::ios_base::app);
    }
	for (const auto action : action_sequence) {
			plan_file<<"  "<<action<<'\n';
	}
	plan_file.close();
}

void MatlabDemoFiles::GridRobot::writeSinglePlan(const std::vector<std::string>& action_sequence, std::ofstream& open_plan_file) {
	for (const auto action : action_sequence) {
			open_plan_file<<"  "<<action<<'\n';
	}
}

void MatlabDemoFiles::GridRobot::appendLOI(const std::vector<std::string>* loi, std::ofstream& open_plan_file) {
    // Append locations of interest:
	for (const auto l : *loi) {
			open_plan_file<<"LOI: "<<l<<'\n';
	}
}

void MatlabDemoFiles::GridRobot::writeFlexibilityPlanList(const OrderedPlanner::Result& result, const std::string* filepath, const std::vector<std::string>* loi) {
    std::filesystem::path p = getPath(filepath, PATH_TO_PLAN_FILES + DEFAULT_PREF_PLAN_FILE_NAME);
    auto pf = result.getParetoFront();
    std::ofstream plan_file;
    plan_file.open(p);
    plan_file<<"Type: flexibility_plan_list\n";
    if (loi) {
        appendLOI(loi, plan_file);
    }
    for (const auto& pt : *pf) {
        plan_file<<"Flexibility: "<<pt.mu<<'\n';
        writeSinglePlan(pt.plan.action_sequence, plan_file);
    }
}

void MatlabDemoFiles::GridRobot::writeFlexibilityPlanList(const std::vector<std::pair<std::vector<std::string>, float>>& plan_and_flexibility_list, const std::string* filepath, const std::vector<std::string>* loi) {
    std::filesystem::path p = getPath(filepath, PATH_TO_PLAN_FILES + DEFAULT_PREF_PLAN_FILE_NAME);
    std::ofstream plan_file;
    plan_file.open(p);
    plan_file<<"Type: flexibility_plan_list\n";
    if (loi) {
        appendLOI(loi, plan_file);
    }
    for (const auto& pt : plan_and_flexibility_list) {
        plan_file<<"Flexibility: "<<pt.second<<'\n';
        writeSinglePlan(pt.first, plan_file);
    }
}

void MatlabDemoFiles::ParetoFront::writeFlexibilityParetoFront(const OrderedPlanner::Result& result, const std::string* filepath) {
    std::filesystem::path p = getPath(filepath, PATH_TO_PLOT_FILES + DEFAULT_PARETO_PLOT_FILE_NAME);
    std::ofstream plan_file;
    auto pf = result.getParetoFront();
    plan_file.open(p);
    plan_file<<"Title: Flexibility Pareto Front\n";
    plan_file<<"xLabel: Flexiblity (mu)\n";
    plan_file<<"yLabel: Cost\n";
    for (const auto& pt : *pf) {
        plan_file<<pt.mu<<", "<<pt.path_length<<'\n';
    }
}