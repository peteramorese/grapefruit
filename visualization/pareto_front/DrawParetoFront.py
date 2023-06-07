#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import yaml
import argparse

visualize_config = {
    "line_width": 0.4,
    "line_style": "-",
    "show_title": False,
    "grid_on": True,
    "dot_size": 4,
    "dot_alpha": .7,
    "figure_size": (3.9, 3.9),
    "margin_percent": (.05, .05),
    "default_color": "seagreen",
    "default_prl_pref_color": "firebrick",
    "default_axis_labels": ["Objective 0", "Objective 1"],
    "discretization_N": 1000,
    "arrow_thickness": 0.003,
    "line_width": 1.00,
    "legend_fontsize": 10
}

class ParetoFrontVisualizer2D:
    def __init__(self):
        self._data = dict()
        self._data_sets = list()
        self._data_labels = list()
        self.x_bounds = (0.0, 0.0)
        self.y_bounds = (0.0, 0.0)
        
    def deserialize_minimal_data_file(self, filepath):
        self._reset()

        with open(filepath, "r") as f:
            data = yaml.safe_load(f)
        self._data = data["Data"]
        self._data_sets = data["Data Sets"]
        for data_set in self._data_sets:
            self._push_upper_axis_bounds(max(data_set["Objective 0"]), max(data_set["Objective 1"]))
        self._organize_data_sets()

    def deserialize(self, filepath):
        self._reset()

        with open(filepath, "r") as f:
            self._data = yaml.safe_load(f)
        self._organize_data_sets()

    def load_from_dict(self, data: dict):
        self._reset()
        self._data = data
        self._organize_data_sets()

    def add_data_set(self, data_set, label = None, color = None):
        self._push_upper_axis_bounds(max(data_set["Objective 0"]), max(data_set["Objective 1"]))
        if label:
            data_set["Label"] = label
        if color:
            data_set["Color"] = color
        self._data_sets.append(data_set)

    def write_data_to_minimal_file(self, filepath):
        data = dict()
        data["Data"] = self._data
        data["Data Sets"] = self._data_sets
        with open(filepath, "w") as f:
            yaml.dump(data, f, default_flow_style=False)

    def clear_data_sets(self):
        self._data_sets.clear()
    
    def sketch_pareto_front(self, ax = None, connect_points = None, label = None, marker = ".", start_index = None, xmax = None, ymax = None):
        if not ax:
            ax = plt.gca()
        if not start_index:
            start_index = 0
        ax.grid()
        axis_labels = self._data["Axis Labels"] if "Axis Labels" in self._data.keys() else visualize_config["default_axis_labels"]
        ax.set_xlabel(axis_labels[0])
        ax.set_ylabel(axis_labels[1])
        ax.set_xlim(self.x_bounds if not xmax else (self.x_bounds[0], xmax))
        ax.set_ylim(self.y_bounds if not ymax else (self.y_bounds[0], ymax))
        
        for set in self._data_sets:
            set_color = set["Color"] if "Color" in set.keys() else visualize_config["default_color"]
            set_label = set["Label"] if "Label" in set.keys() else label
            if connect_points is None:
                set["Objective 0"][start_index:]
                ax.scatter(set["Objective 0"][start_index:], set["Objective 1"][start_index:], color=set_color, s=visualize_config["dot_size"], label=set_label, marker=marker, alpha=visualize_config["dot_alpha"])
            elif connect_points == "line":
                #ax.scatter(set["Objective 0"], set["Objective 1"], color=set_color, s=visualize_config["dot_size"], label=label)
                ax.plot(set["Objective 0"][start_index:], set["Objective 1"][start_index:], color=set_color, lw=visualize_config["line_width"], ls=visualize_config["line_style"])
            elif connect_points == "arrows":
                x = np.array(set["Objective 0"][start_index:])
                y = np.array(set["Objective 1"][start_index:])
                ax.quiver(x[:-1], y[:-1], x[1:]-x[:-1], y[1:]-y[:-1], scale_units='xy', angles='xy', scale=1, color=set_color, width=visualize_config["arrow_thickness"], label=set_label)
            else:
                print("Unrecognized point connection type: ", visualize_config["connect_points"])
        return ax
    
    def draw(self, block = True, use_legend = False, show_preference = True):
        plt.figure()
        ax = plt.gca()
        self.sketch_pareto_front(ax)
        plt.show(block=False)
        plt.gcf().set_size_inches(visualize_config["figure_size"][0], visualize_config["figure_size"][1])
        if use_legend:
            plt.legend(fontsize=15, loc="upper left")
        if block:
            self._block_for_input()

    def _reset(self):
        self._data.clear()
        self._data_sets.clear()
        
    def _organize_data_sets(self):
        for key, value in self._data.items():
            if key.startswith("DataSet"):
                self._data_sets.append(value)
                self._push_upper_axis_bounds(max(value["Objective 0"]), max(value["Objective 1"]))

    def _push_upper_axis_bounds(self, x_bound, y_bound):
        adjusted_x_bound = (1.0 + visualize_config["margin_percent"][0]) * x_bound
        adjusted_y_bound = (1.0 + visualize_config["margin_percent"][0]) * y_bound
        if adjusted_x_bound > self.x_bounds[1]:
            self.x_bounds = (self.x_bounds[0], adjusted_x_bound)
        if adjusted_y_bound > self.y_bounds[1]:
            self.y_bounds = (self.y_bounds[0], adjusted_y_bound)

    def _block_for_input(self):
        input("Press key to close")



if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath",default="../../build/bin/pareto_fronts/pf.yaml", help="Specify a filepath to the data file")
    args = parser.parse_args()

    visualizer = ParetoFrontVisualizer2D() 

    visualizer.deserialize(args.filepath)
    visualizer.draw()