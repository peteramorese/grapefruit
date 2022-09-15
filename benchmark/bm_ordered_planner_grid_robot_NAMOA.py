#! /root/miniconda3/envs/tpenv/bin/python
import os
import struct
import sys
import subprocess
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__),"../spot_automaton_file_dump"))

import formula2dfa 
import bm_preference_planner_grid_robot

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-N", "--num-dfas", nargs='?', type=int, default=None, help="Number of formulas (default to the number of found formulas")
    parser.add_argument("-l", "--formula-list", default="default", help="Specify formula list inside json formula file")
    parser.add_argument("-t", "--trials", nargs='?', type=int, default=5, help="Number of randomized trials")
    parser.add_argument("-s", "--grid-size", nargs='?', type=int, default=10)
    parser.add_argument("-v", "--verbose", action='store_true', default=False, help="Display the formula ordering")
    args = parser.parse_args()
    trials = args.trials 
    grid_size = args.grid_size
    num_dfas = args.num_dfas

    print("Starting benchmark: ordered_planner_grid_robot_NAMOA")

    READ_FILE_NAME = "ordered_planner_bm_formulas.json"
    WRITE_FILE_DIR_NAME_PREFIX = "../spot_automaton_file_dump/dfas/"
    EXEC_FILE_NAME = "ordered_planner_grid_robot_namoa"
    BM_DATA_FILE_NAME_NO_H = "benchmark_data/bm_ordered_planner_namoa.txt"
    BM_DATA_FILE_NAME_H = "benchmark_data/bm_ordered_planner_heuristic_namoa.txt"

    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_NO_H) # Clear the bm session file
    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_H) # Clear the bm session file
    for i in range(0, trials):
        print("Working on trial {} out of {}...".format(i + 1, trials))
        num_dfas_found = formula2dfa.read_write_json(READ_FILE_NAME, args.formula_list, WRITE_FILE_DIR_NAME_PREFIX, random_ordering=True, verbose=args.verbose)
        if not num_dfas:
            num_dfas = num_dfas_found
        if num_dfas <= 2:
            print("Error: Create more than 2 BM formulas")
            break
        for j in range(2, num_dfas+1):
            #print("\nDijkstra's:")
            bm_preference_planner_grid_robot.exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=j, 
                mu=None, 
                use_h_flag=False, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                bm_manual_iterations=False, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_NO_H,
                grid_size=grid_size)
            ##print("\nAstar:")
            bm_preference_planner_grid_robot.exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=j, 
                mu=None, 
                use_h_flag=True, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                bm_manual_iterations=False, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_H,
                grid_size=grid_size)


