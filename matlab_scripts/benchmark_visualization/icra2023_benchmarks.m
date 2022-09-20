clear; close all; clc;

% Search computation for single plan
figure()
subplot(1,2,1)
sgtitle("Search-Time Benchmarks")
units = plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_single_ndfas.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_single_ndfas.txt"], "num_dfas", ["search"], ["No Heuristic: ", "Heuristic: "]);
set(gca, "YScale", 'log')
title("Single Plan")
ylabel("Time ("+units+")")
xlabel("Number of DFAs")


% subplot(1,2,2)
% plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_single_ndfas.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_single_ndfas.txt"], "num_dfas", ["sq_iterations"], ["No Heuristic: ", "Heuristic: "]);
% set(gca, "YScale", 'log')
% title("Iterations")
% ylabel("Iterations")
% xlabel("Number of DFAs")

% Search computation for whole PF

subplot(1,2,2)

units = plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic.txt"], "num_dfas", ["search"], ["No Heuristic: ", "Heuristic: "]);
set(gca, "YScale", 'log')
title("Pareto Front Computation")
ylabel("Time ("+units+")")
xlabel("Number of DFAs")

% subplot(1,2,2)
% plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic.txt"], "num_dfas", ["iterations"], ["No Heuristic: ", "Heuristic: "]);
% set(gca, "YScale", 'log')
% title("Iterations")
% ylabel("Iterations")
% xlabel("Number of DFAs")

% Naive vs. smart PF computation
figure()
subplot(1,2,1)
sgtitle("Naive/Smart PF Computation")
plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_pf.txt"], "num_dfas", ["naive_time", "smart_time"], ["No Heuristic: ", "Heuristic: "]);
%set(gca, "YScale", 'log')
title("Search Time")
ylabel("Time (ms)")
xlabel("Number of DFAs")

subplot(1,2,2)
plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_pf.txt"], "num_dfas", ["naive_iterations", "smart_iterations"], ["No Heuristic: ", "Heuristic: "]);
%set(gca, "YScale", 'log')
title("Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")

% Ours vs. BOA PF computation
figure()
subplot(1,2,1)
sgtitle("BOA Comparison")
plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_boa.txt","./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_boa.txt"], "num_dfas", ["search", "boa_search"], ["No Heuristic: ", "Heuristic: "]);
%set(gca, "YScale", 'log')
title("Search Time")
ylabel("Time (ms)")
xlabel("Number of DFAs")



subplot(1,2,2)
sgtitle("BOA Comparison")
plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_boa.txt","./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_boa.txt"], "num_dfas", ["iterations", "boa_iterations"], ["No Heuristic: ", "Heuristic: "]);
%set(gca, "YScale", 'log')
title("Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")



% Naive computation w and w/o heuristic:
figure()
subplot(1,2,1)
sgtitle("Naive vs. Smart w/(o) Heuristic")
plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_pf.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_pf.txt"], "num_dfas", ["naive_time", "smart_time"], ["No Heuristic: ", "Heuristic: "]);
%set(gca, "YScale", 'log')
title("Search Time")
ylabel("Time (ms)")
xlabel("Number of DFAs")

subplot(1,2,2)
plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_pf.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_pf.txt"], "num_dfas", ["naive_iterations","smart_iterations"], ["No Heuristic: ", "Heuristic: "]);
%set(gca, "YScale", 'log')
title("Iterations")
ylabel("Iterations")
xlabel("Number of DFAs")


% % Model size:
% figure()
% subplot(1,2,1)
% units = plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_model_size.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_model_size.txt"], "model_size", ["search"], ["No Heuristic: ", "Heuristic: "]);
% title("Total Search Time vs Model Size")
% ylabel("Time ("+units+")")
% xlabel("Number of Model States")
% 
% subplot(1,2,2)
% units = plotAttrVsTimeData(["./icra2023_scaled_benchmark_data/bm_ordered_planner_model_size.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_model_size.txt"], "model_size", ["iterations"], ["No Heuristic: ", "Heuristic: "]);
% title("Total Iterations vs Model Size")
% ylabel("Iterations")
% xlabel("Number of Model States")

% % Computation per pareto point:
% figure()
% subplot(1,2,1)
% binPlotNamedTuples(["./icra2023_scaled_benchmark_data/bm_ordered_planner_flex.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_flex.txt"], "single_point_search_iterations", 16)
% legend(["No heuristic", "Heuristic"])
% title("Mean Cumulative Iterations per Pareto Point")
% ylabel("Iterations")
% xlabel("\mu")
% 
% subplot(1,2,2)
% binPlotNamedTuples(["./icra2023_scaled_benchmark_data/bm_ordered_planner_flex.txt", "./icra2023_scaled_benchmark_data/bm_ordered_planner_heuristic_flex.txt"], "single_point_search", 16)
% legend(["No heuristic", "Heuristic"])
% title("Mean Cumulative Search Time per Pareto Point")
% ylabel("Search Time (ms)")
% xlabel("\mu")

% scatterPlotNamedTuples("./icra2023_scaled_benchmark_data/bm_ordered_planner_flex.txt", "single_point_search_iterations")
