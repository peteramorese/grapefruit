#!/usr/bin/env python
import spot

def create_file(F_arr, filename_prefix):


    i = 0
    for F in F_arr: 
        filename = filename_prefix + "_{}".format(i) + ".txt"
        #filename_list[i] = filename
        i = i + 1;
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


READ_FILE_NAME = "formulas.txt"
WRITE_FILE_NAME_PREFIX = "dfas/dfa"

with open(READ_FILE_NAME, "r") as formula_file:
    lines = formula_file.readlines()

F_arr = list()
for line in lines:
    F_i = line.replace("\n","")
    print("Found formula:     ",F_i)
    F_arr.append(F_i)
create_file(F_arr, WRITE_FILE_NAME_PREFIX)
