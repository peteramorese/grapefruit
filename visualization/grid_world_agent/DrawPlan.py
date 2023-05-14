#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
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
    "show_ticks": False,
    "show_title": True,
    "show_endpoints": True,
    "text_offset": (.1, .5),
    "traj_offset_magnitude": 0.2,
    "arrow_color": "path_line_color",
    "arrow_scale": 20.0,
    "arrow_head_width": 5,
    "cells_per_arrow": 2,
    "show_directions": True,
    "figure_size": (4, 4)
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
        
    def deserialize(self, filepath):
        self.__reset()

        with open(filepath, "r") as f:
            self.__plan_properties = yaml.safe_load(f)

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
            if visualize_config["show_text"] and (region["label"] not in unique_regions or not show_unique_regions_only):
                ax.text(region["lower_left_x"] - 0.5 + visualize_config["text_offset"][0], region["lower_left_y"] - 0.5 + visualize_config["text_offset"][1], region["label"], fontsize=visualize_config["text_font_size"])
                unique_regions.add(region["label"])


    def sketch_plan(self, ax = None):
        if not ax:
            ax = self.sketch_environment()
        x_seq = list()
        y_seq = list()
        u_seq = list()
        v_seq = list()
        x_seq.append(self.__state_str_to_coord(self.__plan_properties["State Sequence"][0])[0])
        y_seq.append(self.__state_str_to_coord(self.__plan_properties["State Sequence"][0])[1])
        prev_offset = np.array([0.0, 0.0])
        for i in range(1, len(self.__plan_properties["State Sequence"])):
            s_prev_coord = self.__state_str_to_coord(self.__plan_properties["State Sequence"][i-1])
            s_coord = self.__state_str_to_coord(self.__plan_properties["State Sequence"][i])
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

        if "Title" in self.__plan_properties:
            print("Displaying: ", self.__plan_properties["Title"])
            if visualize_config["show_title"]:
                ax.set_title(self.__plan_properties["Title"])

        ax.plot(x_seq, y_seq, ls=visualize_config["path_line_style"], lw=visualize_config["path_line_width"], color=visualize_config["path_line_color"])

        if visualize_config["show_directions"]:
            cells_per_arrow = visualize_config["cells_per_arrow"]
            if visualize_config["arrow_color"] == "path_line_color":
                ax.quiver(x_seq[0:-1:cells_per_arrow], y_seq[0:-1:cells_per_arrow], u_seq[0::cells_per_arrow], v_seq[0::cells_per_arrow], color=visualize_config["path_line_color"], scale=visualize_config["arrow_scale"], headwidth = visualize_config["arrow_head_width"])
            else:
                ax.quiver(x_seq[0:-1:cells_per_arrow], y_seq[0:-1:cells_per_arrow], u_seq[0::cells_per_arrow], v_seq[0::cells_per_arrow], color=visualize_config["arrow_color"], scale=visualize_config["arrow_scale"], headwidth = visualize_config["arrow_head_width"])

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
            self.block_for_input()

    def block_for_input(self):
        input("Press key to close")

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
        
        return ax
    




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
        

