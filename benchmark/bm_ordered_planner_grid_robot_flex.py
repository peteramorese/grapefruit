import os
import struct
import sys
import subprocess

sys.path.append(os.path.join(os.path.dirname(__file__),"../spot_automaton_file_dump"))

import formula2dfa 
import bm_preference_planner_grid_robot

if __name__ == "__main__":
    print("Starting benchmark: ordered_planner_grid_robot")

    READ_FILE_NAME = "ordered_planner_bm_formulas.txt"
    WRITE_FILE_DIR_NAME_PREFIX = "../spot_automaton_file_dump/dfas/"
    EXEC_FILE_NAME = "ordered_planner_grid_robot"
    BM_DATA_FILE_NAME_FLEX_NO_H = "benchmark_data/bm_ordered_planner_flex.txt"
    BM_DATA_FILE_NAME_FLEX_H = "benchmark_data/bm_ordered_planner_heuristic_flex.txt"

    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_FLEX_NO_H) # Clear the bm session file
    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_FLEX_H) # Clear the bm session file
    trials_flexibility = 5 #Number of random orderings
    grid_size = 10
    mu = 10000
    #mu_disc = range(0, 55, 5)
    print("\n\nRunning flexibility trials: \n\n")
    for i in range(0, trials_flexibility):
        print("Working on trial {} out of {}...".format(i + 1, trials_flexibility))
        num_dfas = formula2dfa.read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, random_ordering=True, verbose=True)
        if num_dfas <= 2:
            print("Error: Create more than 2 BM formulas")
            break
        else:
            #print("\nDijkstra's:")
            bm_preference_planner_grid_robot.exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=num_dfas, 
                mu=None, 
                use_h_flag=False, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_FLEX_NO_H,
                grid_size=grid_size)
            ##print("\nAstar:")
            bm_preference_planner_grid_robot.exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=num_dfas, 
                mu=None, 
                use_h_flag=True, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_FLEX_H,
                grid_size=grid_size)


