# This Python file uses the following encoding: utf-8

''' handleCSV.py : generic script for automating CSV file operations.
    - task1 : filter Literal labels from lexrep.csv
'''

import os,sys,time,shutil,pprint
from os import walk
from shutil import copyfile
pp = pprint.PrettyPrinter()

language_par='*' # '*' # default to all languages
if (len(sys.argv)>1):
    language_par = sys.argv[1]

def filter_doubles_from_lexreps(full_path_name):
    print("filtering doubles from \"" + full_path_name + "\"")
    lexreps_csv = [] # filtered collection
    lexreps_map = {} # dictionary to filter doubles
    f_lexreps = open(full_path_name,"r",True,"utf8")
    for txt_line in f_lexreps:
        if (txt_line.find("/*") != -1) or (len(txt_line.split(';'))<4): # comment or empty
            lexreps_csv.append(txt_line)
            continue

        line_split = txt_line.split(';')
        lexrep = line_split[2]

        if lexrep in lexreps_map: # this is a double
            print("found double : " + txt_line)
            print("*** removed from lexreps collection ***")
        else:
            lexreps_map[lexrep]=txt_line
            lexreps_csv.append(txt_line)

    f_lexreps.close()

    f_lexreps = open(full_path_name,"w",True,"utf8")
    for txt_line in lexreps_csv:
        f_lexreps.write(txt_line)
    f_lexreps.close()

    pass

def filter_literal_label_from_lexreps(full_path_name):
    print("filtering: \"" + full_path_name + "\"")
    lexreps_csv = [] # filtered collection
    
    copyfile(full_path_name, full_path_name+".bak") # one backup file should do, use local git for original file
    f_lexreps = open(full_path_name,"r",True,"utf8")
    for txt_line in f_lexreps:
        if txt_line.find("/**** Rewritten by DELVE") != -1:
            line_split = txt_line.split(';')
            meta = line_split[1]
            lexrep = line_split[2]
            line_reconstruct = ";" + meta + ";" + lexrep + ";;"
            for i in range(4,len(line_split),1):
                if (line_split[i]=="\n"):
                    continue
                if (len(line_split[i])): # skip in the (exception) case of emptiness
                    line_reconstruct += line_split[i] + ";"
            line_reconstruct += "\n"
            lexreps_csv.append(line_reconstruct) # add modified line
            # pp.pprint(line_split)

            txt_lit = f_lexreps.readline() # line containing literal, exclude !
            if txt_lit.find("Lit_") == -1:
                raise ValueError('Lexrep line {} must have a literal label.'.format(txt_lit))

        else:
            lexreps_csv.append(txt_line)

    f_lexreps.close
    # pp.pprint(lexreps_csv)

    f_lexreps = open(full_path_name,"w",True,"utf8")
    for txt_line in lexreps_csv:
        f_lexreps.write(txt_line)
    f_lexreps.close()
    pass

def task1(language,path):
    print("Processing language=\"" + language + "\" in path=\"" + path + "\"\n")

    for (dirpath, dirnames, filenames) in walk(path):
        for single_file in filenames:
            if (single_file.endswith('.csv')):
                if single_file == "lexreps.csv":
                    filter_literal_label_from_lexreps(dirpath + single_file)
                    # filter_doubles_from_lexreps(dirpath + single_file)
    pass

def main():
    currentDirectory = os.getcwd() # Get Current working Directory
    
    if currentDirectory.find("language_development") == -1: # run from top "iknow" directory
        os.chdir(currentDirectory+'/language_models')
    else: # run from "language_development" directory
        os.chdir(currentDirectory+'/../language_models') # Change the Current working Directory to the language_models
    
    currentDirectory = os.getcwd()
    
    for (dirpath, dirnames, filenames) in walk(currentDirectory):
        if language_par == '*': # process them all
            for single_dir in dirnames:
                full_dir = dirpath + "/" + single_dir + "/"
                task1(single_dir, full_dir)
        else:
            full_dir = dirpath + "/" + language_par + "/"
            task1(language_par, full_dir)
            break
        
if __name__ == '__main__':
    main()