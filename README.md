# Task Planning Library

An abstract, efficient C++ library for autonomous high-level discrete task planning.

### Why?

Imagine you have a robotic system equipped with motion planning, low-level control, and state estimation/observation. This robot can *plan* safe non-colliding continuous movements, *execute* these movements in real time, and may even be able to *observe* state feedback. Now put this robot in a kitchen, and tell it to efficiently and safely prepare meals for the customers. Your robot may know how to move from one location to the other, or plan a motion that flips a burger patty. But how does your robot know how to assemble each meal correctly, efficiently, and safely? 

This task planning library gives you the tools and algorithms to utilize or design a discrete task planner, capable of satisfying complex Linear Temporal Logic (LTL) task specifications.

### What is included?

#### Generic Shortest Path Problem Graph Search
 - **Dijkstra's Algorithm** (Single-objective shortest path)
 - **A\*** (Single-objective heuristic-guided shortest path)
 - **Bi-Objective A\*** (a.k.a. BOA*, Two-objective optimal trade-off Pareto Front)
 - **New Approach to Multi-Objective A\*** (a.k.a. NAMOA*, N-objective optimal trade-off Pareto Front)

#### Custom Model Generator
The custom model generator and logic parser allow the user to create discrete *Transition System* models of an autonomous system. This library also includes simple model generators for a robotic manipulator capable of grasping and moving objects, and a grid-world mobile robot with rectangular regions of interest.

#### Task Planners
The **Deterministic Task Planner** will autonomously plan a sequence of actions that when executed from the initial state, satisfies a set of LTL tasks. The **Bi-Objective Preference Planner** and **Multi-Objective Preference Planner** will plan a sequence of actions that will satisfies a set of LTL tasks, while optimizing over multiple preference objectives.


###### Dependencies
 - C++20
 - Python3
 - SpotLTL
 - YAML-cpp
 - Eigen
 - MATLAB (deprecating)


###### Building
First install YAML-cpp (git submodule) and Eigen in the `dependencies` directory.

Then simply build using CMake
```
mkdir build && cd build
cmake ..
make
```
