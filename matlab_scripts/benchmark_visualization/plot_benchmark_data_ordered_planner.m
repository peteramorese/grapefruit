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
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_pf.txt"], "num_dfas", ["naive_iterations", "smart_iterations"], ["No Heuristic: ", "Heuristic: "]);
title("Naive vs. Smart Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")

figure()
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_pf.txt"], "num_dfas", ["naive_time", "smart_time"], ["No Heuristic: ", "Heuristic: "]);
title("Naive vs. Smart Search Time")
ylabel("Iterations")
xlabel("Number of DFAs")

% scatterPlotNamedTuples("../../benchmark/benchmark_data/bm_ordered_planner_flex.txt", "single_point_search_iterations")

binPlotNamedTuples(["../../benchmark/benchmark_data/bm_ordered_planner_flex.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_flex.txt"], "single_point_search_iterations", 16)
legend(["No heuristic", "Heuristic"])
title("Mean Cumulative Iterations per Pareto Point")
ylabel("Iterations")
xlabel("\mu")

binPlotNamedTuples(["../../benchmark/benchmark_data/bm_ordered_planner_flex.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_flex.txt"], "single_point_search", 16)
legend(["No heuristic", "Heuristic"])
title("Mean Cumulative Search Time per Pareto Point")
ylabel("Search Time (ms)")
xlabel("\mu")


