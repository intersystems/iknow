# -*- coding: utf-8 -*-
"""
Created on Mon Feb  8 16:48:22 2021
@author: sdebergh

# This Python file uses the following encoding: utf-8

    find_examples_for_rule.py is a tool to find sentences in which a given rule is applied. 
    Usage: "python find_examples_for_rule.py <text files directory> <output file> <language> <rule number>"
    Example (on Windows): "python find_examples_for_rule.py C:\TextCorpus\English\Financial\ C:\output\ en 531"
                          -> find examples for rule 531 of the English language model
"""

import sys, os

# do "pip install iknowpy" if iknowpy is not installed
import iknowpy

# read command line
in_path_par = sys.argv[1]
out_path_par = sys.argv[2]
language_par = sys.argv[3]
rule_number = sys.argv[4]


# initiate variables  
mapping_file = language_par + "_compiler_report.log" # detect applicable xx_compiler_report.log based on language code
mapping_table = {}   
f_rec = []
engine = iknowpy.iKnowEngine()
 

print('Looking for examples for rule ' + rule_number + ' of the ' + language_par + ' language model in ' + in_path_par)


# functions
# add a line in the output file
def write_ln(file_,text_):
    file_.write((text_+"\r\n").encode('utf8'))

# create a mapping table for rule numbers, based on xx_compiler_report.log
def create_mapping_table(mapping_file):
    read_mapping_file = open(mapping_file, encoding='utf-8')
    for line in read_mapping_file:
        if line != '\n':
            mapping = line.split()[0]
        if ':' in mapping:
            mapping_table[mapping.split(':')[0]] = mapping.split(':')[1]

# find the matching number in the mapping table
def extract_rule_id(rule_order):
    rule_id = mapping_table[rule_order]
    return rule_id



# make a list of input file (recursive list of files, .txt only) - copied from https://stackoverflow.com/questions/18394147/recursive-sub-folder-search-and-return-files-in-a-list-python
f_rec = [os.path.join(dp, f) for dp, dn, filenames in os.walk(in_path_par) for f in filenames if
                  os.path.splitext(f)[1].lower() == '.txt']


# create mapping table for rule numbers
create_mapping_table(mapping_file)


# open output file and add UTF-8 BOM and information about the content of the file
if os.path.exists(out_path_par):
        os.remove(out_path_par)
f_output = open(out_path_par, "ab")
f_output.write(b'\xef\xbb\xbf') # Utf8 BOM
write_ln(f_output, 'Examples for rule ' + rule_number + ' of the ' + language_par + ' language model in ' + in_path_par + '\n')

# read input files one by one
for text_file in f_rec:
    print('processing ' + text_file)
    f_text = open(text_file, "rb")
    header = f_text.read(3)
    if (header == b'\xef\xbb\xbf'): # check for Utf8 BOM
        header = b''    # remove BOM
    text = header + f_text.read() # read text, must be utf8 encoded
    text = text.decode('utf8') # decode text to Unicode
    f_text.close()

    # index input file
    engine.index(text, language_par, traces=True)

    # read trace output
    for trace in engine.m_traces:
#        print(trace)
        key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
        # store the sentence
        if (key == "SentenceFound"):
            Sentence = value.split('"')[7]
        # check if the demanded rule is applied to process the sentence    
        elif (key == "RuleApplication"):
            # rule_id in trace refers actually to rule order -> retrieve rule order value
            rule_order = value.split(';')[0].split('=')[1]
            # extract the number that corresponds to the rule id in rules.csv from compiler_report.log
            rule_id = extract_rule_id(rule_order)
            # if the rule id corresponds to the demanded rule number, look for the concerned lexreps 
            if rule_id == rule_number:
                lexreps = value.split(';')[3:]
                lexreps = str(lexreps)
                lexreps_indexes = ''
                while 'index=' in lexreps:
                    lexreps_indexes = lexreps_indexes + ' ' + lexreps[lexreps.find('index=\"')+7:lexreps.find('labels=')-2]
                    lexreps = lexreps[lexreps.find('labels=')+7:]  # cut off left part of lexreps information in order to julp to the next lexrep
                # add the concerned lexrep(s) and the sentence to the output
                #print(lexreps_indexes.lstrip())
                write_ln(f_output, lexreps_indexes.lstrip() + ';' + Sentence)


f_output.close()