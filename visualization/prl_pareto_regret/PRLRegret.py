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
    "legend_fontsize": 10,
    "legend_loc": "upper left"
}

benchmark_fields = [
    ("Regret", "linear"),
    ("Cumulative Regret", "linear"),
    ("Coverage Bias", "linear"),
    ("Containment Bias", "linear"),
    ("Total Bias", "linear"),
    #("Cumulative Bias", "linear"),
    ("Worst outlier Bias", "linear"),
]

class DataSet:
    def __init__(self, filepath, label, color):
        print("Loading data file...")
        with open(filepath, "r") as f:
            raw_data = yaml.safe_load(f)
        print("Done.")
        self.n_trials = raw_data["Trials"]
        assert self.n_trials > 0
        self.n_instances = raw_data["Trial 0"]["Instances"]
        self.label = label
        self.color = color
        
        self.data = np.zeros((self.n_instances, self.n_trials, len(benchmark_fields)))
        self.avg_data = None 
        for trial in range(self.n_trials):
            print("Parsing trial ", trial + 1, "/", self.n_trials)
            trial_key = "Trial " + str(trial)
            try:
                trial_data = raw_data[trial_key]
            except ValueError:
                print(trial_key," not found in data file")
            
            for instance in range(self.n_instances):
                instance_key = "Instance " + str(instance)
                try:
                    instance_data = trial_data[instance_key]
                except ValueError:
                    print(instance_key," not found in data file")

                for idx, benchmark_field in enumerate(benchmark_fields):
                    self.data[instance][trial][idx] = instance_data[benchmark_field[0]]
        
    def get_stat_data(self, benchmark_field : str):
        if self.avg_data is None:
            self.avg_data = np.mean(self.data, axis=1)
            self.std_data = np.std(self.data, axis=1)
        for idx, field in enumerate(benchmark_fields):
            if field[0] == benchmark_field:
                return self.avg_data[:, idx], self.std_data[:, idx] if self.n_trials > 1 else None

class PRLRegret:
    def __init__(self, data_sets : list):
        self._data_sets = data_sets
            
    def sketch_all_data_sets(self, benchmark_field, ax = None, start_instance = None, end_instance = None, fancy_region_width = 20):
        fancy_idx = end_instance
        if not ax:
            ax = plt.gca()
        for data_set in self._data_sets:
            std_start_instance = None
            std_end_instance = None
            if fancy_region_width is not None:
                if fancy_idx is None:
                    fancy_idx = data_set.n_instances
                std_end_instance = fancy_idx 
                std_start_instance = fancy_idx - fancy_region_width
                fancy_idx -= fancy_region_width
            PRLRegret.sketch_data_set(data_set, benchmark_field, ax, start_instance, end_instance, std_start_instance, std_end_instance)
        return ax

    @staticmethod
    def sketch_data_set(data_set, benchmark_field, ax = None, start_instance = None, end_instance = None, std_start_instance = None, std_end_instance = None):
        if not ax:
            ax = plt.gca()
        
        mean, std = data_set.get_stat_data(benchmark_field[0])

        if not start_instance:
            start_instance = 0

        if end_instance:
            assert end_instance > start_instance
        else:
            end_instance = len(mean)

        if not std_start_instance:
            std_start_instance = 0

        if std_end_instance:
            assert std_end_instance > std_start_instance
        else:
            std_end_instance = len(mean)

        if visualize_config["grid_on"]:
            ax.grid()

        ax.plot(mean[start_instance:end_instance], color=data_set.color, label=data_set.label)
        std_upper = [mu + 1.0 * sigma for mu, sigma in zip(mean[start_instance:end_instance], std[start_instance:end_instance])]
        std_lower = [mu - 1.0 * sigma for mu, sigma in zip(mean[start_instance:end_instance], std[start_instance:end_instance])]
        ax.fill_between(range(start_instance, end_instance), std_upper[start_instance:end_instance], std_lower[start_instance:end_instance], color = data_set.color, alpha=0.1)
        print("range: ",range(std_start_instance, std_end_instance) )
        print("upper: ", std_upper[std_start_instance:std_end_instance])
        ax.fill_between(range(std_start_instance, std_end_instance), std_upper[std_start_instance:std_end_instance], std_lower[std_start_instance:std_end_instance], color = data_set.color, alpha=0.3)
            #ax.plot([mean + 1.0 * var for mean, var in zip(data_set["mean"][start_instance:end_instance], data_set["var"][start_instance:end_instance])], color = data_set["color"], ls=':')
            #ax.plot([mean - 1.0 * var for mean, var in zip(data_set["mean"][start_instance:end_instance], data_set["var"][start_instance:end_instance])], color = data_set["color"], ls=':')
        ax.set_xlabel("Instance")
        ax.set_title(benchmark_field[0])
        ax.set_yscale(benchmark_field[1])
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
    parser.add_argument("--filepaths", nargs='+', required=True, help="Specify multiple filepaths to the each data file")
    parser.add_argument("--labels", nargs='+', help="Custom label each data set in '--filepaths'")
    parser.add_argument("--colors", nargs='+', help="Custom color of each data set in '--filepaths'")
    parser.add_argument("--start-instance", default=0, type=int, help="Starting instance to display data from")
    parser.add_argument("--end-instance", default=None, type=int, help="Ending instance to display data from")
    parser.add_argument("--highlight-width", default=20, type=int, help="Number of instances to highlight variance across")
    #parser.add_argument("--hist", default=False, action="store_true", help="Display a histogram of the cumulative regret")
    args = parser.parse_args()

    data_sets = list()
    for i in range(len(args.filepaths)):
        if args.labels:
            label = args.labels[i]
        else:
            label = os.path.splitext(args.filepaths[i])[0]
        if args.colors:
            color = args.colors[i]
        else:
            color = np.random.rand(3)
        data_sets.append(DataSet(args.filepaths[i], label, color))

    regret_visualizer = PRLRegret(data_sets)

    for benchmark_field in benchmark_fields:
        fig, ax = plt.subplots(figsize=visualize_config["figure_size"])
        ax = regret_visualizer.sketch_all_data_sets(benchmark_field, ax, fancy_region_width=args.highlight_width)
        ax.legend(loc=visualize_config["legend_loc"])

    plt.show(block=False)
    input("Press key to close")

    #if args.filepath:
    #    regret_visualizer.deserialize_data_set(args.filepath, args.label if args.label else None, cumulative_regret=args.cumulative)
    #elif args.filepaths:
    #    ind = 0
    #    for f in args.filepaths:
    #        label = args.labels[ind] if args.labels else None
    #        color = args.colors[ind] if args.colors else None
    #        regret_visualizer.deserialize_data_set(f, label, color=color, cumulative_regret=args.cumulative)
    #        ind += 1
    #else:
    #    assert False

    #if args.hist:
    #    regret_visualizer.draw_histograms()
    #else:
    #    regret_visualizer.draw(one_plot=not args.subplots)
