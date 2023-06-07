#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, FancyBboxPatch
import numpy as np
import yaml
import argparse

visualize_config = {
    "path_line_width": 2.0,
    "path_line_style": '-',
    "path_line_color": "blue",
    "grid_line_width": 0.1,
    "grid_line_style": "-",
    "text_font_size": 15.0,
    "show_text": True,
    "show_ticks": True,
    "show_title": False,
    "show_endpoints": True,
    "text_offset": (.1, .5),
    "traj_offset_magnitude": 0.2,
    "arrow_color": "path_line_color",
    "arrow_scale": 20.0,
    "arrow_head_width": 2,
    "cells_per_arrow": 2,
    "show_directions": True,
    "figure_size": (4, 4),
    "obstacle_color": [0.2, 0.2, 0.2],
    "region_scale": 0.03,
    "default_region_alpha": 0.7,
    "fancy_corner": .6
}

class GridWorldAgentVisualizer:

    def __init__(self, config_filepath):
        self._data = dict()

        with open(config_filepath, "r") as f:
            self.__config = yaml.safe_load(f)

        self.__grid_size = (self.__config["Grid X"], self.__config["Grid Y"])
        self.__environment = None
        self.__obstacles = None
        if "Region Labels" in self.__config:
            self.__environment = list()
            region_labels = self.__config["Region Labels"]
            print("region labels: ", region_labels)
            for region_label in region_labels:
                region_data = self.__config[region_label]
                region = dict()
                print("found region: ", region_label)
                region["label"] = region_data["Proposition"] if "Proposition" in region_data.keys() else region_label
                bounds = region_data["Bounds"]
                assert len(bounds) == 4
                region["lower_left_x"] = bounds[0]
                region["upper_right_x"] = bounds[1]
                region["lower_left_y"] = bounds[2]
                region["upper_right_y"] = bounds[3]
                region["color"] = region_data["Color"]
                region["fancy"] = region_data["Fancy"] if "Fancy" in region_data.keys() else True
                region["alpha"] = region_data["Alpha"] if "Alpha" in region_data.keys() else visualize_config["default_region_alpha"]
                self.__environment.append(region)
        if "Obstacles" in self.__config:
            self.__obstacles = list()
            for obs_label, data in self.__config["Obstacles"].items():
                obstacle = dict()
                obstacle["label"] = obs_label
                bounds = data["Bounds"]
                assert len(bounds) == 4
                obstacle["lower_left_x"] = bounds[0]
                obstacle["upper_right_x"] = bounds[1]
                obstacle["lower_left_y"] = bounds[2]
                obstacle["upper_right_y"] = bounds[3]
                self.__obstacles.append(obstacle)


    def reset(self):
        self._data.clear()
    
    def deserialize(self, filepath):
        self.reset()

        with open(filepath, "r") as f:
            self._data = yaml.safe_load(f)

    def load_from_dict(self, data: dict):
        self.reset()
        self._data = data

    def sketch_plan(self, ax = None, show_directions = visualize_config["show_directions"], color = visualize_config["path_line_color"], arrow_color = visualize_config["arrow_color"], label="title", ls=visualize_config["path_line_style"], zorder=None):
        if not ax:
            ax = self.sketch_environment()
        x_seq = list()
        y_seq = list()
        u_seq = list()
        v_seq = list()
        x_seq.append(self.__state_str_to_coord(self._data["State Sequence"][0])[0])
        y_seq.append(self.__state_str_to_coord(self._data["State Sequence"][0])[1])
        prev_offset = np.array([0.0, 0.0])
        for i in range(1, len(self._data["State Sequence"])):
            s_prev_coord = self.__state_str_to_coord(self._data["State Sequence"][i-1])
            s_coord = self.__state_str_to_coord(self._data["State Sequence"][i])
            direction = np.array([s_coord[0] - s_prev_coord[0], s_coord[1] - s_prev_coord[1], 0.0])
            u_seq.append(direction[0])
            v_seq.append(direction[1])
            offset_v = np.cross(direction, np.array([0.0, 0.0, 1.0]))
            offset = np.array([visualize_config["traj_offset_magnitude"] * offset_v[0], visualize_config["traj_offset_magnitude"] * offset_v[1]])
            x_seq[-1] += offset[0]
            y_seq[-1] += offset[1]
            if sum(abs(offset -prev_offset)) > 0.000000001:
                x_seq[-1] += prev_offset[0]
                y_seq[-1] += prev_offset[1]
                x_seq.append(s_coord[0])
                y_seq.append(s_coord[1])
            else:
                x_seq.append(s_coord[0])
                y_seq.append(s_coord[1])
            prev_offset = offset

        if "Title" in self._data:
            if visualize_config["show_title"]:
                ax.set_title(self._data["Title"])

        if label:
            if label == "title":
                label = self._data["Title"]
            ax.plot(x_seq, y_seq, lw=visualize_config["path_line_width"], color=color, label=label, ls=ls, zorder=zorder)
        else:
            ax.plot(x_seq, y_seq, lw=visualize_config["path_line_width"], color=color, ls=ls, zorder=zorder)
            

        if show_directions:
            cells_per_arrow = visualize_config["cells_per_arrow"]
            if arrow_color == "path_line_color":
                ax.quiver(x_seq[0:-1:cells_per_arrow], y_seq[0:-1:cells_per_arrow], u_seq[0::cells_per_arrow], v_seq[0::cells_per_arrow], color=color, scale=visualize_config["arrow_scale"], headwidth = visualize_config["arrow_head_width"], ls=ls)
            else:
                ax.quiver(x_seq[0:-1:cells_per_arrow], y_seq[0:-1:cells_per_arrow], u_seq[0::cells_per_arrow], v_seq[0::cells_per_arrow], color=arrow_color, scale=visualize_config["arrow_scale"], headwidth = visualize_config["arrow_head_width"], ls=ls)

        if visualize_config["show_endpoints"]:
            ax.scatter(x_seq[0], y_seq[0], c="r")
            ax.scatter(x_seq[-1], y_seq[-1], c="g")
            ax.text(x_seq[0] + visualize_config["text_offset"][0], y_seq[0] + visualize_config["text_offset"][1], "I", fontsize=visualize_config["text_font_size"])
            ax.text(x_seq[-1] + visualize_config["text_offset"][0], y_seq[-1] + visualize_config["text_offset"][1], "F", fontsize=visualize_config["text_font_size"])
        return ax

    def draw(self, env_only = False, block = True):
        plt.figure()
        ax = plt.gca()
        ax = self.sketch_environment(ax)
        if not env_only:
            self.sketch_plan(ax)
        plt.show(block=False)
        plt.gcf().set_size_inches(visualize_config["figure_size"][0], visualize_config["figure_size"][1])
        if block:
            self._block_for_input()

    def sketch_environment(self, ax = None):
        if not ax:
            ax = plt.gca()
        ax.set_xticks(range(self.__grid_size[0]))
        ax.set_yticks(range(self.__grid_size[1]))
        ax.set_xlim(-0.5, self.__grid_size[0] - 0.5)
        ax.set_ylim(-0.5, self.__grid_size[1] - 0.5)
        ax.set_xticks(np.arange(0.5, self.__grid_size[0] + 0.5, 1), minor=True)
        ax.set_yticks(np.arange(0.5, self.__grid_size[1] + 0.5, 1), minor=True)
        if not visualize_config["show_ticks"]:
            ax.axes.xaxis.set_ticklabels([])
            ax.axes.yaxis.set_ticklabels([])
        ax.grid(which="minor", ls=visualize_config["grid_line_style"], lw=visualize_config["grid_line_width"])

        if self.__environment:
            self.__draw_regions(ax)

        if self.__obstacles:
            self.__draw_obstacles(ax)
        
        return ax
    
    @staticmethod
    def __state_str_to_coord(state_str):
        split_str = state_str.split(", ")
        assert len(split_str) == 2
        return (int(split_str[0].replace('x', '')), int(split_str[1].replace('y', '')))
    
    def __draw_regions(self, ax, show_unique_region_labels_only = True):
        unique_regions = set()
        for region in self.__environment:
            if region["fancy"]:
                corner = visualize_config["fancy_corner"]
                width = region["upper_right_x"] - region["lower_left_x"] + 1 - visualize_config["region_scale"] - corner
                height = region["upper_right_y"] - region["lower_left_y"] + 1 - visualize_config["region_scale"] - corner
                x = region["lower_left_x"] - 0.5 + .5*visualize_config["region_scale"] + corner / 2
                y = region["lower_left_y"] - 0.5 + .5*visualize_config["region_scale"] + corner / 2
                rectangle = FancyBboxPatch((x, y), width, height, color=region["color"], mutation_scale=corner, alpha=region["alpha"])
            else:
                width = region["upper_right_x"] - region["lower_left_x"] + 1 - visualize_config["region_scale"]
                height = region["upper_right_y"] - region["lower_left_y"] + 1 - visualize_config["region_scale"]
                x = region["lower_left_x"] - 0.5
                y = region["lower_left_y"] - 0.5
                rectangle = Rectangle((x, y), width, height, color=region["color"], alpha=region["alpha"])

            ax.add_patch(rectangle)
            if visualize_config["show_text"] and (region["label"] not in unique_regions or not show_unique_region_labels_only):
                ax.text(region["lower_left_x"] - 0.5 + visualize_config["text_offset"][0], region["lower_left_y"] - 0.5 + visualize_config["text_offset"][1], region["label"], fontsize=visualize_config["text_font_size"])
                unique_regions.add(region["label"])
    
    def __draw_obstacles(self, ax):
        for obstacle in self.__obstacles:
            width = obstacle["upper_right_x"] - obstacle["lower_left_x"] + 1
            height = obstacle["upper_right_y"] - obstacle["lower_left_y"] + 1
            rectangle = Rectangle((obstacle["lower_left_x"] - 0.5, obstacle["lower_left_y"] - 0.5), width, height, color=visualize_config["obstacle_color"])
            ax.add_patch(rectangle)


    def _block_for_input(self):
        input("Press key to close")



if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("--read-directory", default="../../build/bin/grid_world_plans", help="Specify a directory to read plan files from")
    parser.add_argument("--config-filepath", default="../../build/bin/configs/grid_world_config.yaml", help="Specify a grid world config file")
    parser.add_argument("--environment-only", default=False, action="store_true", help="Show the environment only")
    args = parser.parse_args()

    visualizer = GridWorldAgentVisualizer(args.config_filepath)

    if not args.environment_only:
        for file in os.scandir(args.read_directory):
            visualizer.deserialize(file)
            visualizer.draw(block=False)
        visualizer.block_for_input()
    else:
        visualizer.draw(env_only=True)
        

