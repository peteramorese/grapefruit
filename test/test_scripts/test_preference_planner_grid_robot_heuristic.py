import os
import sys
import subprocess
sys.path.append(os.path.join(os.path.dirname(__file__),"../../spot_automaton_file_dump"))
import formula2dfa 


def exec_test_pref_plan_grid_robot_heuristic(exec_file_name, num_dfas, mu, verbose, dfas_filepath, grid_size=None):
    exec_dir_name = "../../build/bin/"
    exec_file_path = exec_dir_name + exec_file_name
    #print("Executable file path: ",exec_file_path)
    exec_cmd = os.path.join(os.path.dirname(__file__), exec_file_path)
    exec_cmd += ' --numdfas ' + str(num_dfas) 
    exec_cmd += ' --mu ' + str(mu) 
    if verbose:
        exec_cmd += ' --verbose'
    exec_cmd += ' --dfas-filepath ' + dfas_filepath
    if grid_size is not None:
        exec_cmd += ' --gridsize ' + str(grid_size) 
    #print("Executable command: ", exec_cmd)
    pc = subprocess.call(exec_cmd, shell=True)
    #print("\nFINISHED ON PYTHON SIDE")
    #pc.read()


if __name__ == "__main__":
    print("Starting test: test_preference_planner_grid_robot_heuristic")

    READ_FILE_NAME = "test_formulas.txt"
    WRITE_FILE_DIR_NAME_PREFIX = "../../spot_automaton_file_dump/dfas/"
    EXEC_FILE_NAME = "test_preference_planner_grid_robot_heuristic"

    trials = 5 #Number of random orderings
    grid_size = 10
    mu_disc = range(0, 50, 5)
    print("\n\nScaling Formulas: \n\n")
    count = 0
    for _ in range(0, trials):
        num_dfas = formula2dfa.read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, random_ordering=True)
        if num_dfas <= 2:
            print("Error: Create more than 2 BM formulas")
            break
        for j in range(2, num_dfas + 1):
            for mu in mu_disc:

            # Run without the heuristic:
                exec_test_pref_plan_grid_robot_heuristic(EXEC_FILE_NAME, 
                    num_dfas=j, 
                    mu=mu, 
                    verbose=False, 
                    dfas_filepath=WRITE_FILE_DIR_NAME_PREFIX,
                    grid_size=grid_size)
                count += 1
    print("Test finished. " + count + " scenarios tested.")
