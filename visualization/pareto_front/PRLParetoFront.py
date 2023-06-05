#!/usr/bin/env python3

import os
import sys
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse
import numpy as np
import yaml
import argparse
from scipy.stats import multivariate_normal

sys.path.append("..")
from pareto_front.DrawParetoFront import visualize_config, ParetoFrontVisualizer2D

class PRLParetoFrontVisualizer(ParetoFrontVisualizer2D):
    def __init__(self):
        super().__init__()

    def deserialize_data_set(self, filepath, start_instance = 0, label = ""):
        with open(filepath, "r") as f:
            data = yaml.safe_load(f)
        samples = {
            "Objective 0": [],
            "Objective 1": [],
        }
        for inst in range(start_instance, data["Instances"]):
            instance_key = "Instance " + str(inst)
            try:
                instance_data = data[instance_key]
            except ValueError:
                print(instance_key," not found in animation file")
            samples["Objective 0"].append(instance_data["Sample"][0])
            samples["Objective 1"].append(instance_data["Sample"][1])
        self.add_data_set(samples, label)
        self._data["PRL Preference Mean"] = data["PRL Preference Mean"]
        self._data["PRL Preference Covariance"] = data["PRL Preference Covariance"]
        self._organize_data_sets()
        #super().deserialize(filepath)
        #mu = self._data["PRL Preference Mean"]
        #self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mu[0], (1.0 + visualize_config["margin_percent"][0]) * mu[1])

    def load_from_dict(self, data: dict):
        super().load_from_dict(data)
        #mu = self._data["PRL Preference Mean"]
        #self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mu[0], (1.0 + visualize_config["margin_percent"][0]) * mu[1])

    def sketch_distribution(self, mean, covariance, ax = None, fill_contour = True, levels = 2, label = None, marker = "o", cmap = "GnBu", marker_color = "teal"):
        if not ax:
            ax = plt.gca()

        self._push_upper_axis_bounds((1.0 + visualize_config["margin_percent"][0]) * mean[0], (1.0 + visualize_config["margin_percent"][0]) * mean[1])

        ax.set_xlim(self.x_bounds)
        ax.set_ylim(self.y_bounds)
        mu = np.array([mean[0], mean[1]])
        sigma = np.array([[covariance[0], covariance[1]], [covariance[1], covariance[2]]])
        if fill_contour:
            x_ls = np.linspace(self.x_bounds[0], self.x_bounds[1], visualize_config["discretization_N"])
            y_ls = np.linspace(self.y_bounds[0], self.y_bounds[1], visualize_config["discretization_N"])
            x, y = np.meshgrid(x_ls, y_ls)
            grid_point_arr = np.stack([x.flatten(), y.flatten()], axis=1)
            pdf_vals = multivariate_normal.pdf(x=grid_point_arr, cov = sigma, mean = mu)
            ax.contourf(x, y, pdf_vals.reshape(x.shape), levels=levels, cmap=cmap)
        else:
            """
            Source: https://stackoverflow.com/questions/12301071/multidimensional-confidence-intervals/12321306#12321306
            """
            vals, vecs = np.linalg.eigh(sigma)
            order = vals.argsort()[::-1]
            vals = vals[order]
            vecs = vecs[:,order]
            theta = np.degrees(np.arctan2(*vecs[:,0][::-1]))
            cmap_vals = np.linspace(0.6, 0.8, levels)
            cmap_object = matplotlib.cm.get_cmap(cmap)
            for i in range(levels):
                w, h = 2 * (i + 1) * np.sqrt(vals)
                color = cmap_object(cmap_vals[i])
                ellipse = Ellipse(xy = mu, width=w, height=h, angle=theta, lw=visualize_config["line_width"], fill=False, color=color)
                ax.add_artist(ellipse)
        

        if label:
            ax.scatter(x=mean[0], y=mean[1], s=30, c=marker_color, marker=marker, label=label)
        else:
            ax.scatter(x=mean[0], y=mean[1], s=30, c=marker_color, marker=marker)
        return ax

    def sketch_preference_distribution(self, ax = None, fill_contour = True, levels = 20, label = "Preference"):
        mean = self._data["PRL Preference Mean"]
        covariance = self._data["PRL Preference Covariance"]
        ax = self.sketch_distribution(mean, covariance, ax, fill_contour=True, label=label, levels = 20, marker = "D")

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
    parser.add_argument("-f", "--filepath", help="Specify a filepath to the pareto-front file", required=True)
    parser.add_argument("--filepaths", nargs='+', help="Specify multiple filepaths to the each data file")
    parser.add_argument("--hide-preference", default=False, action="store_true", help="Hide the preference distribution")
    parser.add_argument("-l","--label", default=None, help="Label the data set")
    parser.add_argument("--labels", nargs='+', help="Label each data set in '--filepaths'")
    args = parser.parse_args()

    visualizer = PRLParetoFrontVisualizer()

    if args.filepaths:
        for filepath in args.filepaths:
            visualizer.deserialize_data_set(filepath)
    else:
        if args.label:
            visualizer.deserialize_data_set(args.filepath, label = args.label)
        else:
            visualizer.deserialize_data_set(args.filepath)
    visualizer.draw(use_legend=True)