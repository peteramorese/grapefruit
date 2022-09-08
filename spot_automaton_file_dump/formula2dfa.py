#! /root/miniconda3/envs/tpenv/bin/python
from posixpath import dirname
import spot, os, glob, random, json
import argparse

def remove_dfa_files(dirname_prefix):
    # Remove all dfa files in directory:
    for file in os.scandir(dirname_prefix):
        os.remove(file.path)
        #print(file.path)

def create_file(F_arr, dirname_prefix, custom_filename, random_ordering, verbose=False, f_complete=False):
    remove_dfa_files(dirname_prefix)
    inds = [i for i in range(0, len(F_arr))]
    if random_ordering:
        random.shuffle(inds)
    i = 0
    for F in F_arr: 
        file_ind = inds[i]
        if custom_filename is None:
            filename = dirname_prefix + "dfa_{}".format(file_ind) + ".txt"
            if verbose:
                print("Writing to: dfa_{}".format(file_ind))
        else:
            filename = dirname_prefix + custom_filename + ".txt"
        #filename_list[i] = filename
        i = i + 1
        lines_list = list()
        accepting_list = list()
        if f_complete:
            A = spot.formula(F).translate("BA","complete","deterministic","sbacc")
        else:
            A = spot.formula(F).translate("BA","deterministic","sbacc")
        bdict = A.get_dict()	
        #print("Acceptance: ", A.get_acceptance())
        #print("num states: ", A.num_states())
        lines_list.append("Alphabet: ")
        for ap in A.ap():
            #print(" ", ap)
            temp_str = "- {}".format(ap)
            lines_list.append(temp_str)
        lines_list.append("Initial States: ")
        lines_list.append("- {}".format(A.get_init_state_number()))
        lines_list.append("Graph:")
        for s in range(0, A.num_states()):
            lines_list.append("- {}".format(s))
            for s_con in A.out(s):
                temp_str = "   - {} -> {}".format(s_con.src, s_con.dst)
                temp_str = temp_str + " : " + spot.bdd_format_formula(bdict, s_con.cond)
                lines_list.append(temp_str)
                if "{}".format(s_con.acc) != "{}":
                    accepting_list.append("{}".format(s))
        lines_list.append("Accepting States:")	
        for a_s in accepting_list:
            lines_list.append("- " + a_s)
        lines_list.append("")
        with open(filename, "w+") as file:
            for line in lines_list:
                file.write(line)
                file.write("\n")

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
                    print("Found formula:     ",F_i)
                F_arr.append(F_i)
                if custom_single:
                    break

    if verbose: 
        if not custom_single:
            print("Number of formulas: ", len(F_arr))
        else:
            print("Running as single custom filename...")
    create_file(F_arr, write_file_dir_name_prefix, write_file_name, random_ordering, verbose=verbose, f_complete=f_complete)
    return len(F_arr)

def read_write_json(read_json_name, formula_list_name, write_file_dir_name_prefix, write_file_name=None, random_ordering=False, verbose=False, f_complete=False):
    with open(read_json_name) as f:
        formulas = json.load(f)

    custom_single = False
    if write_file_name is not None:
        custom_single = True
        
    F_arr = list()
    for F_i in formulas[formula_list_name]:
        if not F_i[0]=="#":
            if verbose:
                print("Found formula:     ",F_i)
            F_arr.append(F_i)
            if custom_single:
                break

    if verbose: 
        if not custom_single:
            print("Number of formulas: ", len(F_arr))
        else:
            print("Running as single custom filename...")
    create_file(F_arr, write_file_dir_name_prefix, write_file_name, random_ordering, verbose=verbose, f_complete=f_complete)
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
    args = parser.parse_args()

    if not args.dfa_path.endswith("/"):
        WRITE_FILE_DIR_NAME_PREFIX = args.dfa_path + "/"
    else:
        WRITE_FILE_DIR_NAME_PREFIX = args.dfa_path
    WRITE_FILE_NAME = args.dfa_filename
    if args.formulas:
        print("Argument formulas: ", args.formulas)
        create_file(args.formulas, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=False, verbose=True, f_complete=args.complete)
    else:
        print("Reading file: ", args.filepath)
        if args.filepath is not None:
            print("DFA file target: ", args.dfa_filename)
        if args.dfa_filename is not None and args.random_ordering:
            raise Exception("Cannot use 'random_ordering' with a 'dfa_filename'")

        READ_FILE_NAME = args.filepath
        if not args.use_txt:
            read_write_json(READ_FILE_NAME, args.formula_list, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=args.random_ordering, verbose=True, f_complete=args.complete)
        else: 
            read_write_txt(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=args.random_ordering, verbose=True, f_complete=args.complete)
