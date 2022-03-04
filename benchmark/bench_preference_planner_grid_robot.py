import os
import sys
import subprocess
print(sys.executable)
#print(sys.path)


sys.path.append(os.path.join(os.path.dirname(__file__),"../spot_automaton_file_dump"))
#print(os.path.join(os.path.dirname(__file__),"../spot_automaton_file_dump"))

import formula2dfa 

def exec_pref_plan_grid_robot(exec_file_name, grid_size, num_dfas, mu, use_h_flag, write_file_flag, verbose, benchmark, write_file_dir_name_prefix):
    exec_dir_name = "../build/bin/"
    exec_file_path = exec_dir_name + exec_file_name
    #print("Executable file path: ",exec_file_path)
    exec_cmd = os.path.join(os.path.dirname(__file__), exec_file_path)
    exec_cmd += ' ' + str(grid_size) 
    exec_cmd += ' ' + str(num_dfas) 
    exec_cmd += ' ' + str(mu) 
    exec_cmd += ' ' + use_h_flag
    exec_cmd += ' ' + write_file_flag
    exec_cmd += ' ' + verbose
    exec_cmd += ' ' + benchmark
    exec_cmd += ' ' + write_file_dir_name_prefix
    print("Executable command: ", exec_cmd)
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
    BM_DATA_FILE_NAME = "benchmark_data/preference_planner_bm.txt"

    clear_file(BM_DATA_FILE_NAME) # Clear the bm session file
    trials = 5 #Number of random orderings
    grid_size = 10
    mu = 10000
    for _ in range(0, trials):
        num_dfas = formula2dfa.read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, random_ordering=True)
        if num_dfas <= 2:
            print("Error: Create more than 2 BM formulas")
            break
        for j in range(2, num_dfas):
            print("\n PYTHON j: ", j, "\n")
            exec_pref_plan_grid_robot(EXEC_FILE_NAME, grid_size, j, mu, 'n', 'n', 'n', 'y', WRITE_FILE_DIR_NAME_PREFIX)



