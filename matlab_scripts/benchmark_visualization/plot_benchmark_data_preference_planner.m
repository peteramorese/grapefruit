clear; close all; clc;

% Plot 'first_search' and 'total_search' time vs number of formulas
time_data_lbls = ["first_search", "total_search"];
box_time_data_lbls = ["total_search"];
figure()
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic.txt"], "num_dfas", time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
title("Total Search Time")
ylabel("Time ("+units+")")
xlabel("Number of DFAs")

% Plot 'first_search' and 'total_search' time vs flexibility
figure()
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner_flex.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "flexibility", time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
title("Total Search Time")
ylabel("Time ("+units+")")
xlabel("Flexibility")

% Plot 'iterations' vs flexibility
figure()
time_data_lbls = ["iterations"];
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_preference_planner_flex.txt", "../../benchmark/benchmark_data/bm_preference_planner_heuristic_flex.txt"], "flexibility", time_data_lbls, ["No Heuristic: ", "Heuristic: "]);
title("Total Search Iterations")
ylabel("Iterations")
xlabel("Flexibility")
