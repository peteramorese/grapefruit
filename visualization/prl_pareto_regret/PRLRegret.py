#!/usr/bin/env python3

import os
import sys
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import yaml
import argparse

# Internal Modules
sys.path.append("..")
from grid_world_agent.DrawPlan import GridWorldAgentVisualizer
from pareto_front.PRLParetoFront import PRLParetoFrontVisualizer

visualize_config = {
    "line_width": 0.4,
    "line_style": "-",
    "show_title": False,
    "show_legend": True,
    "grid_on": False,
    "figure_size": (3.9, 3.9),
    "default_color": "rebeccapurple",
    "default_axis_labels": ["Instance", "Regret"],
    "legend_fontsize": 10
}

class PRLRegret:
    def __init__(self):
        self._data_sets = list()
        self._data_sets_avg = list()
        self._n_trials = None

    def deserialize_data_set(self, filepath, label = None, color = visualize_config["default_color"], cumulative_regret = True):
        print("Loading data file...")
        with open(filepath, "r") as f:
            data = yaml.safe_load(f)
        print("Done.")
        self._data_sets.append({
            "data": data,
            "label": label,
            "color": color,
            "cumulative": cumulative_regret
            })
        if "Trials" in data.keys():
            trials_data = list()
            self._n_trials = data["Trials"]
            for trial in range(self._n_trials):
                print("Parsing trial ", trial + 1, "/", self._n_trials)
                trials_data.append(self._create_data_set(data["Trial " + str(trial)], label, color, cumulative_regret))
            self._data_sets_avg.append(self._average_data_across_trials(trials_data))
        else:
            self._data_sets_avg.append(self._create_data_set(data, label, color, cumulative_regret))

    def _create_data_set(self, raw_data, label, color, cumulative_regret):
        data_set = {
            "data": np.zeros((5, raw_data["Instances"])),
            "label": label,
            "color": color,
            "cumulative": cumulative_regret
        }
        for inst in range(0, raw_data["Instances"]):
            instance_key = "Instance " + str(inst)
            try:
                instance_data = raw_data[instance_key]
            except ValueError:
                print(instance_key," not found in data file")
            data_set["data"][0][inst] = instance_data["Cumulative Regret" if cumulative_regret else "Regret"]
            data_set["data"][1][inst] = instance_data["Coverage Bias"]
            data_set["data"][2][inst] = instance_data["Containment Bias"]
            data_set["data"][3][inst] = instance_data["Total Bias"]
            data_set["data"][4][inst] = instance_data["Worst outlier Bias"]
        return data_set

    def _average_data_across_trials(self, trials_data : list):
        n_instances = len(trials_data[0]["data"])
        instance_data = [list() for _ in trials_data[0]["data"]]
        #instance_data = np.zeros((5, len(trials_data[0]["data"])))
        for data_set in trials_data:
            data = data_set["data"]
            assert len(data) == n_instances 
            for i in range(n_instances):
                instance_data[i].append(data[:, i])
        return {
            "mean": [np.mean(data) for data in instance_data],
            "std": [np.std(data) for data in instance_data],
            "label": trials_data[0]["label"],
            "color": trials_data[0]["color"],
            "cumulative": trials_data[0]["cumulative"]
        }
            
    def sketch_data_set(self, data_set : dict, ax = None, start_instance = None, end_instance = None):
        if not ax:
            ax = plt.gca()
        
        if not start_instance:
            start_instance = 0

        if end_instance:
            assert end_instance > start_instance

        assert "mean" in data_set.keys()
        assert "label" in data_set.keys()
        assert "color" in data_set.keys()
        assert "cumulative" in data_set.keys()

        if visualize_config["grid_on"]:
            ax.grid()

        ax.plot(data_set["mean"][start_instance:end_instance], color=data_set["color"], label=data_set["label"])
        if "std" in data_set.keys():
            upper = [mean + 1.0 * var for mean, var in zip(data_set["mean"][start_instance:end_instance], data_set["std"][start_instance:end_instance])]
            lower = [mean - 1.0 * var for mean, var in zip(data_set["mean"][start_instance:end_instance], data_set["std"][start_instance:end_instance])]
            ax.fill_between(range(len(upper)), upper, lower, color = data_set["color"], alpha=0.2)
            #ax.plot([mean + 1.0 * var for mean, var in zip(data_set["mean"][start_instance:end_instance], data_set["var"][start_instance:end_instance])], color = data_set["color"], ls=':')
            #ax.plot([mean - 1.0 * var for mean, var in zip(data_set["mean"][start_instance:end_instance], data_set["var"][start_instance:end_instance])], color = data_set["color"], ls=':')
        ax.set_xlabel("Instance")
        ax.set_ylabel("Cumulative Regret" if data_set["cumulative"] else "Regret")
        return ax
    
    def sketch_data_histograms(self, instance = None, n_bins = 20):
        fig, axs = plt.subplots(len(self._data_sets), 1)
        x_max = 0
        for data_set, ax in zip(self._data_sets, axs):
            data = data_set["data"]
            assert "Trials" in data.keys()
            assert self._n_trials
            if instance is None:
                instance = data["Trial 0"]["Instances"] - 1
            instance_cumulative_regrets = list()
            for trial in range(self._n_trials):
                instance_data = data["Trial " + str(trial)]["Instance " + str(instance)]
                regret = instance_data["Cumulative Regret" if data_set["cumulative"] else "Regret"]
                if regret > x_max:
                    x_max = regret
                instance_cumulative_regrets.append(regret)
                #trials_data.append(self._create_data_set(data["Trial " + str(trial)], label, color, cumulative_regret))
            ax.hist(instance_cumulative_regrets, bins=n_bins, color=data_set["color"])
            ax.set_title(data_set["label"])
        for ax in axs:
            ax.set_xlim(0.0, x_max)

    def draw(self, block = True, one_plot = True, use_legend = visualize_config["show_legend"], start_instance = None, end_instance = None):

        if one_plot:
            ax = plt.gca()
            for data_set in self._data_sets_avg:
                self.sketch_data_set(data_set, ax)
        else:
            fig, axes = plt.subplots(len(self._data_sets_avg), 1)
            if len(self._data_sets_avg) > 1:
                for i in range(len(axes)):
                    self.sketch_data_set(self._data_sets_avg[i], axes[i])
            else:
                self.sketch_data_set(self._data_sets_avg[0], axes)

        plt.show(block=False)
        if use_legend:
            legend_ax = plt.legend(fontsize=visualize_config["legend_fontsize"], loc="upper left", markerscale=6)
        if block:
            input("Press key to close")

    def draw_histograms(self, block = True, use_legend = visualize_config["show_legend"], instance = None, n_bins = 20):
        self.sketch_data_histograms(instance, n_bins)
        plt.show(block=False)
        if block:
            input("Press key to close")


if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath",default=None, help="Data file")
    parser.add_argument("--filepaths", nargs='+', help="Specify multiple filepaths to the each data file")
    parser.add_argument("-l","--label", default=None, help="Label the data set")
    parser.add_argument("--cumulative", default=False, action="store_true", help="Specify if cumulative regret should be plotted")
    parser.add_argument("--subplots", default=False, action="store_true", help="Draw a subplot for each file")
    parser.add_argument("--labels", nargs='+', help="Label each data set in '--filepaths'")
    parser.add_argument("--colors", nargs='+', help="Specify the color of each data set in '--filepaths'")
    parser.add_argument("--config-filepath", default="../../build/bin/configs/grid_world_config.yaml", help="Specify a grid world config file")
    parser.add_argument("--start-instance", default=0, type=int, help="Animation starting instance")
    parser.add_argument("--end-instance", default=None, type=int, help="Animation ending instance")
    parser.add_argument("--hist", default=False, action="store_true", help="Display a histogram of the cumulative regret")
    args = parser.parse_args()

    regret_visualizer = PRLRegret()

    if args.filepath:
        regret_visualizer.deserialize_data_set(args.filepath, args.label if args.label else None, cumulative_regret=args.cumulative)
    elif args.filepaths:
        ind = 0
        for f in args.filepaths:
            label = args.labels[ind] if args.labels else None
            color = args.colors[ind] if args.colors else None
            regret_visualizer.deserialize_data_set(f, label, color=color, cumulative_regret=args.cumulative)
            ind += 1
    else:
        assert False

    if args.hist:
        regret_visualizer.draw_histograms()
    else:
        regret_visualizer.draw(one_plot=not args.subplots)
