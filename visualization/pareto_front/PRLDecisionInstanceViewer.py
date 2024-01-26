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
from matplotlib.colors import LinearSegmentedColormap

visualize_config = {
    "line_style": "-",
    "show_title": False,
    "grid_on": True,
    "dot_size": 20,
    "dot_alpha": .7,
    "figure_size": (3.9, 3.9),
    "margin_percent": (.15, .15),
    "default_color": "rebeccapurple",
    "default_prl_pref_color": "firebrick",
    #"default_axis_labels": ["Objective 0", "Objective 1"],
    "default_axis_labels": ["Time", "Radiation"],
    "discretization_N": 1000,
    "arrow_thickness": 0.003,
    "line_width": 1.00,
    "legend_fontsize": 10,
    "discretization_N": 1000
}

class Instance:
    def __init__(self):
        self.true_distributions = list()
        self.estimate_distributions = list()
        self.x_max = 0.0
        self.y_max = 0.0
        self.chosen_plan = None

    def add_true_dist(self, true_distribution_data):
        mean = true_distribution_data["True Plan Mean"]
        covariance = true_distribution_data["True Plan Covariance"]
        self._push_bounds(mean)
        self.true_distributions.append(self._make_distribution_parameters(mean, covariance, None))
            
    def add_estimate_dist(self, estimate_distribution_data):
        mean = estimate_distribution_data["Plan Mean"]
        covariance = estimate_distribution_data["Plan Covariance"]
        ucb = estimate_distribution_data["Plan Pareto UCB"]
        self._push_bounds(mean)
        self.estimate_distributions.append(self._make_distribution_parameters(mean, covariance, ucb))

    def _make_distribution_parameters(self, mean_data : list, covariance_data : list, ucb_data = None):
        mean = np.array([mean_data[0], mean_data[1]])
        cov = np.array([[covariance_data[0], covariance_data[1]], [covariance_data[1], covariance_data[2]]])
        if ucb_data is not None:
            ucb = np.array([ucb_data[0], ucb_data[1]])
        else:
            ucb = None
        return (mean, cov, ucb)

    def _push_bounds(self, point):
        if point[0] > self.x_max:
            self.x_max = point[0]
        if point[1] > self.y_max:
            self.y_max = point[1]
             

def create_threshold_colormap(threshold=0.01):
    # Define the color dictionary with white at the beginning and green at the end
    colors = {'red': [(0.0, 1.0, 1.0), (threshold, 1.0, 1.0), (1.0, 0.2, 0.0)],
              'green': [(0.0, 1.0, 1.0), (threshold, 1.0, 1.0), (1.0, 0.7, 0.0)],
              'blue': [(0.0, 1.0, 1.0), (threshold, 1.0, 1.0), (1.0, 0.3, 0.0)]}

    # Create the colormap
    threshold_cmap = LinearSegmentedColormap('threshold_colormap', colors, N=256)
    
    return threshold_cmap


class PRLDecisionInstanceViewer:

    def __init__(self):
        self.x_max = 0.0
        self.y_max = 0.0
        self.bias_values = None

    def deserialize_data_set(self, filepath, trial = 0):
        with open(filepath, "r") as f:
            data = yaml.safe_load(f)

        trial_key = "Trial " + str(trial)
        trial_data = data[trial_key]
        
        mean = trial_data["PRL Preference Mean"]
        covariance = trial_data["PRL Preference Covariance"]
        self._pev_mu = np.array([mean[0], mean[1]])
        self._pev_sigma = np.array([[covariance[0], covariance[1]], [covariance[1], covariance[2]]])

        self.bias_values = np.zeros(trial_data["Instances"])
        self.cumulative_bias_values = np.zeros(trial_data["Instances"])
        self.n_instances = trial_data["Instances"]
        self.instance_distributions = list()
        #instance_benchmarks = list
        
        for inst in range(0, trial_data["Instances"]):
            instance_key = "Instance " + str(inst)
            try:
                instance_data = trial_data[instance_key]
            except ValueError:
                print(instance_key," not found in animation file")

            #print("instance data: ", instance_data)
            instance = Instance()
            i = 0
            while True:
                try:
                    candidate_plan_data = instance_data["Candidate Plan " + str(i)]
                    instance.add_estimate_dist(candidate_plan_data)
                    if "Candidate Plan " + str(i) == instance_data["Chosen Plan"]:
                        instance.chosen_plan = i
                    i += 1
                except KeyError:
                    break

            i = 0
            while True:
                try:
                    true_plan_data = instance_data["True Plan " + str(i)]
                    instance.add_true_dist(true_plan_data)
                    i += 1
                except KeyError:
                    break
            
            self._push_bounds((instance.x_max, instance.y_max))
            self.instance_distributions.append(instance)

            # Add bias values
            self.bias_values[inst] = instance_data["Total Bias"]
            self.cumulative_bias_values[inst] = instance_data["Cumulative Bias"]
            #self.bias_values[inst] = instance_data["Total Bias"]
        
        assert len(self.instance_distributions) == trial_data["Instances"]

    def _push_bounds(self, point):
        if point[0] > self.x_max:
            self.x_max = point[0]
        if point[1] > self.y_max:
            self.y_max = point[1]
             
    def size_ax(self, ax, instance = None):
        if instance is None:
            ax.set_xlim((0.0, (1.0 + visualize_config["margin_percent"][0]) * self.x_max))
            ax.set_ylim((0.0, (1.0 + visualize_config["margin_percent"][1]) * self.y_max))
        else:
            instance = self.instance_distributions[instance]
            sz = max(instance.x_max, instance.y_max)
            ax.set_xlim((0.0, (1.0 + visualize_config["margin_percent"][0]) * sz))
            ax.set_ylim((0.0, (1.0 + visualize_config["margin_percent"][1]) * sz))
                
    def sketch_distribution(self, mean, covariance, ax = None, ucb = None, levels = 2, label = None, marker = "o", cmap = "GnBu", marker_color = "teal", zorder = None, lw = visualize_config["line_width"], fill_contour = False):
        if not ax:
            ax = plt.gca()

        if fill_contour:
            x_ls = np.linspace(0, ax.get_xlim()[1], visualize_config["discretization_N"])
            y_ls = np.linspace(0, ax.get_ylim()[1], visualize_config["discretization_N"])
            x, y = np.meshgrid(x_ls, y_ls)
            grid_point_arr = np.stack([x.flatten(), y.flatten()], axis=1)
            pdf_vals = multivariate_normal.pdf(x=grid_point_arr, cov = covariance, mean = mean)
            ax.contourf(x, y, pdf_vals.reshape(x.shape), levels=levels, cmap=create_threshold_colormap(), zorder=zorder)
        else:
            """
            Source: https://stackoverflow.com/questions/12301071/multidimensional-confidence-intervals/12321306#12321306
            """
            vals, vecs = np.linalg.eigh(covariance)
            order = vals.argsort()[::-1]
            vals = vals[order]
            vecs = vecs[:,order]
            theta = np.degrees(np.arctan2(*vecs[:,0][::-1]))
            cmap_vals = np.linspace(0.6, 0.8, levels)
            cmap_object = matplotlib.colormaps.get_cmap(cmap)
            for i in range(levels):
                w, h = 2 * (i + 1) * np.sqrt(vals)
                color = cmap_object(cmap_vals[i])
                ellipse = Ellipse(xy = mean, width=w, height=h, angle=theta, lw=lw, fill=False, color=color, zorder=zorder)
                ax.add_artist(ellipse)

            if label:
                ax.scatter(x=mean[0], y=mean[1], s=1, c=marker_color, marker=marker, label=label)
            else:
                ax.scatter(x=mean[0], y=mean[1], s=1, c=marker_color, marker=marker)
            
            if ucb is not None:
                ax.scatter(ucb[0], ucb[1], s=1, c=marker_color, marker=marker)
                ax.plot([ucb[0], mean[0]], [ucb[1], mean[1]], c=marker_color, ls="--", lw=lw)
        if label:
            ax.scatter(x=mean[0], y=mean[1], s=1, c=marker_color, marker=marker, label=label)
        else:
            ax.scatter(x=mean[0], y=mean[1], s=1, c=marker_color, marker=marker)
        return ax

    def sketch_instance(self, instance : int, ax = None):
        assert instance < self.n_instances

        if not ax:
            ax = plt.gca()

        self.size_ax(ax, instance)

        instance_data = self.instance_distributions[instance]

        # Preference distribution

        #for dist in instance_data.estimate_distributions:
        for i in range(len(instance_data.estimate_distributions)):
            dist = instance_data.estimate_distributions[i]
            cmap = "winter" if i != instance_data.chosen_plan else "Reds"
            self.sketch_distribution(dist[0], dist[1], ax, ucb=dist[2], cmap=cmap)
            if i == instance_data.chosen_plan:
                chosen_plan_mean = dist[0]

        for dist in instance_data.true_distributions:
            self.sketch_distribution(dist[0], dist[1], ax, cmap="Wistia")

        selection_line_pts = np.stack((self._pev_mu, chosen_plan_mean))
        ax.plot(selection_line_pts[:,0], selection_line_pts[:,1], color="Red", ls=":", lw=visualize_config["line_width"])
        self.sketch_distribution(self._pev_mu, self._pev_sigma, ax, cmap="pink", fill_contour=True, levels=20)

    def sketch_bias_plot(self, instance : int, ax = None):
        assert instance < self.n_instances

        if not ax:
            ax = plt.gca()

        ax.plot(self.bias_values[:instance], ls="-", color="blue")
        ax.set_title("Bias: " + str(self.bias_values[instance]))

    def sketch_cumulative_bias_plot(self, instance : int, ax = None):
        assert instance < self.n_instances

        if not ax:
            ax = plt.gca()

        ax.plot(self.cumulative_bias_values[:instance], ls="-", color="blue")
        ax.set_title("Cumulative Bias: " + str(self.cumulative_bias_values[instance]))

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath", help="Specify a filepath to the data file")
    parser.add_argument("-t", "--trial", default=0, type=int, help="Specific trial to view")
    args = parser.parse_args()

    viewer = PRLDecisionInstanceViewer()

    viewer.deserialize_data_set(args.filepath, trial=args.trial)
    #fig, axes = plt.subplots(nrows=1, ncols=3)
    #fig.suptitle(args.filepath)
    #ax_1, ax_2, ax_3 = axes
    fig, ax_1 = plt.subplots(nrows=1, ncols=1)

    viewer.size_ax(ax_1)

    i = 0
    while i < viewer.n_instances:
        print("Displaying instance: ", i)
        viewer.sketch_instance(i, ax_1)
        #viewer.sketch_bias_plot(i, ax_2)
        #viewer.sketch_cumulative_bias_plot(i, ax_3)
        #viewer.size_ax(ax_1, i)
        plt.pause(0.1)
        #plt.show(block=False)
        inp = input("[enter for next] [q to quit]: ")
        i += 1
        if inp and inp == "q":
            break
        elif inp:
            i = int(inp)
        ax_1.clear()
        #ax_2.clear()
        #ax_3.clear()

    plt.close()



