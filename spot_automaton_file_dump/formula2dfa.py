#!/usr/bin/env python
from posixpath import dirname
import spot, os, glob, random

def remove_dfa_files(dirname_prefix):
    # Remove all dfa files in directory:
    for file in os.scandir(dirname_prefix):
        os.remove(file.path)
        #print(file.path)

def create_file(F_arr, dirname_prefix, random_ordering):
    remove_dfa_files(dirname_prefix)
    inds = [i for i in range(0, len(F_arr))]
    if random_ordering:
        random.shuffle(inds)
    i = 0
    for F in F_arr: 
        file_ind = inds[i]
        filename = dirname_prefix + "dfa_{}".format(file_ind) + ".txt"
        #filename_list[i] = filename
        i = i + 1
        lines_list = list()
        accepting_list = list()
        A = spot.formula(F).translate("BA","complete","deterministic","sbacc")
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


def read_write(read_file_name, write_file_dir_name_prefix, random_ordering):
    with open(read_file_name, "r") as formula_file:
        lines = formula_file.readlines()

    F_arr = list()
    for line in lines:
        F_i = line.replace("\n","")
        if not line == "\n": 
            if not F_i[0]=="#":
                print("Found formula:     ",F_i)
                F_arr.append(F_i)

    print("Number of formulas: ", len(F_arr))
    create_file(F_arr, write_file_dir_name_prefix, random_ordering)
    return len(F_arr)

if __name__ == "__main__":
    READ_FILE_NAME = "formulas.txt"
    WRITE_FILE_DIR_NAME_PREFIX = "dfas/"
    read_write(READ_FILE_NAME, WRITE_FILE_DIR_NAME_PREFIX)
