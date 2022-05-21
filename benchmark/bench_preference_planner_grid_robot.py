import os
import struct
import sys
import subprocess
#print(sys.executable)
#print(sys.path)


sys.path.append(os.path.join(os.path.dirname(__file__),"../spot_automaton_file_dump"))
#print(os.path.join(os.path.dirname(__file__),"../spot_automaton_file_dump"))

import formula2dfa 


def exec_pref_plan_grid_robot(exec_file_name, num_dfas, mu, use_h_flag, write_file_flag, verbose, benchmark, dfas_filepath, bm_file, grid_size=None, ):
    exec_dir_name = "../build/bin/"
    exec_file_path = exec_dir_name + exec_file_name
    #print("Executable file path: ",exec_file_path)
    exec_cmd = os.path.join(os.path.dirname(__file__), exec_file_path)
    exec_cmd += ' --numdfas ' + str(num_dfas) 
    exec_cmd += ' --mu ' + str(mu) 
    if use_h_flag:
        exec_cmd += ' --use-h' 
    if verbose:
        exec_cmd += ' --verbose'
    if benchmark:
        exec_cmd += ' --benchmark'
    if write_file_flag:
        exec_cmd += ' --write-plan'
    exec_cmd += ' --dfas-filepath ' + dfas_filepath
    exec_cmd += ' --bm-file ' + bm_file
    if grid_size is not None:
        exec_cmd += ' --gridsize ' + str(grid_size) 
    #print("Executable command: ", exec_cmd)
    pc = subprocess.call(exec_cmd, shell=True)
    #print("\nFINISHED ON PYTHON SIDE")
    #pc.read()

def clear_file(file_name):
    file = open(file_name, "w")
    file.close()

if __name__ == "__main__":
    print("Starting benchmark: preference_planner_grid_robot")

    READ_FILE_NAME = "benchmark_formulas.txt"
    WRITE_FILE_DIR_NAME_PREFIX = "../spot_automaton_file_dump/dfas/"
    EXEC_FILE_NAME = "preference_planner_grid_robot"
    BM_DATA_FILE_NAME_NO_H = "benchmark_data/bm_preference_planner.txt"
    BM_DATA_FILE_NAME_H = "benchmark_data/bm_preference_planner_heuristic.txt"
    BM_DATA_FILE_NAME_FLEX_NO_H = "benchmark_data/bm_preference_planner_flex.txt"
    BM_DATA_FILE_NAME_FLEX_H = "benchmark_data/bm_preference_planner_heuristic_flex.txt"

    clear_file(BM_DATA_FILE_NAME_NO_H) # Clear the bm session file
    clear_file(BM_DATA_FILE_NAME_H) # Clear the bm session file
    clear_file(BM_DATA_FILE_NAME_FLEX_NO_H) # Clear the bm session file
    clear_file(BM_DATA_FILE_NAME_FLEX_H) # Clear the bm session file
    trials_formulas = 4 #Number of random orderings
    trials_flexibility = 4 #Number of random orderings
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
            exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
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
            exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=j, 
                mu=mu, 
                use_h_flag=True, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_H,
                grid_size=grid_size)
    print("\n\nScaling Flexibility: \n\n")
    for i in range(0, trials_flexibility):
        print("Working on trial {} out of {}...".format(i + 1, trials_flexibility))
        num_dfas = formula2dfa.read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, random_ordering=False, verbose=False)
        if num_dfas <= 2:
            print("Error: Create more than 2 BM formulas")
            break
        for mu_i in mu_disc:
            #print("\nDijkstra's:")
            exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=num_dfas, 
                mu=mu_i, 
                use_h_flag=False, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_FLEX_NO_H,
                grid_size=grid_size)
            #print("\nAstar:")
            exec_pref_plan_grid_robot(EXEC_FILE_NAME, 
                num_dfas=num_dfas, 
                mu=mu_i, 
                use_h_flag=True, 
                write_file_flag=False, 
                verbose=False, 
                benchmark=True, 
                dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                bm_file=BM_DATA_FILE_NAME_FLEX_H,
                grid_size=grid_size)


