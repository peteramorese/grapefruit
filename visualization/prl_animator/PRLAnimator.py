#!/usr/bin/env python3

import os
import sys
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.animation import FuncAnimation
import yaml
import argparse

# Internal Modules
sys.path.append("..")
from grid_world_agent.DrawPlan import GridWorldAgentVisualizer
from pareto_front.PRLParetoFront import PRLParetoFrontVisualizer

visualize_config = {
    "chosen_plan_color": "crimson",
    "candidate_plan_color": "cadetblue",
    "legend_font_size": 10,
    "selection_line_color": "red",
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

        pref_mean = self._data["PRL Preference Mean"]

        samples = {
            "Objective 0": [],
            "Objective 1": [],
        }

        def init():
            self._plan_visualizer.sketch_environment(plan_ax)
            samples["Objective 0"].clear()
            samples["Objective 1"].clear()
            return plan_ax, pf_ax

        def update(frame):

            # Plan
            plan_ax.clear()
            pf_ax.clear()
            self._pf_visualizer.clear_data_sets()
            self._plan_visualizer.sketch_environment(plan_ax)
            self._pf_visualizer.sketch_preference_distribution(pf_ax)
            print("FRAME: ", frame)
            instance_key = "Instance " + str(frame)
            print("Animating ", instance_key)
            try:
                instance_data = self._data[instance_key]
            except ValueError:
                print(instance_key," not found in animation file")

            # Add sample
            samples["Objective 0"].append(instance_data["Sample"][0])
            samples["Objective 1"].append(instance_data["Sample"][1])
            chosen_plan = instance_data["Chosen Plan"]

            for k, v in instance_data.items():
                if k.startswith("Candidate Plan"):
                    mean = np.array(v["Plan Mean"])
                    variance = np.array(v["Plan Variance"])
                    if k != chosen_plan:
                        color = visualize_config["candidate_plan_color"] 
                        title = k
                    else:
                        chosen_mean = mean
                        color = visualize_config["chosen_plan_color"]
                        title = "Chosen Plan"
                    self._plan_visualizer.load_from_dict(v.copy())
                    self._plan_visualizer.sketch_plan(plan_ax, color=color, label=title)
                    self._pf_visualizer.sketch_distribution(mean, variance, pf_ax, levels=2, fill_contour=False, label=k)

            # Add samples to pf
            self._pf_visualizer.add_data_set(samples.copy())
            self._pf_visualizer.sketch_pareto_front(pf_ax, label="Samples")
            selection_line_pts = np.stack((pref_mean, chosen_mean))
            pf_ax.plot(selection_line_pts[:,0], selection_line_pts[:,1], color=visualize_config["selection_line_color"], ls=":")
            plan_ax.legend(fontsize=visualize_config["legend_font_size"], loc="upper left")
            pf_ax.legend(fontsize=visualize_config["legend_font_size"], loc="upper left")

            # Pareto Front
            
            return plan_ax, pf_ax
        
        print("Number of instances: ", self._instances)
        animator = FuncAnimation(fig, update, frames=self._instances, init_func=init, interval=1000, blit=True, repeat=False)
        animator.save("test.gif")
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