# This Python file uses the following encoding: utf-8
''' genTrace.py tool for logging linguistic traces
    It uses /kit/x64/release/bin/compiler_report.log. Make sure to edit line 26 of the script
    to comply with your local situation.
    Usage: "python genTrace.py <text files directory> <output directory> <language>"
    Example (on Windows): "python genTrace.py C:\TextCorpus\English\Financial\ C:\tmp\ en
'''

import sys

# do "pip install iknowpy" if iknowpy is not installed
import iknowpy

in_path_par = "C:/tmp/text_input_data/"
out_path_par = "C:/tmp/output/"
language_par = "en"
OldStyle = True
mapping_file = language_par + "_compiler_report.log"   
mapping_table = {}    

if (len(sys.argv)>1):
    in_path_par = sys.argv[1]
if (len(sys.argv)>2):
    out_path_par = sys.argv[2]
if (len(sys.argv)>3):
    language_par = sys.argv[3]

print('genTrace input_dir=\"'+in_path_par+'\" output_dir=\"'+out_path_par+'\" language=\"'+language_par+'\"')

def write_ln(file_,text_):
    file_.write((text_+"\r\n").encode('utf8'))

def create_mapping_tabel(mapping_file):
    read_mapping_file = open(mapping_file, 'r')
    for line in read_mapping_file:
        if line != '\n':
            mapping = line.split()[0]
        if ':' in mapping:
            mapping_table[mapping.split(':')[0]] = mapping.split(':')[1]

def extract_rule_id(rule_order):
    rule_id = mapping_table[rule_order]
    return rule_id


#
# collect text documents in 'in_path_par'
#
from os import walk

f = []
for (dirpath, dirnames, filenames) in walk(in_path_par):
    f.extend(filenames)
    break

create_mapping_tabel(mapping_file)
# print('mapping table ready\n')

engine = iknowpy.iKnowEngine()

for text_file in f:
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
#        print(trace)
        key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
        if (key=='LexrepCreated'):
            Literal = value.split('"')[1]
            write_ln(f_trace, 'Literal:'+Literal)
#            pass
        elif (key == 'NormalizeToken'):
            write_ln(f_trace, key+":"+value)
#            pass
        elif (key == 'AttributeDetected'):
            attribute_type = value.split(';')[0]
            Literal = value.split('"')[1]
            write_ln(f_trace, key+":"+attribute_type+";"+Literal)
#            pass
        elif (key == 'PreprocessToken'):
            write_ln(f_trace, key+":"+value)
#            pass
        elif (key == "SentenceFound"):
            Sentence = value.split('"')[7]
            write_ln(f_trace, 'Sentence='+Sentence)
        elif (key == "LexrepIdentified"):
            Index, Labels = trace.split('"')[3],trace.split('"')[5]
            if (Index=='B'): # B&E are Begin&End markers, not real lexreps
                continue
            if (Index=='E'):
                continue
            write_ln(f_trace, "LexrepIdentified:"+Index+":"+Labels)
        elif (key == "RuleApplication"):
            # rule_id in trace refers actually to rule order -> retrieve rule order value
            rule_order = value.split(';')[0].split('=')[1]
            # extract the number that cooresponds to the rule id in rules.csv from compiler_report.log
            rule_id = extract_rule_id(rule_order)
            first_semicolon = value.index(';')
            updated_value = 'rule_id=' + rule_id + value[first_semicolon:]
            write_ln(f_trace, key + ':' + updated_value)
#            pass
        elif (key == "RuleApplicationResult"):
            write_ln(f_trace, key + ':' + value)
            pass
        elif (key == "JoinResult"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "RulesComplete"):
            pass
        elif (key == "AmbiguityResolved"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "ConceptFiltered"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "MergingConcept"):
            pass
        elif (key == "MergedConcept"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "MergingRelation"):
            pass
        elif (key == "MergedRelation"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "MergedRelationNonrelevant"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "SentenceComplete"):
            write_ln(f_trace, key + value + '\n')
#            pass
        elif (key == "EntityVector"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "MergedKatakana"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "LabelKatakana"):
            write_ln(f_trace, key + value)
#            pass
        elif (key == "InvalidEntityVector"):
            write_ln(f_trace, key + value)
        elif (key == "MissingEntityVector"):
            write_ln(f_trace, key + value)
        elif (key == "TraceTime"):
            ## Express time in milliseconds:
            milli_time = value.split(';')[1]
            write_ln(f_trace, key + ":" + milli_time + " ms")
            ## Express time in microseconds:
#            micro_time = value.split(';')[2]
#            write_ln(f_trace, key + ":" + micro_time + " µs")
#            pass
        else:
            print(key)

    f_trace.close()