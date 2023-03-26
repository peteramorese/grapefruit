clear; close all; clc;

% Search computation for whole PF
figure()
subplot(1,2,1)
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic.txt"], "num_dfas", ["search"], ["No Heuristic: ", "Heuristic: "]);
title("Total Search Time")
ylabel("Time ("+units+")")
xlabel("Number of DFAs")

subplot(1,2,2)
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic.txt"], "num_dfas", ["iterations"], ["No Heuristic: ", "Heuristic: "]);
title("Total Search Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")

% Naive vs. smart PF computation
figure()
subplot(1,2,1)
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_pf.txt"], "num_dfas", ["naive_iterations", "smart_iterations"], ["No Heuristic: ", "Heuristic: "]);
title("Naive vs. Smart Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")

subplot(1,2,2)
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_pf.txt"], "num_dfas", ["naive_time", "smart_time"], ["No Heuristic: ", "Heuristic: "]);
title("Naive vs. Smart Search Time")
ylabel("Iterations")
xlabel("Number of DFAs")

% Naive computation w and w/o heuristic:
figure()
subplot(1,2,1)
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_pf.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_pf.txt"], "num_dfas", ["naive_iterations","smart_iterations"], ["No Heuristic: ", "Heuristic: "]);
title("Naive vs. Smart w/(o) Heuristic Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")

subplot(1,2,2)
plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_pf.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_pf.txt"], "num_dfas", ["naive_time", "smart_time"], ["No Heuristic: ", "Heuristic: "]);
title("Naive vs. Smart w/(o) Heuristic Iterations")
ylabel("Search Time (ms)")
xlabel("Number of DFAs")

% Model size:
figure()
subplot(1,2,1)
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_model_size.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_model_size.txt"], "model_size", ["search"], ["No Heuristic: ", "Heuristic: "]);
title("Total Search Time vs Model Size")
ylabel("Time ("+units+")")
xlabel("Number of Model States")

subplot(1,2,2)
units = plotAttrVsTimeData(["../../benchmark/benchmark_data/bm_ordered_planner_model_size.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_model_size.txt"], "model_size", ["iterations"], ["No Heuristic: ", "Heuristic: "]);
title("Total Iterations vs Model Size")
ylabel("Iterations")
xlabel("Number of Model States")

% Computation per pareto point:
figure()
subplot(1,2,1)
binPlotNamedTuples(["../../benchmark/benchmark_data/bm_ordered_planner_flex.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_flex.txt"], "single_point_search_iterations", 16)
legend(["No heuristic", "Heuristic"])
title("Mean Cumulative Iterations per Pareto Point")
ylabel("Iterations")
xlabel("\mu")

subplot(1,2,2)
binPlotNamedTuples(["../../benchmark/benchmark_data/bm_ordered_planner_flex.txt", "../../benchmark/benchmark_data/bm_ordered_planner_heuristic_flex.txt"], "single_point_search", 16)
legend(["No heuristic", "Heuristic"])
title("Mean Cumulative Search Time per Pareto Point")
ylabel("Search Time (ms)")
xlabel("\mu")

% scatterPlotNamedTuples("../../benchmark/benchmark_data/bm_ordered_planner_flex.txt", "single_point_search_iterations")
