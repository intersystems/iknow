# This Python file uses the following encoding: utf-8
''' verify_rules.py tool for checking if all examples in rules.csv are present and valid
    It uses xx_compiler_report.log (generated during compilation in language_development directory). 

    Run this program from the language_development directory.
    Usage: "python verify_rules.py <language> <output_file>"
    Example (on Windows): "python verify_rules.py en C:\iKnow\output\en_verify_rules.txt"

    WARNING: The path to the directory with language models is hardcoded. 

    KNOWN ISSUE: words with straight, double quotation marks (") are not shown in the example sentence in the output of this script.
                 This has to do with the way the trace output is parsed.
'''

import sys, os

# do "pip install iknowpy" if iknowpy is not installed
import iknowpy

# defaults
out_path_par = "C:/tmp/output/verify_rules_output.txt"
language_par = "en"

if (len(sys.argv)>1):
    language_par = sys.argv[1]
if (len(sys.argv)>2):
     out_path_par = sys.argv[2]

rules_file = os.path.join("../language_models", language_par, "rules.csv")
mapping_file = language_par + "_compiler_report.log"
compiler_report_location = "../language_development"
mapping_file = os.path.join(compiler_report_location, mapping_file)   
mapping_table = {}    
#output_file = os.path.join(out_path_par, text_file) + '.log'
output_file = open(out_path_par, 'w', encoding='utf-8')
udct_file = language_par + '_udct.txt'
user_dct_par = os.path.join("../reference_materials/udct_test_dictionaries", udct_file)

sentence_complete_counter = 0
active_rule = ''
rule_number = ''

print("Verify rules in " + rules_file)

# functions
# #########

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

def check_example(traces, rule_application_list, ud_rule, rule_text, sentence_complete_counter, rule_fires):
    for trace in traces:
        key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
        if (key == "RuleApplication"):
            # rule_id in trace refers actually to rule order -> retrieve rule order value
            rule_order = value.split(';')[0].split('=')[1]
            # extract the number that corresponds to the rule id in rules.csv from compiler_report.log
            rule_id = extract_rule_id(rule_order)
            rule_application_list.append(rule_id)

            if rule_id == rule_number:
                rule_fires = 'yes'
            
        elif (key == "SentenceComplete"):
            sentence_complete_counter +=1
            sentence_text_list = []
            sentence_text_list = trace.split('value=\"')
            #print(sentence_text_list)
            sentence_text = ''
            for elt in sentence_text_list[1:]:
                sentence_text = sentence_text + ' ' + elt.split('\"')[0]

            if sentence_complete_counter == 1: # first example
                if rule_fires == 'yes':
                    output_file.write('Rule ' + str(rule_number) + ' is applied on the example.\n')
                else:
                    print('WRONG EXAMPLE for rule ' + str(rule_number) + ud_rule + ': ' + sentence_text)
                    output_file.write('WRONG EXAMPLE for rule ' + str(rule_number) + ud_rule + ': ' + str(sentence_text) + '\n')
                rule_fires = 'no'
                
            else:  # more than one example for a rule
                if rule_fires == 'yes':
                    output_file.write('Rule ' + str(rule_number) +  ' is applied on the example.\n') 
                else:
                    print('CHECK NON-FIRST EXAMPLE for rule ' + str(rule_number) + ud_rule + ': ' + sentence_text)
                    output_file.write('CHECK EXAMPLE for rule ' + str(rule_number) + ud_rule + ': ' + sentence_text + '\n')
                rule_fires = 'no'
            rule_application_list = []


# program
# #######

# create a table that matches rule order numbers (in trace output) with rule ID numbers (in rules.csv)
create_mapping_table(mapping_file)

# activate the iKnow engine
engine = iknowpy.iKnowEngine()

# open rules file for reading
rules_text = open(rules_file, 'r', encoding='utf-8')

# read the rules file line by line
for line in rules_text:
    rule_fires = 'no'   # reset variable
    if line.startswith('/*\t\t'):      # check if line starts with /*^t^t (= line with example)
        line_text = str(line[4:])
        rule_application_list = []
        if language_par == 'de':
            rule_number_field = line_text.split(';')[0]   # detect the number preceding the example sentence in German
        else:
            rule_number_field = line_text.split('.')[0]   # detect the number preceding the example sentence
        rule_number = ''
        for c in rule_number_field:
            if c.isdigit():
                rule_number = rule_number + c

        # detect text of examples in different cases
        if language_par == 'de':
            # case 1: examples for UD-related rules in German
            if rule_number_field.endswith('UD'):
                ud_rule = ' (UD-related)'

            # case 2: examples for not UD-related rules in German
            else:
                ud_rule = ' (not UD-related)'
            rule_text = line_text.split(';',1)[1]

        else:
            # case 3: examples for not UD-related rules in other language
            if line_text.split('.')[1] != 'UD':
                ud_rule = ' (not UD-related)'
                rule_text = line_text.split('.',1)[1]

            # case 4: examples for UD-related rules (starting with /* ^t ^t number.UD.) in other languages
            else:
                ud_rule = ' (UD-related)'
                rule_text = line_text.split('.',2)[2]


        rule_text = rule_text.lstrip('\\t')

        # check examples that are not UD-related
        if ud_rule == ' (not UD-related)':
                # process input
                engine.index(rule_text, language_par, traces=True)
                traces = engine.m_traces

                check_example(traces, rule_application_list, ud_rule, rule_text, sentence_complete_counter, rule_fires)

        # check UD-related examples
        else:
                # load user dictionary
                user_dictionary = iknowpy.UserDictionary()
                read_udct_file(user_dct_par, user_dictionary)
                ret = engine.load_user_dictionary(user_dictionary)

                # process input
                engine.index(rule_text, language_par, traces=True)
                traces = engine.m_traces

                check_example(traces, rule_application_list, ud_rule, rule_text, sentence_complete_counter, rule_fires)

                engine.unload_user_dictionary()
    else:
        try:  # detect lines with a rule
            temp_var = line.split(';')[3]
            #print('active rule: ' + active_rule)
            #print('example rule: ' + rule_number)
            if rule_number != active_rule:  # detect mismatch between last-found rule and last-found example
                print('No example found for rule ' + str(active_rule))
                output_file.write('No example found for rule ' + str(active_rule) + '.\n')
            if line[0].isdigit() == True:  # ignore lines that accidently match the pattern, e.g. rules that are commented out
                active_rule = line.split(';')[0]   # store the new rule number
        except IndexError:
            continue

if rule_number != active_rule:   # check if there is an example for the last rule
    print('No example found for rule ' + str(active_rule))
    output_file.write('No example found for rule ' + str(active_rule) + '.\n')


output_file.close()

