# This Python file uses the following encoding: utf-8
''' genTrace.py tool for logging linguistic traces
    Usage: "python genTrace.py <text files directory> <output directory> <language>"
    Example (on Windows): "python genTrace.py(r'C:\TextCorpus\English\Financial\',r'C:\tmp\','en')
'''

import sys
sys.path.insert(0, 'C:/Users/jdenys/source/repos/iknow/kit/x64/Release/bin')
import engine as iknowpy

import os
import pprint
import time
#import iknowpy

# print(sys.argv)

in_path_par = "C:\\P4\\Users\\jdenys\\text_input_data\\en\\" # ja/"
out_path_par = "C:/tmp/"
language_par = "en"
OldStyle = True

if (len(sys.argv)>1):
    in_path_par = sys.argv[1]
if (len(sys.argv)>2):
    out_path_par = sys.argv[2]
if (len(sys.argv)>3):
    language_par = sys.argv[3]

print('genTrace input_dir=\"'+in_path_par+'\" output_dir=\"'+out_path_par+'\" language=\"'+language_par+'\"')

def write_ln(file_,text_):
    file_.write((text_+"\r\n").encode('utf8'))

#
# collect text documents in 'in_path_par'
#
from os import walk

f = []
for (dirpath, dirnames, filenames) in walk(in_path_par):
    f.extend(filenames)
    break

engine = iknowpy.iKnowEngine()

for text_file in f:
    print(text_file)
    
    f_text = open(in_path_par+text_file, "rb")
    header = f_text.read(3)
    if (header == b'\xef\xbb\xbf'): #Utf8 BOM
        header = b''    # remove BOM
    text = header + f_text.read() # read text, must be utf8 encoded
    text = text.decode('utf8') # decode text to Unicode
    f_text.close()

    engine.index(text, language_par, traces=True)

    f_trace = open(out_path_par + text_file + ".log", "wb")
    f_trace.write(b'\xef\xbb\xbf') # Utf8 BOM
    for trace in engine.m_traces:
	    #print(trace)
        key, value = trace.split(':')[0],trace.split(':')[1]
        if (key=='LexrepCreated'):
		    # print(value)
            Literal = value.split('"')[1]
            print('Literal=',Literal)
        elif (key == 'NormalizeToken'):
            pass
        elif (key == "SentenceFound"):
            Sentence = value.split('"')[7]
            print('Sentence=',Sentence)
        elif (key == "LexrepIdentified"):
            Index, Labels = value.split('"')[3],value.split('"')[5]
            if (Index=='B'): # B&E are Begin&End markers, not real lexreps
                continue
            if (Index=='E'):
                continue
            print(Index, ":", Labels)
        elif (key == "RuleApplication"):
            pass
        elif (key == "RuleApplicationResult"):
            pass
        elif (key == "JoinResult"):
            pass
        elif (key == "RulesComplete"):
            pass
        elif (key == "AmbiguityResolved"):
            pass
        elif (key == "MergingConcept"):
            pass
        elif (key == "MergedConcept"):
            pass
        elif (key == "MergingRelation"):
            pass
        elif (key == "MergedRelation"):
            pass
        elif (key == "MergedRelationNonrelevant"):
            pass
        elif (key == "SentenceComplete"):
            pass
        elif (key == "EntityVector"):
            pass
        elif (key == "MergedKatakana"):
            pass
        elif (key == "LabelKatakana"):
            pass
        elif (key == "TraceTime"):
            pass
        else:
            print(key)

    f_trace.close()