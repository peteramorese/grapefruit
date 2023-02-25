#!/usr/bin/env python3

import matplotlib.pyplot as plt
import numpy as np
import yaml


class GridWorldAgentVisualizer:
    def __init__(self, config_filepath):
        self.__plan_properties = dict()

        with open(config_filepath, "r") as f:
            self.__config = yaml.safe_load(f)

        self.__grid_size = (self.__config["Grid X"], self.__config["Grid Y"])
    
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

        plt.xticks(range(self.__grid_size[0]))
        plt.yticks(range(self.__grid_size[1]))
        ax = plt.gca()
        ax.set_xticks(np.arange(0.5, self.__grid_size[0] + 0.5, 1), minor=True)
        ax.set_yticks(np.arange(0.5, self.__grid_size[1] + 0.5, 1), minor=True)
        plt.grid(which="minor", ls="-", lw=1)

        plt.plot(x_seq, y_seq)

        if show_endpoints:
            plt.scatter(x_seq[0], y_seq[0], c="r")
            plt.text(x_seq[0] + 0.1, y_seq[0] + 0.1, "I")
            plt.scatter(x_seq[-1], y_seq[-1], c="g")
            plt.text(x_seq[-1] + 0.1, y_seq[-1] + 0.1, "F")

    def display(self):
        plt.show()

if __name__ == "__main__":
    visualizer = GridWorldAgentVisualizer("test_grid_world_config.yaml")
    visualizer.serialize("test_plan.yaml")
    visualizer.display()
        

