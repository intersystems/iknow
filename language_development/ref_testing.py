# -*- coding: utf-8 -*-
"""
Created on Tue Nov 17 17:14:05 2020

@author: ISC-SDE
"""

import os

import iknowpy
import difflib    # see https://docs.python.org/3/library/difflib.html#module-difflib


def write_ln(file_,text_):
    file_.write((text_+"\r\n").encode('utf8'))


def genRAW_for_reference_testing(txt_file, in_path_par, out_path_par):   # generates RAW output for a txt (UTF-8) input file
    print('Processing ' + elt)
#    filename = elt.rstrip('txt')   # note: rstrip('.txt') can remove too much, e.g. text.txt -> te
    filename_raw = elt.replace('.txt', '.raw')   # note: rstrip('.txt') can remove too much, e.g. text.txt -> te
    language_par = elt[0:2]
    print('language: ' + language_par)
    f_raw = open(os.path.join(output_path, filename_raw), 'wb')
    f_raw.write(b'\xef\xbb\xbf') # Utf8 BOM
    write_ln(f_raw,'#')
    write_ln(f_raw,"# in_path_par:"+in_path_par)
    write_ln(f_raw,"# out_path_par:"+out_path_par)
    write_ln(f_raw,"# language_par:"+language_par)
    write_ln(f_raw,"#")
    write_ln(f_raw,'\nD\x01'+elt+'\x01'+in_path_par+'/'+elt)
    
    f_text = open(os.path.join(input_path, elt), "rb")
    header = f_text.read(3)
    if (header == b'\xef\xbb\xbf'): #Utf8 BOM
        header = b''    # remove BOM
    text = header + f_text.read() # read text, must be utf8 encoded
    text = text.decode('utf8') # decode text to Unicode
    f_text.close()
    
    engine.index(text, language_par)
    for sent in engine.m_index['sentences']: # S志望学部の決定時期について経営学部生に関しては表7 （ a ） （ b ）に、経済学部生に関しては表8 （ a ） （ b ）に示す。
        #
        # reconstruct sentence literal
        #
        sentence_raw = 'S\x01'
        ent_stop = ''
        if language_par == 'ja':  # Separate procedure for Japanese: make sure double-byte spaces are kept. 
            for entity in sent['entities']:
                ent_type = entity['type']
                lit_text = text[entity['offset_start']:entity['offset_stop']].replace("\n"," ") # literal representation of sentence, with newlines removed
                while "  " in lit_text:   # get rid of double spaces, caused by replaced newlines
                    lit_text = lit_text.replace("  ", " ")
                ent_start = entity['offset_start']
                if ent_type == 'NonRelevant':
                    if (ent_start != ent_stop):
                        sentence_raw = sentence_raw.rstrip(' ') + lit_text
                    else:
                        sentence_raw = sentence_raw.rstrip(' ') + lit_text
                    ent_stop = entity['offset_stop']
                if ent_type == 'Concept':
                    if(ent_start != ent_stop):
                        sentence_raw = sentence_raw + '\x02' + lit_text + '\x02'
                    else:
                        sentence_raw = sentence_raw.rstrip(' ') + '\x02' + lit_text + '\x02'
                    ent_stop = entity['offset_stop']
                if ent_type == 'Relation':
                    if(ent_start != ent_stop):
                        sentence_raw = sentence_raw + '\x03' + lit_text + '\x03'
                    else:
                        sentence_raw = sentence_raw.rstrip(' ') + '\x03' + lit_text + '\x03'
                    ent_stop = entity['offset_stop']
                if ent_type == 'PathRelevant':
                    if(ent_start != ent_stop):
                        sentence_raw = sentence_raw + (' ' if entity == sent['entities'][0] else '') + '<' + lit_text + '>'
                    else:
                        sentence_raw = sentence_raw.rstrip(' ') + (' ' if entity == sent['entities'][0] else '') + '<' + lit_text + '>'
                    ent_stop = entity['offset_stop']

                if entity != sent['entities'][len(sent['entities'])-1]: # not for the last one
                    sentence_raw = sentence_raw + ' '

        else:
            for entity in sent['entities']:
                ent_type = entity['type']
                lit_text = text[entity['offset_start']:entity['offset_stop']]
                ent_start = entity['offset_start']
                if ent_type == 'NonRelevant':
                    if (ent_start != ent_stop):
                        sentence_raw = sentence_raw + lit_text
                    else:
                        sentence_raw = sentence_raw.rstrip() + lit_text
                    ent_stop = entity['offset_stop']
                if ent_type == 'Concept':
                    if(ent_start != ent_stop):
                        sentence_raw = sentence_raw + '\x02' + lit_text + '\x02'
                    else:
                        sentence_raw = sentence_raw.rstrip() + '\x02' + lit_text + '\x02'
                    ent_stop = entity['offset_stop']
                if ent_type == 'Relation':
                    if(ent_start != ent_stop):
                        sentence_raw = sentence_raw + '\x03' + lit_text + '\x03'
                    else:
                        sentence_raw = sentence_raw.rstrip() + '\x03' + lit_text + '\x03'
                    ent_stop = entity['offset_stop']
                if ent_type == 'PathRelevant':
                    if(ent_start != ent_stop):
                        sentence_raw = sentence_raw + (' ' if entity == sent['entities'][0] else '') + '<' + lit_text + '>'
                    else:
                        sentence_raw = sentence_raw.rstrip() + (' ' if entity == sent['entities'][0] else '') + '<' + lit_text + '>'
                    ent_stop = entity['offset_stop']
    
                if entity != sent['entities'][len(sent['entities'])-1]: # not for the last one
                    sentence_raw = sentence_raw + ' '


        write_ln(f_raw, sentence_raw)
        #
        # sentence attributes
        #
        if (len(sent['sent_attributes'])):
            # print(sent['sent_attributes'])

            # <attr type="time" literal="合格後" token="合格後">
            for sent_attribute in sent['sent_attributes']:
                # print(sent_attribute)
                attr_name = sent_attribute['type'].lower()
                
                # issue #33 - for Japanese - Entity Vector emitted as an attribute ' entity_vector'
                if attr_name == 'entityvector':
                    entity_vec_raw = '<attr type=\"entity_vector\"'
                    for sent_index in sent_attribute['entity_vector']:
                        entity = sent['entities'][sent_index]
                        lit_text = text[entity['offset_start']:entity['offset_stop']]
                        entity_vec_raw = entity_vec_raw + ' \"' + lit_text + '\"'
                    entity_vec_raw = entity_vec_raw + '>'
                    write_ln(f_raw, entity_vec_raw)
                    continue
                
                
                attr_entity_literal = text[sent['entities'][sent_attribute['entity_ref']]['offset_start']:sent['entities'][sent_attribute['entity_ref']]['offset_stop']].replace("\n","") # corresponding entity index literal value, remove newlines

                if (attr_name == 'datetime'):
                    attr_name = 'time'
                sent_attribute_raw = '<attr type=\"' + attr_name + '\" literal=\"' + attr_entity_literal + '\" token=\"' + sent_attribute['marker'].lstrip() + '\"'
                if attr_name == 'certainty':
                    if len(sent_attribute['parameters']):
                        value = sent_attribute['parameters'][0][0] # the certainty level is the first parameter in list of pairs, hence [0][0]
                        if value:
                            sent_attribute_raw = sent_attribute_raw + ' level=\"' + value + '\"'
                else:
                    # cnt_value_unit_pairs = len(sent_attribute['parameters'])
                    cnt_value_unit_pairs = 0
                    for valunit_parameter in sent_attribute['parameters']:
                        cnt_string = ''
                        if cnt_value_unit_pairs:
                            cnt_string = str(cnt_value_unit_pairs + 1)
                        cnt_value_unit_pairs = cnt_value_unit_pairs + 1

                        # value/unit pairs
                        value = valunit_parameter[0]
                        unit = valunit_parameter[1]
                        if value:
                            sent_attribute_raw = sent_attribute_raw + ' value' + cnt_string + '=\"' + value + '\"'
                        if unit:
                            sent_attribute_raw = sent_attribute_raw + ' unit' + cnt_string + '=\"' + unit + '\"'

                sent_attribute_raw = sent_attribute_raw + '>'
                # print(sent_attribute_raw)
                write_ln(f_raw, sent_attribute_raw)

        #
        #
        # write entities
        #
        for entity in sent['entities']:
            if entity['type'] == 'Concept': # C2only sound
                write_ln(f_raw, 'C\x01' + str(entity['index'].count(' ')+1) + '\x01' + entity['index'])
            if entity['type'] == 'Relation':
                write_ln(f_raw, 'R\x01' + entity['index'])
            if entity['type'] == 'PathRelevant': # PR<we>
                write_ln(f_raw, 'PR<' + entity['index'] + '>')
        #
        # write path
        #
        if (len(sent['path'])):
            path_raw = 'P\x01'
            for sent_index in sent['path']:
                path_raw = path_raw + sent['entities'][sent_index]['index'] + ' '

            write_ln(f_raw, path_raw.rstrip())
        #
        # path attributes
        # <attr type="negation" span="no sign here of hustle and bustle of french alpine tourist honeypots">
        #
        if (len(sent['path_attributes'])):
            for path_attribute in sent['path_attributes']:
                attr_name = path_attribute['type']
                if (attr_name == "DateTime"):
                    attr_name = "time"
                if (attr_name == "Measurement"):
                    attr_name = "measurement"
                if (attr_name == "Negation"):
                    attr_name = "negation"
                if (attr_name == "Certainty"):
                    attr_name = "certainty"

                start_position = path_attribute['pos']
                attribute_span = path_attribute['span']
                attr_path = sent['path'][start_position:start_position+attribute_span]
                path_attribute_raw = '<attr type=\"' + attr_name + '\" span=\"'

                for sent_index in attr_path:
                    path_attribute_raw = path_attribute_raw + sent['entities'][sent_index]['index'] + ' '

                path_attribute_raw = path_attribute_raw.strip() + '\">'
                write_ln(f_raw,path_attribute_raw)

    f_raw.close()


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


def f_compare(file1, file2, pair_report, language_par = 'xx'):
    """Compares 2 RAW files and writes reports (1 per file pair + 1 with a
    summary for all compared files).

    Return True if no difference was found, False if a difference was found."""
    compfilename = os.path.basename(file1.name)

    text1 = file1.readlines()
    text2 = file2.readlines()
    outputfile = open(pair_report, 'wb')
    filecounter1, filecounter2, linecounter, qmcounter, qmcounter2, diffcounter = 0, 0, 0, 0, 0, 0
    sentence_diff, index_diff, attr_cert_diff, attr_neg_diff, attr_possent_diff, attr_negsent_diff = 0, 0, 0, 0, 0, 0
    attr_time_diff, attr_freq_diff, attr_dur_diff, attr_meas_diff, attr_cert_diff, attr_ev_diff, undef_diff = 0, 0, 0, 0, 0, 0, 0
    
    for line in text1:
        if line[0] == 'D':
            filecounter1 +=1
    for line in text2:
        if line[0] == 'D':
            filecounter2 +=1
    
    d = difflib.Differ()
    
    result = list(d.compare(text1, text2))
    result_diff = []
    
    for elt in result:
        #    print(linecounter)
        if elt[2] =='#':
            continue  # skip header of file
        elif elt[0] == '-' or elt[0] == '+':  # - means the line is unique to input 1, + means the line is unique to input 2
            write_ln(outputfile,elt)
            result_diff.append(elt)
            linecounter +=1
        elif elt[0] == '?' and linecounter > 0:  # characters not present in either input, skip header of file
            if language_par != 'ja':
                write_ln(outputfile,elt)
            if '^' in elt:
                qmcounter2 +=1
            else:
                qmcounter +=1
    
    for elt in result_diff:   # scroll through differences and count them per category
        if elt[2] == 'S':
            sentence_diff +=1
        elif elt[2] == 'C' or elt[2]  == 'R':
            index_diff +=1
        elif elt[2:4] == 'PR':
            index_diff +=1
        elif 'type=\"certainty\"' in elt:
            attr_cert_diff +=1
        elif 'type=\"negation\"' in elt:
            attr_neg_diff +=1
        elif 'type=\"sentpositive\"' in elt:
            attr_possent_diff +=1
        elif 'type=\"sentnegative\"' in elt:
            attr_negsent_diff +=1
        elif 'type=\"time\"' in elt:
            attr_time_diff +=1
        elif 'type=\"frequency\"' in elt:
            attr_freq_diff +=1
        elif 'type=\"duration\"' in elt:
            attr_dur_diff +=1
        elif 'type=\"measurement\"' in elt:
            attr_meas_diff +=1
        elif 'type=\"entity_vector\"' in elt:
            attr_ev_diff +=1
        else:
            undef_diff +=1

    # Write summary
    if linecounter == 0:
        print('No changes are found in ' + compfilename + '.')
        write_ln(report, '\nNo changes are found in ' + compfilename + '.')
    else:
        write_ln(report, '\nSummary for the comparison of reference and new_output for ' + compfilename + ':')
        write_ln(outputfile,'\nSUMMARY:')
        if filecounter1 != filecounter2:
            print('\nWARNING: The RAW files contain output for a different number of input files!')
            write_ln(outputfile,'WARNING: The RAW files contain output for a different number of input files!')
            write_ln(report,'WARNING: The RAW files contain output for a different number of input files!')

        diffcounter = int(linecounter - qmcounter - (qmcounter2 / 2))
        print(str(linecounter) + ' distinct line(s) reported, corresponding to approximately ' + str(diffcounter) + ' difference(s).')
        pair_report_path = str(outputfile)[25:-1]
        pair_report_path = pair_report_path.replace('\\\\','/')
        print('See ' + pair_report_path + ' for details.')

        write_ln(outputfile,str(linecounter) + ' lines don\'t have a(n exact) copy in the other input file, of which')
        write_ln(report,str(linecounter) + ' lines don\'t have a(n exact) copy in the other input file, of which')
        if sentence_diff > 0:
            write_ln(outputfile,str(sentence_diff) + ' concern the sentence level (sentence boundaries or indexes),')
            write_ln(report,str(sentence_diff) + ' concern the sentence level (sentence boundaries or indexes),')
        if index_diff > 0:
            write_ln(outputfile,str(index_diff) + ' concern indexes of Concepts, Relations and PathRelevant entities,')
            write_ln(report,str(index_diff) + ' concern indexes of Concepts, Relations and PathRelevant entities,')
        if attr_neg_diff > 0:
            write_ln(outputfile,str(attr_neg_diff) + ' concern negation attributes,')
            write_ln(report,str(attr_neg_diff) + ' concern negation attributes,')
        if attr_possent_diff > 0:
            write_ln(outputfile,str(attr_possent_diff) + ' concern positive sentiment attributes,')
            write_ln(report,str(attr_possent_diff) + ' concern positive sentiment attributes,')
        if attr_negsent_diff > 0:
            write_ln(outputfile,str(attr_negsent_diff) + ' concern negative sentiment attributes,')
            write_ln(report,str(attr_negsent_diff) + ' concern negative sentiment attributes,')
        if attr_time_diff > 0:
            write_ln(outputfile,str(attr_time_diff) + ' concern time attributes,')
            write_ln(report,str(attr_time_diff) + ' concern time attributes,')
        if attr_freq_diff > 0:
            write_ln(outputfile,str(attr_freq_diff) + ' concern frequency attributes,')
            write_ln(report,str(attr_freq_diff) + ' concern frequency attributes,')
        if attr_dur_diff > 0:
            write_ln(outputfile,str(attr_dur_diff) + ' concern duration attributes,')
            write_ln(report,str(attr_dur_diff) + ' concern duration attributes,')
        if attr_meas_diff > 0:
            write_ln(outputfile,str(attr_meas_diff) + ' concern measurement attributes,')
            write_ln(report,str(attr_meas_diff) + ' concern measurement attributes,')
        if attr_cert_diff > 0:
            write_ln(outputfile,str(attr_cert_diff) + ' concern certainty attributes,')
            write_ln(report,str(attr_cert_diff) + ' concern certainty attributes,')
        if attr_ev_diff > 0:
            write_ln(outputfile,str(attr_ev_diff) + ' concern entity vectors,')
            write_ln(report,str(attr_ev_diff) + ' concern entity vectors,')
        if undef_diff > 0:
            write_ln(outputfile,str(undef_diff) + ' are not further specified.')
            write_ln(report,str(undef_diff) + ' are not further specified.')

    outputfile.close()
    return linecounter == 0


test_passed = True

# ---------------------------------------------------------------------------------------------------------
# Process input in ../reference_materials/input/core and ..\reference_materials/input/test.
# Generate RAW output in ../reference_materials/new_output/core and ../reference_MAterials/new_output/test.
# ---------------------------------------------------------------------------------------------------------
print('\nGENERATING RAW OUTPUT')

engine = iknowpy.iKnowEngine()

input_path = '../reference_materials/input/core'
output_path = '../reference_materials/new_output/core'
if os.path.exists(output_path):
    pass
else:
    os.makedirs(output_path)
input_core_list = os.listdir(input_path)
for elt in input_core_list:
    genRAW_for_reference_testing(elt, input_path, output_path)
    
input_path = '../reference_materials/input/test'
output_path = '../reference_materials/new_output/test'
if os.path.exists(output_path):
    pass
else:
    os.makedirs(output_path)
input_test_list = os.listdir(input_path)
for elt in input_test_list:
    genRAW_for_reference_testing(elt, input_path, output_path)


input_path = '../reference_materials/input/udct_test'
output_path = '../reference_materials/new_output/udct_test'
if os.path.exists(output_path):
    pass
else:
    os.makedirs(output_path)
input_udct_list = os.listdir(input_path)
for elt in input_udct_list:
    dct_name = elt[0:3] + 'udct.txt'
#    dct_to_use = open(os.path.join('../reference_materials/udct_test_dictionaries', dct_name), encoding='utf-8')
    dct_to_use = os.path.join('../reference_materials/udct_test_dictionaries', dct_name)
    user_dictionary = iknowpy.UserDictionary()
    read_udct_file(dct_to_use, user_dictionary)
    engine.load_user_dictionary(user_dictionary)
    genRAW_for_reference_testing(elt, input_path, output_path)
    engine.unload_user_dictionary()


# ---------------------------------------------------------------------------------------------------------
# Prepare comparison of RAW files: check whether all RAW files in input have a counterpart in new_output.
# Throw a warning for files without a counterpart.
# ---------------------------------------------------------------------------------------------------------
print('\nPREPARING COMPARISON OF RAW FILES')

if os.path.isfile('../reference_materials/reports/report.log'):
    os.remove('../reference_materials/reports/report.log')  # remove existing overall report
report = open('../reference_materials/reports/report.log', 'ab')    # open new overall report


ref_core_list = os.listdir('../reference_materials/reference/core')
new_core_list = os.listdir('../reference_materials/new_output/core')
ref_core_list.sort()
new_core_list.sort()
if ref_core_list != new_core_list:
    ref_core_list_for_comparison = []
    for elt in ref_core_list:
        if not elt in new_core_list:
            print('\nWARNING: ' + elt + ' is not found in new_output and will be excluded.')
            write_ln(report, 'WARNING: ' + elt + ' was not found in new_output and has been excluded.')
        else:
            ref_core_list_for_comparison.append(elt)
    for elt in new_core_list:
        if not elt in ref_core_list:
            print('\nWARNING: ' + elt + ' is not found in reference and will be excluded.')
            write_ln(report, 'WARNING: ' + elt + ' was not found in reference and has been excluded.')
else:
    ref_core_list_for_comparison = ref_core_list

ref_test_list = os.listdir('../reference_materials/reference/test')
new_test_list = os.listdir('../reference_materials/new_output/test')
ref_test_list.sort()
new_test_list.sort()
if ref_test_list != new_test_list:
    ref_test_list_for_comparison = []
    for elt in ref_test_list:
        if not elt in new_test_list:
            print('\nWARNING: ' + elt + ' is not found in new_output and will be excluded.')
            write_ln(report, 'WARNING: ' + elt + ' was not found in new_output and has been excluded.')
        else:
            ref_test_list_for_comparison.append(elt)
    for elt in new_test_list:
        if not elt in ref_test_list:
            print('\nWARNING: ' + elt + ' is not found in reference and will be excluded.')
            write_ln(report, 'WARNING: ' + elt + ' was not found in reference and has been excluded.')
else:
    ref_test_list_for_comparison = ref_test_list

ref_udct_list = os.listdir('../reference_materials/reference/udct_test')
new_udct_list = os.listdir('../reference_materials/new_output/udct_test')
ref_udct_list.sort()
new_udct_list.sort()
if ref_udct_list != new_udct_list:
    ref_udct_list_for_comparison = []
    for elt in ref_udct_list:
        if not elt in new_udct_list:
            print('\nWARNING: ' + elt + ' is not found in new_output and will be excluded.')
            write_ln(report, 'WARNING: ' + elt + ' was not found in new_output and has been excluded.')
        else:
            ref_udct_list_for_comparison.append(elt)
    for elt in new_udct_list:
        if not elt in ref_udct_list:
            print('\nWARNING: ' + elt + ' is not found in reference and will be excluded.')
            write_ln(report, 'WARNING: ' + elt + ' was not found in reference and has been excluded.')
else:
    ref_udct_list_for_comparison = ref_udct_list


# ---------------------------------------------------------------------------------------------------------
# Perform comparison of RAW files.
# ---------------------------------------------------------------------------------------------------------
print('\nCOMPARING RAW FILES')

for elt in ref_core_list_for_comparison:
    raw1 = open(os.path.join('../reference_materials/reference/core', elt), encoding='utf-8')
    raw2 = open(os.path.join('../reference_materials/new_output/core', elt), encoding='utf-8')
    report_name = elt[0:3] + 'core_report.txt'
    comp_report = os.path.join('../reference_materials/reports/core', report_name)
    print('\nComparing ' + elt)
    if elt[0:2] =='ja':
        # bitwise and avoids short-circuit evaluation
        test_passed = test_passed & f_compare(raw1, raw2, comp_report, 'ja')
    else:
        test_passed = test_passed & f_compare(raw1, raw2, comp_report)
    raw1.close()
    raw2.close()


for elt in ref_test_list_for_comparison:
    raw1 = open(os.path.join('../reference_materials/reference/test', elt), encoding='utf-8')
    raw2 = open(os.path.join('../reference_materials/new_output/test', elt), encoding='utf-8')
    report_name = elt[0:3] + 'test_report.txt'
    comp_report = os.path.join('../reference_materials/reports/test', report_name)
    print('\nComparing ' + elt)
    if elt[0:2] == 'ja':
        test_passed = test_passed & f_compare(raw1, raw2, comp_report, 'ja')
    else:
        test_passed = test_passed & f_compare(raw1, raw2, comp_report)
    raw1.close()
    raw2.close()


for elt in ref_udct_list_for_comparison:
    raw1 = open(os.path.join('../reference_materials/reference/udct_test', elt), encoding='utf-8')
    raw2 = open(os.path.join('../reference_materials/new_output/udct_test', elt), encoding='utf-8')
    report_name = elt[0:3] + 'udct_test_report.txt'
    comp_report = os.path.join('../reference_materials/reports/udct_test', report_name)
    print('\nComparing ' + elt)
    if elt[0:2] == 'ja':
        test_passed = test_passed & f_compare(raw1, raw2, comp_report, 'ja')
    else:
        test_passed = test_passed & f_compare(raw1, raw2, comp_report)
    raw1.close()
    raw2.close()

report.close()
print('\nPlease check the overall report: ../reference_materials/reports/report.log.')

if 'GITHUB_ENV' in os.environ:
    # if we're running in a GitHub Actions workflow, output the test result
    print(f'::set-output name=REF_TESTING_PASSED::{int(test_passed)}')
