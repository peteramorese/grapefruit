#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import yaml
import argparse

visualize_config = {
    "line_width": 0.8,
    "line_style": "-",
    "show_title": False,
    "grid_on": True,
    "dot_size": 50,
    "figure_size": (3.9, 3.9)
}

class ParetoFrontVisualizer2D:
    def __init__(self):
        self.__data = dict()
        
    def __reset(self):
        self.__data.clear()
        
    def serialize(self, filepath):
        self.__reset()

        with open(filepath, "r") as f:
            self.__data = yaml.safe_load(f)

        self.__create_figure()

    def __create_figure(self):
        plt.grid()
        plt.plot(self.__data["Objective Cost 0"], self.__data["Objective Cost 1"])
        plt.scatter(self.__data["Objective Cost 0"], self.__data["Objective Cost 1"], color="seagreen", s=visualize_config["dot_size"])
        axis_labels = self.__data["Axis Labels"]
        plt.xlabel(axis_labels[0])
        plt.ylabel(axis_labels[1])

    def display(self):
        plt.draw()
        plt.gcf().set_size_inches(visualize_config["figure_size"][0], visualize_config["figure_size"][1])
        plt.show()

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("--read-filepath", default="../../build/bin/pareto_fronts/pf.yaml", help="Specify a filepath to the pareto-front file")
    args = parser.parse_args()

    visualizer = ParetoFrontVisualizer2D()

    visualizer.serialize(args.read_filepath)
    visualizer.display()