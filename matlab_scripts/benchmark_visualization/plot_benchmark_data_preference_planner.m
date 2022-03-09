clear; close all; clc;

figure()
plotNumDFAsVsSearchTime(["../../benchmark/benchmark_data/bm_preference_planner.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic.txt"], "Total Search Speed", ["No Heuristic", "Heuristic"]);

figure()
%plotFlexibilityVsSearchTime(["../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "Total Search Speed", ["No Heuristic", "Heuristic"]);
plotFlexibilityVsSearchTime(["../../benchmark/benchmark_data/bm_preference_planner_flex.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "Total Search Speed", ["No Heuristic", "Heuristic"]);
% subplot(2,1,2)
% plotNumDFAsVsSearchTime(, "With Heuristic (A*)");