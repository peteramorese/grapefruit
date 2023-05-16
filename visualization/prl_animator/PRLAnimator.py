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
    "ucb_line_color": "cadetblue",
    "show_point_lines": False,
    "speed_intervals": {
        "turtle": 2000,
        "rabbit": 1000,
        "cheetah": 500,
        "plane": 250,
        "zoom": 100
    }
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
        
    def __plot_ucb_pareto_point(self, ax, dist_mean, ucb_val, label = None):
        diff = np.stack((dist_mean, ucb_val))
        #ax.quiver(dist_mean[0], dist_mean[1], diff[0], diff[1], color=visualize_config["ucb_line_color"], ls="--", )
        ax.plot(diff[:,0], diff[:,1], color=visualize_config["ucb_line_color"], ls="--")
        ax.scatter(ucb_val[0], ucb_val[1], marker="x", s=20, color=visualize_config["ucb_line_color"], label=label)
        if visualize_config["show_point_lines"]:
            ax.axvline(ucb_val[0], ls=":")
            ax.axhline(ucb_val[1], ls=":")

    def animate(self, repeat = True, save_file = None, playback_speed = "rabbit", starting_instance = 0, ending_instance = None, show_full_traj = False):
        fig, (plan_ax, pf_ax) = plt.subplots(1, 2)

        self.__initialize()

        pref_mean = self._data["PRL Preference Mean"]

        samples = {
            "Objective 0": [],
            "Objective 1": [],
        }

        if show_full_traj:
            for inst in range(starting_instance):
                instance_key = "Instance " + str(inst + starting_instance)
                try:
                    instance_data = self._data[instance_key]
                except ValueError:
                    print(instance_key," not found in animation file")

                # Add sample
                samples["Objective 0"].append(instance_data["Sample"][0])
                samples["Objective 1"].append(instance_data["Sample"][1])

        def init():
            self._plan_visualizer.sketch_environment(plan_ax)
            #samples["Objective 0"].clear()
            #samples["Objective 1"].clear()
            return plan_ax, pf_ax

        def update(frame):

            # Plan
            plan_ax.clear()
            pf_ax.clear()
            self._pf_visualizer.clear_data_sets()
            self._plan_visualizer.sketch_environment(plan_ax)
            instance_key = "Instance " + str(frame + starting_instance)
            try:
                instance_data = self._data[instance_key]
            except ValueError:
                print(instance_key," not found in animation file")

            # Add sample
            samples["Objective 0"].append(instance_data["Sample"][0])
            samples["Objective 1"].append(instance_data["Sample"][1])
            chosen_plan = instance_data["Chosen Plan"]

            # Add samples to pf
            self._pf_visualizer.add_data_set(samples.copy())
            # Plot preference first (base layer)
            self._pf_visualizer.sketch_preference_distribution(pf_ax)

            for k, v in instance_data.items():
                if k.startswith("Candidate Plan"):
                    mean = np.array(v["Plan Mean"])
                    variance = np.array(v["Plan Variance"])
                    ucb_val = np.array(v["Plan Pareto UCB"])
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
                    self.__plot_ucb_pareto_point(pf_ax, mean, ucb_val, label = (k + " UCB value"))

            self._pf_visualizer.sketch_pareto_front(pf_ax, label="Samples", connect_points="line")
            selection_line_pts = np.stack((pref_mean, chosen_mean))
            pf_ax.plot(selection_line_pts[:,0], selection_line_pts[:,1], color=visualize_config["selection_line_color"], ls=":")
            
            # Add legends
            plan_ax.legend(fontsize=visualize_config["legend_font_size"], loc="upper left")
            pf_ax.legend(fontsize=visualize_config["legend_font_size"], loc="upper left")

            # Update title
            plan_ax.set_title(instance_key)
            
            return plan_ax, pf_ax
        
        if ending_instance and ending_instance < self._instances:
            assert ending_instance > starting_instance
            n_instances = ending_instance - starting_instance
        else:
            print("starting instance: ", starting_instance)
            n_instances = self._instances - starting_instance
        print("Animation starting from instance: ", starting_instance, " until instance: ", starting_instance + n_instances)
        animator = FuncAnimation(fig, update, frames=n_instances, init_func=init, interval=visualize_config["speed_intervals"][playback_speed], blit=False, repeat=repeat)
        if save_file:
            animator.save(save_file)
        plt.show()
            

    
    

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath",default="animation.yaml", help="Animation file")
    parser.add_argument("-r", "--repeat",action="store_true", help="Loop the animation")
    parser.add_argument("-s", "--save-filepath",default=None, help="Save the animation")
    parser.add_argument("--playback-speed",default="rabbit", help="Playback speed from slow to quick: (turtle, rabbit, cheetah, plane, zoom)")
    parser.add_argument("--config-filepath", default="../../build/bin/configs/grid_world_config.yaml", help="Specify a grid world config file")
    parser.add_argument("--start-instance", default=0, type=int, help="Animation starting instance")
    parser.add_argument("--full-traj", action="store_true", help="Show the trajectory before the start-instance")
    parser.add_argument("--end-instance", default=None, type=int, help="Animation ending instance")
    args = parser.parse_args()

    try:
        assert args.playback_speed in visualize_config["speed_intervals"].keys()
    except ValueError:
        print("Playback speed must be one of the preset values: (turtle, rabbit, cheetah, plane, zoom)")

    animator = PRLAnimator(args.config_filepath)

    animator.deserialize(args.filepath)
    animator.animate(repeat=args.repeat, save_file=args.save_filepath, playback_speed=args.playback_speed, starting_instance=args.start_instance, ending_instance=args.end_instance, show_full_traj=args.full_traj)
    #animator.draw(use_legend=True)