import random
import json
import os
import argparse

class GenerateRandomFormulas:
    ap_delimeter = str()
    formula_template = list() # Defines the structure of the input formula
    formulas = dict() # Holds all of the generated formulas for lookup or write to json
    existing_filepath = None

    def __init__(self, formula_template_str, existing_filepath_ = None, ap_delimeter_ = "$"):
        self.ap_delimeter = ap_delimeter_
        self.formula_template = formula_template_str.split(ap_delimeter_)
        self.existing_filepath = existing_filepath_
        if existing_filepath_:
            with open(existing_filepath_, 'w+') as file:
                if not os.stat(existing_filepath_).st_size == 0:
                    self.formulas = json.load(file)
        else:
            self.formulas.clear()

    def generateGridWorld(self, formula_list_key, N_formulas, x_lower, x_upper, y_lower, y_upper):
        formula_list = []
        for i in range(0, N_formulas):
            ap_list = list()
            for j in range(0, len(self.formula_template)):
                x = random.randrange(x_lower, x_upper)
                y = random.randrange(y_lower, y_upper)
                ap_list.append("ap_x{}_y{}".format(x, y))
            f_i = self.formula_template[0]
            for j in range(1, len(self.formula_template)):
                f_i += str(ap_list[j]) + self.formula_template[j]

            formula_list.append(f_i)
        self.formulas[formula_list_key] = formula_list
        

    def getFormulaList(self):
        return self.formulas

    def writeFormulasToJson(self, filepath = None):
        if self.existing_filepath and filepath:
            print("Warning (writeFormulasToJson): Existing filepath specified in __init__, ignoring given filepath...")
        if not self.existing_filepath and not filepath:
            print("Error (writeFormulasToJson): Must specify filepath.")
        if self.existing_filepath:
            filepath = self.existing_filepath
        with open(filepath, 'w+') as file:
            json.dump(self.formulas, file, indent=4)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--filepath", default="grid_size_bm_formulas.json", help="Specify forumla file")
    parser.add_argument("-s", "--grid-size", default=10, type=int, help="Square grid AP's from '0' to 'grid_size'")
    parser.add_argument("-N", "--num-formulas", default=10, type=int, help="Specify number for random formulas")
    parser.add_argument("-F", "--formula-template", default="F($)", type=str, help="Specify formula structure")
    parser.add_argument("-l", "--formula-list", default="formulas", help="Specify formula list name")
    parser.add_argument("-v", "--verbose", action='store_true', default=False, help="Display generated formulas")
    args = parser.parse_args()

    # Example use case of class:
    gen = GenerateRandomFormulas(args.formula_template, args.filepath)
    gen.generateGridWorld(args.formula_list, args.num_formulas, 0, args.grid_size, 0, args.grid_size)
    if args.verbose: 
        print(gen.getFormulaList())
    gen.writeFormulasToJson()


            