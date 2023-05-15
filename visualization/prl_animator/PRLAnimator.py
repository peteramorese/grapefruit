#!/usr/bin/env python3

import os
import sys
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import yaml
import argparse

# Internal Modules
sys.path.append("..")
from grid_world_agent.DrawPlan import GridWorldAgentVisualizer
from pareto_front.PRLParetoFront import PRLParetoFrontVisualizer

visualize_config = {
    "chosen_plan_color": "crimson",
    "candidate_plan_color": "cadetblue"
}

class PRLAnimator:
    def __init__(self, config_filepath):
        self._plan_visualizer = GridWorldAgentVisualizer(config_filepath)
        self._pf_visualizer = PRLParetoFrontVisualizer()

    def deserialize(self, filepath):
        with open(filepath, "r") as f:
            self._data = yaml.safe_load(f)

    def __initialize(self):
        self._instances = self._data["Instances"]

        # Make pf data
        pf_data = dict()
        pf_data["PRL Preference Mean"] = self._data["PRL Preference Mean"]
        pf_data["PRL Preference Variance"] = self._data["PRL Preference Variance"]
        self._pf_visualizer.load_from_dict(pf_data)
        

    def animate(self):
        fig, (plan_ax, pf_ax) = plt.subplots(1, 2)

        self.__initialize()

        def init():
            self._plan_visualizer.sketch_environment(plan_ax)
            return plan_ax,

        def update(frame):
            print("FRAME: ", frame)
            instance_key = "Instance " + str(frame)
            print("Animating ", instance_key)
            try:
                instance_data = self._data[instance_key]
            except ValueError:
                print(instance_key," not found in animation file")
            chosen_plan = instance_data["Chosen Plan"]
            for k, v in instance_data.items():
                if k.startswith("Candidate Plan"):
                    self._plan_visualizer.load_from_dict(v)
                    color = visualize_config["candidate_plan_color"] if k == chosen_plan else visualize_config["chosen_plan_color"]
                    self._plan_visualizer.sketch_plan(plan_ax, color=color)
            return plan_ax,
        
        print("Number of instances: ", self._instances)
        animator = FuncAnimation(fig, update, frames=self._instances, init_func=init, blit=True)
        plt.show()
            

    
    

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath",default="animation.yaml", help="Specify animation file")
    parser.add_argument("--config-filepath", default="../../build/bin/configs/grid_world_config.yaml", help="Specify a grid world config file")
    args = parser.parse_args()

    animator = PRLAnimator(args.config_filepath)

    animator.deserialize(args.filepath)
    animator.animate()
    #animator.draw(use_legend=True)