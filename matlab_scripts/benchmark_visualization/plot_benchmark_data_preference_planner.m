clear; close all; clc;

% figure()
% plotNumDFAsVsSearchTime(["../../benchmark/benchmark_data/bm_preference_planner.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic.txt"], "Total Search Speed", ["No Heuristic", "Heuristic"]);
box_time_data_lbls = ["first_search", "total_search"];
%box_time_data_lbls = ["total_search"];
figure()
%plotFlexibilityVsSearchTime(["../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "Total Search Speed", ["No Heuristic", "Heuristic"]);
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic.txt"], "num_dfas", box_time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
title("Total Search Speed")
ylabel("Time ("+units+")")
xlabel("Number of DFAs")


figure()
%plotFlexibilityVsSearchTime(["../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "Total Search Speed", ["No Heuristic", "Heuristic"]);
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner_flex.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "flexibility", box_time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
title("Total Search Speed")
ylabel("Time ("+units+")")
xlabel("Flexibility")
