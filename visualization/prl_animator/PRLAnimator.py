#!/usr/bin/env python3

import os
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import yaml
import sys
sys.path.append("..")
from grid_world_agent.DrawPlan import GridWorldAgentVisualizer

class PRLAnimator:
    def __init__(self):
        pass

    def deserialize(self, filepath):
        with open(filepath, "r") as f:
            self._data = yaml.safe_load(f)
    
    

if __name__ == "__main__":
    print("hello")