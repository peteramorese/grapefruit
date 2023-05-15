#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import yaml
import argparse
from scipy.stats import multivariate_normal

from .DrawParetoFront import visualize_config, ParetoFrontVisualizer2D

class PRLParetoFrontVisualizer(ParetoFrontVisualizer2D):
    def __init__(self):
        super().__init__()

    def deserialize(self, filepath):
        super().deserialize(filepath)
        #mu = self._data["PRL Preference Mean"]
        #self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mu[0], (1.0 + visualize_config["margin_percent"][0]) * mu[1])

    def load_from_dict(self, data: dict):
        super().load_from_dict(data)
        #mu = self._data["PRL Preference Mean"]
        #self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mu[0], (1.0 + visualize_config["margin_percent"][0]) * mu[1])

    def sketch_distribution(self, mean, variance, ax = None, fill_contour = True, levels = 20, label = None, marker = "o"):
        if not ax:
            ax = plt.gca()

        self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mean[0], (1.0 + visualize_config["margin_percent"][0]) * mean[1])

        ax.set_xlim(self.x_bounds)
        ax.set_ylim(self.y_bounds)
        x_ls = np.linspace(self.x_bounds[0], self.x_bounds[1], visualize_config["discretization_N"])
        y_ls = np.linspace(self.y_bounds[0], self.y_bounds[1], visualize_config["discretization_N"])
        x, y = np.meshgrid(x_ls, y_ls)
        grid_point_arr = np.stack([x.flatten(), y.flatten()], axis=1)
        mu = np.array([mean[0], mean[1]])
        sigma = np.array([[variance[0], 0.0], [0.0, variance[1]]])
        pdf_vals = multivariate_normal.pdf(x=grid_point_arr, cov = sigma, mean = mu)
        if fill_contour:
            ax.contourf(x, y, pdf_vals.reshape(x.shape), levels=levels, cmap='GnBu')
        else:
            ax.contour(x, y, pdf_vals.reshape(x.shape), levels=levels, cmap='GnBu')
        if label:
            ax.scatter(x=mean[0], y=mean[1], s=30, c='teal', marker=marker, label=label)
        else:
            ax.scatter(x=mean[0], y=mean[1], s=30, c='teal', marker=marker)
        return ax

    def sketch_preference_distribution(self, ax = None, fill_contour = True, levels = 20, label = "Preference"):
        mean = self._data["PRL Preference Mean"]
        variance = self._data["PRL Preference Variance"]
        ax = self.sketch_distribution(mean, variance, ax, fill_contour=True, label=label, marker = "D")

    def draw(self, block = True, use_legend = False):
        plt.figure()
        ax = plt.gca()
        self.sketch_preference_distribution(ax)
        self.sketch_pareto_front(ax)
        plt.show(block=False)
        plt.gcf().set_size_inches(visualize_config["figure_size"][0], visualize_config["figure_size"][1])
        if use_legend:
            plt.legend(fontsize=15, loc="upper left")
        if block:
            self._block_for_input()

    #def sketch_pareto_front(self, ax = None, connect_points = "arrows", label = None):
    #    if not ax:
    #        ax = plt.gca()
    #    self.sketch_preference_distribution()
    #    ax = super().sketch_pareto_front(ax, connect_points=connect_points, label=label)
    #    return ax

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath",default="../../build/bin/pareto_fronts/pf.yaml", help="Specify a filepath to the pareto-front file")
    args = parser.parse_args()

    visualizer = PRLParetoFrontVisualizer()

    visualizer.deserialize(args.filepath)
    visualizer.draw(use_legend=True)