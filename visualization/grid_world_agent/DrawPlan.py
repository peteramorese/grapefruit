#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import yaml
import argparse

visualize_config = {
    "path_line_width": 3.0,
    "path_line_style": '-',
    "grid_line_width": 1.0,
    "grid_line_style": "--",
    "text_font_size": 20.0,
    "show_text": True,
    "text_offset": (.1, .1)
}

class GridWorldAgentVisualizer:

    def __init__(self, config_filepath):
        self.__plan_properties = dict()

        with open(config_filepath, "r") as f:
            self.__config = yaml.safe_load(f)

        self.__grid_size = (self.__config["Grid X"], self.__config["Grid Y"])
        self.__environment = None
        if "N Regions" in self.__config:
            self.__environment = list()
            for i in range(self.__config["N Regions"]):
                region = dict()
                region["label"] = self.__config["Region Labels"][i]
                region["lower_left_x"] = self.__config["Lower Left Cells X"][i]
                region["lower_left_y"] = self.__config["Lower Left Cells Y"][i]
                region["upper_right_x"] = self.__config["Upper Right Cells X"][i]
                region["upper_right_y"] = self.__config["Upper Right Cells Y"][i]
                region["color"] = self.__config["Region Colors"][i]
                self.__environment.append(region)
    
    def __reset(self):
        self.__plan_properties.clear()
        
    def serialize(self, filepath):
        self.__reset()

        with open(filepath, "r") as f:
            self.__plan_properties = yaml.safe_load(f)

        print(self.__plan_properties)
        self.__create_figure()

    @staticmethod
    def __state_str_to_coord(state_str):
        split_str = state_str.split(", ")
        assert len(split_str) == 2
        return (int(split_str[0].replace('x', '')), int(split_str[1].replace('y', '')))
    
    def __draw_regions(self, ax, show_unique_regions_only = True):
        unique_regions = set()
        for region in self.__environment:
            width = region["upper_right_x"] - region["lower_left_x"] + 1
            height = region["upper_right_y"] - region["lower_left_y"] + 1
            rectangle = matplotlib.patches.Rectangle((region["lower_left_x"] - 0.5, region["lower_left_y"] - 0.5), width, height, color=region["color"])
            ax.add_patch(rectangle)
            if visualize_config["show_text"] and region["label"] not in unique_regions:
                plt.text(region["lower_left_x"] - 0.5 + visualize_config["text_offset"][0], region["lower_left_y"] - 0.5 + visualize_config["text_offset"][1], region["label"], fontsize=visualize_config["text_font_size"])
                unique_regions.add(region["label"])


    def __create_figure(self, show_endpoints = True):
        x_seq = list()
        y_seq = list()
        for s in self.__plan_properties["State Sequence"]:
            s_coord = self.__state_str_to_coord(s)
            x_seq.append(s_coord[0])
            y_seq.append(s_coord[1])

        plt.figure()

        if "Title" in self.__plan_properties:
            plt.title(self.__plan_properties["Title"])

        plt.axis([-0.5, self.__grid_size[0] - 0.5, -0.5, self.__grid_size[1] - 0.5])
        plt.xticks(range(self.__grid_size[0]))
        plt.yticks(range(self.__grid_size[1]))
        ax = plt.gca()
        ax.set_xticks(np.arange(0.5, self.__grid_size[0] + 0.5, 1), minor=True)
        ax.set_yticks(np.arange(0.5, self.__grid_size[1] + 0.5, 1), minor=True)
        plt.grid(which="minor", ls=visualize_config["grid_line_style"], lw=visualize_config["grid_line_width"])

        if self.__environment:
            self.__draw_regions(ax)

        plt.plot(x_seq, y_seq, ls=visualize_config["path_line_style"], lw=visualize_config["path_line_width"])

        if show_endpoints:
            plt.scatter(x_seq[0], y_seq[0], c="r")
            plt.scatter(x_seq[-1], y_seq[-1], c="g")
            if visualize_config["show_text"]:
                plt.text(x_seq[0] + visualize_config["text_offset"][0], y_seq[0] + visualize_config["text_offset"][1], "I", fontsize=visualize_config["text_font_size"])
                plt.text(x_seq[-1] + visualize_config["text_offset"][0], y_seq[-1] + visualize_config["text_offset"][1], "F", fontsize=visualize_config["text_font_size"])

    def display(self):
        plt.show()

    def display_environment(self):
        plt.figure()

        plt.axis([-0.5, self.__grid_size[0] - 0.5, -0.5, self.__grid_size[1] - 0.5])
        plt.xticks(range(self.__grid_size[0]))
        plt.yticks(range(self.__grid_size[1]))
        ax = plt.gca()
        ax.set_xticks(np.arange(0.5, self.__grid_size[0] + 0.5, 1), minor=True)
        ax.set_yticks(np.arange(0.5, self.__grid_size[1] + 0.5, 1), minor=True)
        plt.grid(which="minor", ls=visualize_config["grid_line_style"], lw=visualize_config["grid_line_width"])

        if self.__environment:
            self.__draw_regions(ax)

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("--read-directory", default="../../build/bin/grid_world_plans", help="Specify a directory to read plan files from")
    parser.add_argument("--config-filepath", default="../../build/bin/configs/grid_world_config.yaml", help="Specify a grid world config file")
    parser.add_argument("--display-environment-only", default=False, action="store_true", help="Show the environment only")
    args = parser.parse_args()

    visualizer = GridWorldAgentVisualizer(args.config_filepath)

    if not args.display_environment_only:
        for file in os.scandir(args.read_directory):
            visualizer.serialize(file)
            visualizer.display()
    else:
        visualizer.display_environment()
        visualizer.display()
        

