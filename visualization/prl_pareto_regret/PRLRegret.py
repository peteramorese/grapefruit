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
    "show_legend": False,
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
        data_set = {
            "data": [],
            "label": label,
            "color": color
        }
        regret_values = list()
        for inst in range(0, data["Instances"]):
            instance_key = "Instance " + str(inst)
            try:
                instance_data = data[instance_key]
            except ValueError:
                print(instance_key," not found in animation file")
            regret_values.append(instance_data["Cumulative Regret" if cumulative_regret else "Regret"])
        data_set["data"] = regret_values
        self._data_sets.append(data_set)

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

        if visualize_config["grid_on"]:
            ax.grid()

        ax.plot(data_set["data"][start_instance:end_instance], color=data_set["color"], label=data_set["label"])
        return ax

    def draw(self, block = True, use_legend = visualize_config["show_legend"], start_instance = None, end_instance = None):

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
    parser.add_argument("-f", "--filepath",default="animation.yaml", help="Data file")
    parser.add_argument("--filepaths", nargs='+', help="Specify multiple filepaths to the each data file")
    parser.add_argument("-l","--label", default=None, help="Label the data set")
    parser.add_argument("--cumulative", default=True, action="store_false", help="Specify if cumulative regret should be plotted")
    parser.add_argument("--labels", nargs='+', help="Label each data set in '--filepaths'")
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
    regret_visualizer.draw()
