#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import yaml
import argparse
from scipy.stats import multivariate_normal

visualize_config = {
    "line_width": 0.8,
    "line_style": "-",
    "show_title": False,
    "grid_on": True,
    "dot_size": 8,
    "figure_size": (3.9, 3.9),
    "margin_percent": (.2, .2),
    "default_color": "seagreen",
    "default_prl_pref_color": "firebrick",
    "default_axis_labels": ["Objective 0", "Objective 1"],
    "discretization_N": 100,
    "arrow_thickness": 0.003
}

class ParetoFrontVisualizer2D:
    def __init__(self):
        self._data = dict()
        self.x_bounds = (0.0, 0.0)
        self.y_bounds = (0.0, 0.0)
        
    def _reset(self):
        self._data.clear()
        
    def serialize(self, filepath):
        self._reset()

        with open(filepath, "r") as f:
            self._data = yaml.safe_load(f)
        self._data_sets = list()
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

    def _create_figure(self, connect_points = None):
        plt.grid()
        axis_labels = self._data["Axis Labels"] if "Axis Labels" in self._data.keys() else visualize_config["default_axis_labels"]
        plt.xlabel(axis_labels[0])
        plt.ylabel(axis_labels[1])
        plt.xlim(self.x_bounds)
        plt.ylim(self.y_bounds)

        for set in self._data_sets:
            pt_color = set["Color"] if "Color" in set.keys() else visualize_config["default_color"]
            if connect_points is None:
                plt.scatter(set["Objective 0"], set["Objective 1"], color=pt_color, s=visualize_config["dot_size"])
            elif connect_points is "line":
                plt.plot(set["Objective 0"], set["Objective 1"], color=pt_color, s=visualize_config["dot_size"])
            elif connect_points is "arrows":
                x = np.array(set["Objective 0"])
                y = np.array(set["Objective 1"])
                plt.quiver(x[:-1], y[:-1], x[1:]-x[:-1], y[1:]-y[:-1], scale_units='xy', angles='xy', scale=1, color=pt_color, width=visualize_config["arrow_thickness"])
            else:
                print("Unrecognized point connection type: ", connect_points)

    def display(self):
        self._create_figure()
        plt.draw()
        plt.gcf().set_size_inches(visualize_config["figure_size"][0], visualize_config["figure_size"][1])
        plt.show(block=False)
        input("Press key to close")
    
class PRLParetoFrontVisualizer(ParetoFrontVisualizer2D):
    def __init__(self):
        super().__init__()

    def serialize(self, filepath):
        super().serialize(filepath)
        mu = self._data["PRL Preference Mean"]
        self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mu[0], (1.0 + visualize_config["margin_percent"][0]) * mu[1])

    def __plot_prl_pref(self):
        mean = self._data["PRL Preference Mean"]
        variance = self._data["PRL Preference Variance"]
        print("Plotting PRL Preference Distribution (mean: ", mean, " variance: ", variance, ")")
        x_ls = np.linspace(self.x_bounds[0], self.x_bounds[1], visualize_config["discretization_N"])
        y_ls = np.linspace(self.y_bounds[0], self.y_bounds[1], visualize_config["discretization_N"])
        x, y = np.meshgrid(x_ls, y_ls)
        mu = np.array([mean[0], mean[1]])
        sigma = np.array([[variance[0], 0.0], [0.0, variance[1]]])
        #print("sigma: ", sigma)
        #sigma[0][0] = 100*100
        #sigma[1][1] = 16
        #print("sigma: ", sigma)
        dist = multivariate_normal(cov = sigma, mean = mu)
        pdf_vals = np.zeros(x.shape)
        for i in range(visualize_config["discretization_N"]):
            for j in range(visualize_config["discretization_N"]):
                pdf_vals[i, j] = dist.pdf([x[i, j], y[i, j]])
        plt.contourf(x, y, pdf_vals, cmap='GnBu')        
    
    def _create_figure(self):
        self.__plot_prl_pref()
        super()._create_figure(connect_points="arrows")

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath",default="../../build/bin/pareto_fronts/pf.yaml", help="Specify a filepath to the pareto-front file")
    parser.add_argument("--prl", default=False, action="store_true", help="Interpret as a PRL Pareto front")
    args = parser.parse_args()

    visualizer = ParetoFrontVisualizer2D() if not args.prl else PRLParetoFrontVisualizer()

    visualizer.serialize(args.filepath)
    visualizer.display()