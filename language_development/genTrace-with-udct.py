# This Python file uses the following encoding: utf-8
''' genTrace-with-udct.py tool for logging linguistic traces when a user dictionary is in use.
    It uses xx_compiler_report.log (generated during compilation in language_development directory). 
    Usage: python genTrace-with-udct.py <text files directory> <output directory> <language> <user dictionary path>
    Example (on Windows): python genTrace-with-udct.py C:\TextCorpus\English\Financial\ C:\tmp\ en C:\iKnow\reference_materials\\udct_test_dictionaries\en_udct.txt
'''

import sys, os

# do "pip install iknowpy" if iknowpy is not installed
import iknowpy

in_path_par = "C:/tmp/text_input_data/"
out_path_par = "C:/tmp/output/"
language_par = "en"
user_dct_par = "../reference_materials/udct_test_dictionaries/en_udct.txt"
OldStyle = True

if (len(sys.argv)>1):
    in_path_par = sys.argv[1]
if (len(sys.argv)>2):
    out_path_par = sys.argv[2]
if (len(sys.argv)>3):
    language_par = sys.argv[3]
if (len(sys.argv)>4):
    user_dct_par = sys.argv[4]

mapping_file = language_par + "_compiler_report.log"  
mapping_table = {}   
attribute_list = ['UDNegation', 'UDTime', 'UDCertainty', 'UDValue', 'UDUnit', 'UDGeneric1', 'UDGeneric2', 'UDGeneric3', 'UDPosSentiment', 'UDNegSentiment', 'UDNumber', 'UDConcept', 'UDRelation', 'UDNonRelevant'] 

print('genTrace input_dir=\"'+in_path_par+'\" output_dir=\"'+out_path_par+'\" language=\"'+language_par + '\" user dictionary=\"' + user_dct_par + '\"')

def write_ln(file_,text_):
    file_.write((text_+"\r\n").encode('utf8'))

def create_mapping_table(mapping_file):
    read_mapping_file = open(mapping_file, encoding='utf-8')
    for line in read_mapping_file:
        if line != '\n':
            mapping = line.split()[0]
        if ':' in mapping:
            mapping_table[mapping.split(':')[0]] = mapping.split(':')[1]

def extract_rule_id(rule_order):
    rule_id = mapping_table[rule_order]
    return rule_id

def read_udct_file(file_,udct_):
    f_udct = open(file_,"r",True,"utf8")
    for txt_line in f_udct:
        # print('txt_line: ' + txt_line)
        txt_line = txt_line.rstrip()

        if ',' in txt_line and txt_line[0:2] != '/*':
            txt_list = txt_line.split(',')
            lexrep, action = txt_list[0], txt_list[1]
            if (lexrep[0] == '@'):
                literal = lexrep[1:]
                if action == "UDCertainty":
                    level = txt_list[2]
                    udct_.add_certainty_level(literal,int(level[2]))
                else:
                    ret = udct_.add_label(literal,action)
                    if (ret == -2):
                        print('label ' + action + ' not valid !')
            else: # Set end = $SELECT(command = "\end":1,command = "\noend":0,1:..Err())
                if action == "\\end":
                    udct_.add_sent_end_condition(lexrep, True)
                elif action == "\\noend":
                    udct_.add_sent_end_condition(lexrep, False)
                else:
                    print('action ' + action + ' not valid !')

    f_udct.close()


#
# collect text documents in 'in_path_par'
#
from os import walk

f = []  # non-recursive list of files, .txt only
for (dirpath, dirnames, filenames) in walk(in_path_par):
     for single_file in filenames:
         if (single_file.endswith('.txt')):
             f.append(single_file)
#         break

create_mapping_table(mapping_file)
# print('mapping table ready\n')

engine = iknowpy.iKnowEngine()

# load user dictionary
user_dictionary = iknowpy.UserDictionary()
read_udct_file(user_dct_par, user_dictionary)
ret = engine.load_user_dictionary(user_dictionary)

for text_file in f:
    print('processing ' + text_file)
    f_text = open(in_path_par+text_file, "rb")
    header = f_text.read(3)
    if (header == b'\xef\xbb\xbf'): #Utf8 BOM
        header = b''    # remove BOM
    text = header + f_text.read() # read text, must be utf8 encoded
    text = text.decode('utf8') # decode text to Unicode
    f_text.close()

    engine.index(text, language_par, traces=True)
    output_file = os.path.join(out_path_par, text_file) + '.log'
    if os.path.exists(output_file): # delete existing output file to make sure the new output file gets a new timestamp
        os.remove(output_file)
    f_trace = open(output_file, 'wb')
    f_trace.write(b'\xef\xbb\xbf') # Utf8 BOM
    for trace in engine.m_traces:
        #print(trace)
        key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
        if (key=='LexrepCreated'):
            Literal = value.split('"')[1]
            write_ln(f_trace, 'Literal:'+Literal)
#            pass
        elif (key == 'NormalizeToken'):
            write_ln(f_trace, key+":"+value)
#            pass
        elif key == 'UserDictionaryMatch':
            for elt in attribute_list:
                if elt in value:
                    write_ln(f_trace, key+":"+value)
            #pass
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
            # extract the number that corresponds to the rule id in rules.csv from compiler_report.log
            rule_id = extract_rule_id(rule_order)
            first_semicolon = value.index(';')
            updated_value = 'rule_id=' + rule_id + value[first_semicolon:]
            write_ln(f_trace, key + ':' + updated_value)
#            pass
        elif (key == "RuleApplicationResult"):  # use this only when the code for 'RuleApplication' is activated too
            write_ln(f_trace, key + ':' + updated_value)
            # pass
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

    user_dictionary.clear()
    f_trace.close()