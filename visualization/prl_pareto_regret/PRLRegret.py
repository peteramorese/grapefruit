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
    "grid_on": True,
    "figure_size": (3.9, 3.9),
    "default_color": "rebeccapurple",
    "default_axis_labels": ["Instance", "Regret"],
    "legend_fontsize": 10
}

class PRLRegret:
    def __init__(self):
        self._data_sets = list()

    def deserialize_data_set(self, filepath, label = None, color = visualize_config["default_color"], cumulative_regret = True):
        with open(filepath, "r") as f:
            data = yaml.safe_load(f)
        if "Trials" in data.keys():
            data_set_trials = list()
            for trial in range(data["Trials"]):
                data_set_trials.append(self._create_data_set(data["Trial " + str(trial)], label, color, cumulative_regret))
            self._data_sets.append(self._average_data_across_trials(data_set_trials))
        else:
            self._data_sets.append(self._create_data_set(data, label, color, cumulative_regret))

    def _create_data_set(self, raw_data, label, color, cumulative_regret):
        data_set = {
            "data": [],
            "label": label,
            "color": color,
            "cumulative": cumulative_regret
        }
        regret_values = list()
        for inst in range(0, raw_data["Instances"]):
            instance_key = "Instance " + str(inst)
            try:
                instance_data = raw_data[instance_key]
            except ValueError:
                print(instance_key," not found in animation file")
            regret_values.append(instance_data["Cumulative Regret" if cumulative_regret else "Regret"])
        data_set["data"] = regret_values
        return data_set

    def _average_data_across_trials(self, data_set_trials : list):
        n_instances = len(data_set_trials[0]["data"])
        sum_regrets = [0.0 for _ in data_set_trials[0]["data"]]
        for data_set in data_set_trials:
            data = data_set["data"]
            assert len(data) == n_instances 
            for i in range(n_instances):
                sum_regrets[i] += data[i]
        return {
            "data": [sum_regret / len(data_set_trials) for sum_regret in sum_regrets],
            "label": data_set_trials[0]["label"],
            "color": data_set_trials[0]["color"],
            "cumulative": data_set_trials[0]["cumulative"]
        }
            

    def sketch_data_set(self, data_set : dict, ax = None, start_instance = None, end_instance = None):
        if not ax:
            ax = plt.gca()
        
        if not start_instance:
            start_instance = 0

        if end_instance:
            assert end_instance > start_instance

        assert "data" in data_set.keys()
        assert "label" in data_set.keys()
        assert "color" in data_set.keys()
        assert "cumulative" in data_set.keys()

        if visualize_config["grid_on"]:
            ax.grid()

        ax.plot(data_set["data"][start_instance:end_instance], color=data_set["color"], label=data_set["label"])
        ax.set_xlabel("Instance")
        ax.set_ylabel("Cumulative Regret" if data_set["cumulative"] else "Regret")
        return ax

    def draw(self, block = True, one_plot = True, use_legend = visualize_config["show_legend"], start_instance = None, end_instance = None):

        if one_plot:
            ax = plt.gca()
            for data_set in self._data_sets:
                self.sketch_data_set(data_set, ax)
        else:
            fig, axes = plt.subplots(len(self._data_sets), 1)
            if len(self._data_sets) > 1:
                for i in range(len(axes)):
                    self.sketch_data_set(self._data_sets[i], axes[i])
            else:
                self.sketch_data_set(self._data_sets[0], axes)

        plt.show(block=False)
        if use_legend:
            legend_ax = plt.legend(fontsize=visualize_config["legend_fontsize"], loc="upper left", markerscale=6)
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
    regret_visualizer.draw(one_plot=not args.subplots)
