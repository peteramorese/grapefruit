#!/usr/bin/env python
from posixpath import dirname
import spot, os, glob, random
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
            #print("state {}".format(s))
            lines_list.append("- {}".format(s))
            for s_con in A.out(s):
                #print("  edge({} -> {})".format(s_con.src, s_con.dst))
                #print("    label: ", spot.bdd_format_formula(bdict, s_con.cond))
                #print("    accepting sets: ", s_con.acc)
                temp_str = "   - {} -> {}".format(s_con.src, s_con.dst)
                temp_str = temp_str + " : " + spot.bdd_format_formula(bdict, s_con.cond)
                lines_list.append(temp_str)
                if "{}".format(s_con.acc) != "{}":
                    #temp_str_2 = "{}".format(s_con.acc)
                    #print(" temp str 2 before: ", temp_str_2)
                    #temp_str_2 = temp_str_2.replace("{","")	
                    #temp_str_2 = temp_str_2.replace("}","")	
                    #print(" temp str 2 after: ", temp_str_2)
                    #accepting_list.append(temp_str_2)
                    accepting_list.append("{}".format(s))
        lines_list.append("Accepting States:")	
        for a_s in accepting_list:
            lines_list.append("- " + a_s)
        lines_list.append("")
        with open(filename, "w") as file:
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


def read_write(read_file_name, write_file_dir_name_prefix, write_file_name, random_ordering, verbose=False, f_complete=False):
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

if __name__ == "__main__":
    parser =  argparse.ArgumentParser()
    parser.add_argument("-p", "--filepath", default="formulas.txt", help="Specify forumla.txt file")
    parser.add_argument("-d", "--dfa_filename", default=None, help="Specify custom dfa filename")
    parser.add_argument("-c", "--complete", action='store_true', default=False, help="DFA is complete (instead of minimal)")
    parser.add_argument("--formulas", action="extend", nargs="+", type=str, help="DFA is complete (instead of minimal)")
    args = parser.parse_args()
    if args.formulas:
        print(args.formulas)


    print("Reading file: ", args.filepath)
    if args.filepath is not None:
        print("DFA file target: ", args.dfa_filename)
    READ_FILE_NAME = args.filepath
    WRITE_FILE_DIR_NAME_PREFIX = "dfas/"
    WRITE_FILE_NAME = args.dfa_filename
    if args.formulas:
        create_file(args.formulas, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=False, verbose=True, f_complete=args.complete)
    else:
        read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX, WRITE_FILE_NAME, random_ordering=False, verbose=True, f_complete=args.complete)
