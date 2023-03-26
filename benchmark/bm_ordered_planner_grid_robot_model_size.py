#! /root/miniconda3/envs/tpenv/bin/python
import os
import struct
import sys
import subprocess
import argparse

sys.path.append(os.path.join(os.path.dirname(__file__),"../spot_automaton_file_dump"))

import formula2dfa 
import bm_preference_planner_grid_robot
from generate_random_formulas import GenerateRandomFormulas


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-N", "--num-dfas", nargs='?', type=int, default=3, help="Number of formulas (default to the number of found formulas")
    #parser.add_argument("-l", "--formula-list", default="default", help="Specify formula list inside json formula file")
    parser.add_argument("-t", "--trials", nargs='?', type=int, default=10, help="Number of randomized trials")
    parser.add_argument("-s", "--max-grid-size", nargs='?', type=int, default=50)
    parser.add_argument("-d", "--grid-size-step", nargs='?', type=int, default=10)
    parser.add_argument("--formula-size", nargs='?', type=int, default=3)
    parser.add_argument("-v", "--verbose", action='store_true', default=False, help="Display the formula ordering")
    args = parser.parse_args()
    trials = args.trials #Number of random orderings
    num_dfas = args.num_dfas
    if num_dfas <= 2:
        print("Warning: Create more than 2 BM formulas")

    formula_size_to_template = {
        2: "F($)",
        3: "F($ & F($))",
        5: "F($ & F($) & F($))",
        7: "F($ & F($) | F($ & F($))) & !$ U $"
    }
    try:
        template = formula_size_to_template[args.formula_size]
    except:
        print("(bm_ordered_planner_grid_robot_model_size) Formula template for size: ", args.formula_size, " not found")
        return

    print("Starting benchmark: ordered_planner_grid_robot_model_size")

    #READ_FILE_NAME = "ordered_planner_bm_formulas.json"
    WRITE_FILE_DIR_NAME_PREFIX = "../spot_automaton_file_dump/dfas/"
    EXEC_FILE_NAME = "ordered_planner_grid_robot"
    BM_DATA_FILE_NAME_FLEX_NO_H = "benchmark_data/bm_ordered_planner_model_size.txt"
    BM_DATA_FILE_NAME_FLEX_H = "benchmark_data/bm_ordered_planner_heuristic_model_size.txt"

    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_FLEX_NO_H) # Clear the bm session file
    bm_preference_planner_grid_robot.clear_file(BM_DATA_FILE_NAME_FLEX_H) # Clear the bm session file

    gen = GenerateRandomFormulas(template)
    for i in range(0, trials):
        print("Working on trial {} out of {}...".format(i + 1, trials))
        for grid_size in range(10, args.max_grid_size + 1, args.grid_size_step):
            gen.generateGridWorld("bm_formulas", args.num_dfas, 0, grid_size, 0, grid_size)
            if args.verbose: 
                print(gen.getFormulaList())
            #num_dfas_found = formula2dfa.read_write_json(READ_FILE_NAME, args.formula_list, WRITE_FILE_DIR_NAME_PREFIX, random_ordering=True, verbose=args.verbose)
            formula2dfa.create_file(gen.getFormulaList()["bm_formulas"], WRITE_FILE_DIR_NAME_PREFIX, None, True, verbose=args.verbose)
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

if __name__ == "__main__":
    main()