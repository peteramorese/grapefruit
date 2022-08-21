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
    BM_DATA_FILE_NAME_NO_H = "benchmark_data/bm_ordered_planner.txt"
    BM_DATA_FILE_NAME_H = "benchmark_data/bm_ordered_planner_heuristic.txt"

    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_NO_H) # Clear the bm session file
    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_H) # Clear the bm session file
    trials_formulas = 3 #Number of random orderings
    trials_flexibility = 3 #Number of random orderings
    grid_size = 10
    mu = 10000
    mu_disc = range(0, 55, 5)
    print("\n\nScaling Formulas: \n\n")
    for i in range(0, trials_formulas):
        print("Working on trial {} out of {}...".format(i + 1, trials_formulas))
        num_dfas = formula2dfa.read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, random_ordering=True)
        if num_dfas <= 2:
            print("Error: Create more than 2 BM formulas")
            break
        for j in range(2, num_dfas + 1):

            # Run without the heuristic:
            bm_preference_planner_grid_robot.exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=j, 
                mu=mu, 
                use_h_flag=False, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_NO_H,
                grid_size=grid_size)

            # Run with the heuristic:
            bm_preference_planner_grid_robot.exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=j, 
                mu=mu, 
                use_h_flag=True, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_H,
                grid_size=grid_size)


