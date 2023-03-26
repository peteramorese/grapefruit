clear; clc; close all;

%%%%%%%%%%%%%%%
grid_size = 10;
animate = false;
display_prio = false;
use_subplot = false;
max_subplot_length = 3;
loi_font_size = 9;
show_xyticks = false;
%%%%%%%%%%%%%%%

preferenceGridRobotDemo(grid_size, animate, display_prio, use_subplot, max_subplot_length, loi_font_size, show_xyticks)

% Filename, plot_line = true, plot_scatter = true
plotFile("plot_files/pareto_front.txt", true, true)
grid on