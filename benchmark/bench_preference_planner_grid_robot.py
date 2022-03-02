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
    print("Executable file path: ",exec_file_path)
    exec_cmd = os.path.join(os.path.dirname(__file__), exec_file_path)
    print("lksdfj ", exec_cmd)
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
    print("\nFINISHED ON PYTHON SIDE")
    #pc.read()

if __name__ == "__main__":
    print("Starting benchmark: preference_planner_grid_robot")

    READ_FILE_NAME = "benchmark_formulas.txt"
    WRITE_FILE_DIR_NAME_PREFIX = "../spot_automaton_file_dump/dfas/"
    EXEC_FILE_NAME = "preference_planner_grid_robot"
    num_dfas = formula2dfa.read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX)
    grid_size = 10
    exec_pref_plan_grid_robot(EXEC_FILE_NAME, grid_size, num_dfas, 10000, 'n', 'n', 'n', 'y', WRITE_FILE_DIR_NAME_PREFIX)



