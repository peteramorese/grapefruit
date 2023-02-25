#! /root/miniconda3/envs/tpenv/bin/python
from posixpath import dirname
import spot, os, glob, random, json, yaml
import argparse

def remove_dfa_files(dirname_prefix, verbose=False):
    # Remove all dfa files in directory:
    for file in os.scandir(dirname_prefix):
        os.remove(file.path)
        if verbose:
            print("Removing: ", str(file))

def create_file(F_arr, dirname_prefix, custom_filename, random_ordering, verbose=False, f_complete=False, create_sub_map = False):

    file_dict = dict()

    remove_dfa_files(dirname_prefix, verbose)
    inds = list(range(0, len(F_arr)))
    if random_ordering:
        random.shuffle(inds)
    i = 0
    for F in F_arr: 
        file_ind = inds[i]
        if custom_filename is None:
            filename = dirname_prefix + "dfa_{}".format(file_ind) + ".yaml"
            if create_sub_map:
                sub_map_filename = dirname_prefix + "sub_map_{}".format(file_ind) + ".yaml"
            if verbose:
                print("    > Writing to: dfa_{}".format(file_ind))
        else:
            filename = dirname_prefix + custom_filename + ".yaml"
            if create_sub_map:
                sub_map_filename = dirname_prefix + custom_filename + ".yaml"
        i = i + 1
        lines_list = list()
        accepting_list = list()
        if f_complete:
            A = spot.formula(F).translate("BA","complete","deterministic","sbacc")
        else:
            A = spot.formula(F).translate("BA","deterministic","sbacc")
        bdict = A.get_dict()	
        file_dict["Alphabet"] = [str(ap) for ap in A.ap()]
        file_dict["Initial States"] = [A.get_init_state_number()]
        file_dict["Connections"] = dict()
        file_dict["Labels"] = dict()
        file_dict["Accepting States"] = list()
        for s in range(0, A.num_states()):
            begin = True
            for s_con in A.out(s):
                if begin:
                    file_dict["Connections"][s] = list()
                    file_dict["Labels"][s] = list()
                    begin = False
                file_dict["Connections"][s].append(s_con.dst)
                file_dict["Labels"][s].append(str(spot.bdd_format_formula(bdict, s_con.cond)))
                if "{}".format(s_con.acc) != "{}":
                    file_dict["Accepting States"].append(s)

        if create_sub_map:
            if verbose:
                print("Creating sub map: ", sub_map_filename)
            unique_obs_set = set()
            sub_map_file_dict = dict()
            for _, labels in file_dict["Labels"].items():
                for label in labels:
                    unique_obs_set.add(label)
            sub_map_file_dict["From Observations"] = list(unique_obs_set)
            sub_map_file_dict["To Observations"] = ["INSERT_HERE" for _ in range(0, len(unique_obs_set))]
            sub_map_file_dict["Costs"] = [99 for _ in range(0, len(unique_obs_set))]
            with open(sub_map_filename, "w+") as file:
                yaml.dump(sub_map_file_dict, file)
                

        with open(filename, "w+") as file:
            yaml.dump(file_dict, file)

def print_automaton(A):
    bdict = A.get_dict()	
    #print("acceptance: ", A.get_acceptance())
    print("num states: ", A.num_states())
    for ap in A.ap():
        print(" ", ap)
    for s in range(0, A.num_states()):
        print("state {}".format(s))
        for s_con in A.out(s):
            print("  edge({} -> {})".format(s_con.src, s_con.dst))
            print("    label: ", spot.bdd_format_formula(bdict, s_con.cond))
            print("    accepting sets: ", s_con.acc)


def read_write_txt(read_file_name, write_file_dir_name_prefix, write_file_name=None, random_ordering=False, verbose=False, f_complete=False):
    with open(read_file_name, "r") as formula_file:
        lines = formula_file.readlines()

    custom_single = False
    if write_file_name is not None:
        custom_single = True
        
    F_arr = list()
    for line in lines:
        F_i = line.replace("\n","")
        if not line == "\n": 
            if not F_i[0]=="#":
                if verbose:
                    print("  > Found formula:     ",F_i)
                F_arr.append(F_i)
                if custom_single:
                    break

    if verbose: 
        if not custom_single:
            print("  > Number of formulas: ", len(F_arr))
        else:
            print("Running as single custom filename...")
    create_file(F_arr, write_file_dir_name_prefix, write_file_name, random_ordering, verbose=verbose, f_complete=f_complete)
    return len(F_arr)

def read_write_json(read_json_name, formula_list_name, write_file_dir_name_prefix, write_file_name=None, random_ordering=False, verbose=False, f_complete=False, create_sub_map=False):
    with open(read_json_name) as f:
        formulas = json.load(f)

    custom_single = False
    if write_file_name is not None:
        custom_single = True
        
    F_arr = list()
    for F_i in formulas[formula_list_name]:
        if not F_i[0]=="#":
            if verbose:
                print("  > Found formula:     ",F_i)
            F_arr.append(F_i)
            if custom_single:
                break

    if verbose: 
        if not custom_single:
            print("  > Number of formulas: ", len(F_arr))
        else:
            print("  > Running as single custom filename...")
    create_file(F_arr, write_file_dir_name_prefix, write_file_name, random_ordering, verbose=verbose, f_complete=f_complete, create_sub_map=create_sub_map)
    return len(F_arr)

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath", default="formulas.json", help="Specify forumla file")
    parser.add_argument("-l", "--formula-list", default="default", help="Specify formula list inside json formula file")
    parser.add_argument("--formulas", default=None, action="extend", nargs="+", type=str, help="Manually specify multiple formula strings")
    parser.add_argument("-d", "--dfa-filename", default=None, help="Specify custom dfa filename")
    parser.add_argument("--dfa-path", default="dfas/", type=str, help="Specify custom path for dfa files")
    parser.add_argument("-c", "--complete", action='store_true', default=False, help="DFA is complete (instead of minimal)")
    parser.add_argument("-x", "--use-txt", action='store_true', default=False, help="Use '.txt' interpretation instead of '.json'")
    parser.add_argument("-r", "--random-ordering", action='store_true', default=False, help="Randomly the order of the input formulas")
    parser.add_argument("--sub-map", action='store_true', default=False, help="Create a submap template")
    args = parser.parse_args()
    if args.formulas:
        print(args.formulas)

    if not args.dfa_path.endswith("/"):
        WRITE_FILE_DIR_NAME_PREFIX = args.dfa_path + "/"
    else:
        WRITE_FILE_DIR_NAME_PREFIX = args.dfa_path
    WRITE_FILE_NAME = args.dfa_filename
    if args.formulas:
        print("Argument formulas: ", args.formulas)
        print("using submap: ", args.sub_map)
        create_file(args.formulas, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=False, verbose=True, f_complete=args.complete, create_sub_map=args.sub_map)
    else:
        print("Reading file: ", args.filepath)
        if args.filepath is not None:
            print("DFA file target: ", args.dfa_filename)
        if args.dfa_filename is not None and args.random_ordering:
            raise Exception("Cannot use 'random_ordering' with a 'dfa_filename'")

        READ_FILE_NAME = args.filepath
        if not args.use_txt:
            read_write_json(READ_FILE_NAME, args.formula_list, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=args.random_ordering, verbose=True, f_complete=args.complete, create_sub_map=args.sub_map)
        else: 
            read_write_txt(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=args.random_ordering, verbose=True, f_complete=args.complete)
