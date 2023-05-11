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
    "discretization_N": 100
}

class ParetoFrontVisualizer2D:
    def __init__(self):
        self._data = dict()
        
    def _reset(self):
        self._data.clear()
        
    def serialize(self, filepath):
        self._reset()

        with open(filepath, "r") as f:
            self._data = yaml.safe_load(f)
        self.x_bounds = (0.0, (1.0 + visualize_config["margin_percent"][0]) * max(self._data["Objective 0"]))
        self.y_bounds = (0.0, (1.0 + visualize_config["margin_percent"][1]) * max(self._data["Objective 1"]))

    def _create_figure(self):
        plt.grid()
        pt_color = self._data["Color"] if "Color" in self._data.keys() else visualize_config["default_color"]
        plt.scatter(self._data["Objective 0"], self._data["Objective 1"], color=pt_color, s=visualize_config["dot_size"])
        axis_labels = self._data["Axis Labels"] if "Axis Labels" in self._data.keys() else visualize_config["default_axis_labels"]
        plt.xlabel(axis_labels[0])
        plt.ylabel(axis_labels[1])

    def display(self):
        self._create_figure()
        plt.draw()
        plt.gcf().set_size_inches(visualize_config["figure_size"][0], visualize_config["figure_size"][1])
        plt.show()
    
class PRLParetoFrontVisualizer(ParetoFrontVisualizer2D):
    def __init__(self):
        super().__init__()

    def serialize(self, filepath):
        super().serialize(filepath)
        mu = self._data["PRL Preference Mean"]
        self.x_bounds = (0.0, max(self.x_bounds[1], (1.0 + visualize_config["margin_percent"][0]) * mu[0]))
        self.y_bounds = (0.0, max(self.y_bounds[1], (1.0 + visualize_config["margin_percent"][0]) * mu[1]))

    def __plot_prl_pref(self):
        mean = self._data["PRL Preference Mean"]
        variance = self._data["PRL Preference Variance"]
        print("Plotting PRL Preference Distribution (mean: ", mean, " variance: ", variance, ")")
        x_ls = np.linspace(self.x_bounds[0], self.x_bounds[1], visualize_config["discretization_N"])
        y_ls = np.linspace(self.y_bounds[0], self.y_bounds[1], visualize_config["discretization_N"])
        x, y = np.meshgrid(x_ls, y_ls)
        mu = np.array([mean[0], mean[1]])
        sigma = np.array([[variance[0], 0.0], [0.0, variance[1]]])
        dist = multivariate_normal(cov = sigma, mean = mu)
        pdf_vals = np.zeros(x.shape)
        for i in range(visualize_config["discretization_N"]):
            for j in range(visualize_config["discretization_N"]):
                pdf_vals[i, j] = dist.pdf([x[i, j], y[i, j]])
        plt.contourf(x, y, pdf_vals, cmap='GnBu')        
    
    def _create_figure(self):
        self.__plot_prl_pref()
        super()._create_figure()

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("--read-filepath", default="../../build/bin/pareto_fronts/pf.yaml", help="Specify a filepath to the pareto-front file")
    parser.add_argument("--prl", default=False, action="store_true", help="Interpret as a PRL Pareto front")
    args = parser.parse_args()

    visualizer = ParetoFrontVisualizer2D() if not args.prl else PRLParetoFrontVisualizer()

    visualizer.serialize(args.read_filepath)
    visualizer.display()