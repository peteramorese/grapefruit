clear; close all; clc;

% Plot 'first_search' and 'total_search' time vs number of formulas

figure()
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic.txt"], "num_dfas", ["search"], ["No Heuristic: ", "Heuristic: "]);
title("Total Search Time")
ylabel("Time ("+units+")")
xlabel("Number of DFAs")

figure()
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic.txt"], "num_dfas", ["iterations"], ["No Heuristic: ", "Heuristic: "]);
title("Total Search Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")

figure()
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_pf.txt"], "num_dfas", ["manual_iterations", "auto_iterations"], ["No Heuristic: ", "Heuristic: "]);
title("Manual vs. Single PF Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")

scatterPlotNamedTuples("../../benchmark/benchmark_data/bm_ordered_planner_flex.txt", "single_point_search_iterations")

% % Plot 'first_search' and 'total_search' time vs flexibility
% % time_data_lbls = ["first_search"];
% figure()
% units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner_flex.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "flexibility", time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
% title("Total Search Time")
% ylabel("Time ("+units+")")
% xlabel("Flexibility")
% 
% % Plot 'iterations' vs flexibility
% figure()
% time_data_lbls = ["iterations"];
% units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner_flex.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "flexibility", time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
% title("Total Search Iterations")
% ylabel("Iterations")
% xlabel("Flexibility")

% % Plot 'iterations' vs flexibility for just A*
% figure()
% time_data_lbls = ["iterations"];
% units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "flexibility", time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
% title("Total Search Iterations")
% ylabel("Iterations")
% xlabel("Flexibility")
