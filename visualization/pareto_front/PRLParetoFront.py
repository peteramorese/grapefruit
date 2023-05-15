#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import yaml
import argparse
from scipy.stats import multivariate_normal

from DrawParetoFront import visualize_config, ParetoFrontVisualizer2D

class PRLParetoFrontVisualizer(ParetoFrontVisualizer2D):
    def __init__(self):
        super().__init__()

    def deserialize(self, filepath):
        super().deserialize(filepath)
        mu = self._data["PRL Preference Mean"]
        self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mu[0], (1.0 + visualize_config["margin_percent"][0]) * mu[1])

    def sketch_distribution(self, mean, variance, ax = None, fill_contour = True, levels = 2):
        if not ax:
            ax = plt.gca()
        x_ls = np.linspace(self.x_bounds[0], self.x_bounds[1], visualize_config["discretization_N"])
        y_ls = np.linspace(self.y_bounds[0], self.y_bounds[1], visualize_config["discretization_N"])
        x, y = np.meshgrid(x_ls, y_ls)
        grid_point_arr = np.stack([x.flatten(), y.flatten()], axis=1)
        mu = np.array([mean[0], mean[1]])
        sigma = np.array([[variance[0], 0.0], [0.0, variance[1]]])
        #print("sigma: ", sigma)
        #sigma[0][0] = 100*100
        #sigma[1][1] = 16
        #print("sigma: ", sigma)
        pdf_vals = multivariate_normal.pdf(x=grid_point_arr, cov = sigma, mean = mu)
        #pdf_vals = np.zeros(x.shape)
        #for i in range(visualize_config["discretization_N"]):
        #    for j in range(visualize_config["discretization_N"]):
        #        pdf_vals[i, j] = dist.pdf([x[i, j], y[i, j]])
        if fill_contour:
            ax.contourf(x, y, pdf_vals.reshape(x.shape), cmap='GnBu')        
        else:
            ax.contour(x, y, pdf_vals.reshape(x.shape), levels, cmap='GnBu')
        return ax

    def sketch_pareto_front(self, ax = None):
        if not ax:
            ax = plt.gca()
        mean = self._data["PRL Preference Mean"]
        variance = self._data["PRL Preference Variance"]
        ax = self.sketch_distribution(mean, variance, ax)
        ax = super().sketch_pareto_front(ax, connect_points="arrows")
        return ax

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath",default="../../build/bin/pareto_fronts/pf.yaml", help="Specify a filepath to the pareto-front file")
    args = parser.parse_args()

    visualizer = PRLParetoFrontVisualizer()

    visualizer.deserialize(args.filepath)
    visualizer.draw()