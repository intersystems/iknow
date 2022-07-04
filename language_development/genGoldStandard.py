# This Python file uses the following encoding: utf-8
''' genGoldStandard.py tool
        - to help create Gold Standard output
        - to help compare output with Gold Standard output
    Usage: "python genGoldStandard.py <input directory> <output directory> <language> <action> <type> <certainty> <measurement> <negation> <sentiment> <time> <generic> <entity vector> <certainty span> <measurement span> <negation span> <sentiment span> <time span> <generic span> 
    The parameters action, type, different attributes and different attribute expansions can be switched on or off:
    Remark: a subdirectory structure is automatically created in the main directory defined as input or output parameter (except for 'create' action, input and output directory are identical)
    Choose one of the following actions as action_par:
        -> write 'create' to create a new Gold Standard 
            * input: input parameter (texts to evaluate)
            * output:
                - subdirectory 'gold_standard_wip':
                    -> CSV output with sentence numbers for easy comparison with numbered sentences from XML output
                - subdirectory 'xml_output':
                    -> XML output for evaluation of output (REMARK: 'iKnowXML.xsl' style sheet should be in same directory as genGoldStandard.py)
        -> write 'finish' to create Gold Standard input and output file for comparison:
            * input: subdirectory 'gold_standard_ready' (contains manually corrected gold_standard_wip file(s)) 
            * output: 
                - subdirectory 'gold_input'
                    -> generate Gold Standard input (input sentences from 'gold_standard_ready', without 'Sentence [number];')
                - subdirectory 'gold_standard'
                    -> generate Gold Standard output ('gold_standard_ready' without 'Sentence [number];')
        -> write 'compare' to compare output with existing Gold Standard output:
            * input: subdirectory 'gold_input'
            * output: subdirectory 'current_output'
                -> generate CSV output without sentence numbers for easy comparison with Gold Standard sentences, which are not numbered 
    Choose which parts of the iKnow output you want to evaluate:        
    - type_par: 
        -> write parameter 'type' to output entity types
        -> write another parameter, e.g. 'no_type', if not
    - attribute parameters:
        -> certainty_par: write parameter 'cert' to output certainty attributes; write another parameter, e.g. 'no_cert', if not
        -> measurement_par: write parameter 'meas' to output measurement attributes; write another parameter, e.g. 'no_meas', if not
        -> negation_par: write parameter 'neg' to output negation attributes; write another parameter, e.g. 'no_neg', if not
        -> sentiment_par: write parameter 'sent' to output sentiment attributes; write another parameter, e.g. 'no_sent', if not
        -> time_par: write parameter 'time' to output time attributes; write another parameter, e.g. 'no_time', if not
        -> generic_par: write parameter 'gen' to output generic attributes; write another parameter, e.g. 'no_gen', if not
        -> entityvector_par: write parameter 'vec' to output entity vectors (for Japanese only); write another parameter, e.g. 'no_vec', if not
    - expansion parameters: 
        -> certaintyspan_par: write parameter 'certspan' to output certainty span; write another parameter, e.g. 'no_certspan', if not
        -> measurementspan_par: write parameter 'measspan' to output measurement span; write another parameter, e.g. 'no_measspan', if not
        -> negationspan_par: write parameter 'negspan' to output negation span; write another parameter, e.g. 'no_negspan', if not
        -> sentimentspan_par: write parameter 'sentspan' to output sentiment span; write another parameter, e.g. 'no_sentspan', if not
        -> timespan_par: write parameter 'timespan' to output time span; write another parameter, e.g. 'no_timespan', if not
        -> genericspan_par: write parameter 'genspan' to output generic attributes; write another parameter, e.g. 'no_genspan', if not
    Example (on Windows): "python genGoldStandard.py C:\gs_en_general\input\ C:\gs_en_general\ en create type cert meas neg sent time gen vec certspan measspan negspan sentspan timespan genspan"
'''
import sys, os, ntpath
import html
#
# run "pip install iknowpy" if iknowpy is not installed
#
import iknowpy
#
# command line parameters
#
in_path_par = "C:/tmp/text_input_data/"
out_path_par = "C:/tmp/output/"
language_par = "ja"
action_par = "create"
type_par = "type"
certainty_par = "cert"
measurement_par = "meas"
negation_par = "neg"
sentiment_par = "sent"
time_par = "time"
generic_par = "gen"
entityvector_par = "vec"
certaintyspan_par = "certspan"
measurementspan_par = "measspan"
negationspan_par = "negspan"
sentimentspan_par = "sentspan"
timespan_par = "timespan"
genericspan_par = "genspan"

if (len(sys.argv)>1):
    in_path_par = sys.argv[1]
if (len(sys.argv)>2):
    out_path_par = sys.argv[2]
if (len(sys.argv)>3):
    language_par = sys.argv[3]
if (len(sys.argv)>4):
    action_par = sys.argv[4]
if (len(sys.argv)>5):
    type_par = sys.argv[5]
if (len(sys.argv)>6):
    certainty_par = sys.argv[6]
if (len(sys.argv)>7):
    measurement_par = sys.argv[7]
if (len(sys.argv)>8):
    negation_par = sys.argv[8]
if (len(sys.argv)>9):
    sentiment_par = sys.argv[9]
if (len(sys.argv)>10):
    time_par = sys.argv[10]
if (len(sys.argv)>11):
    generic_par = sys.argv[11]
if (len(sys.argv)>12):
    entityvector_par = sys.argv[12]
if (len(sys.argv)>13):
    certaintyspan_par = sys.argv[13]
if (len(sys.argv)>14):
    measurementspan_par = sys.argv[14]
if (len(sys.argv)>15):
    negationspan_par = sys.argv[15]
if (len(sys.argv)>16):
    sentimentspan_par = sys.argv[16]
if (len(sys.argv)>17):
    timespan_par = sys.argv[17]
if (len(sys.argv)>18):
    genericspan_par = sys.argv[18]

print('genGoldStandard input_dir=\"'+in_path_par+'\" output_dir=\"'+out_path_par+'\" language=\"'+language_par+'\"'+ ' action=\"'+action_par+'\"'+' type=\"'+type_par+'\"'+' certainty=\"'+certainty_par+'\"'+' measurement=\"'+measurement_par+'\"'+' negation=\"'+negation_par+'\"'+' sentiment=\"'+sentiment_par+'\"'+' time=\"'+time_par+'\"'+' generic=\"'+generic_par+'\"'+' entity vector=\"'+entityvector_par+'\"'+' certainty span=\"'+certaintyspan_par+'\"'+' measurement span=\"'+measurementspan_par+'\"'+' negation span=\"'+negationspan_par+'\"'+' sentiment span=\"'+sentimentspan_par+'\"'+' time span=\"'+timespan_par+'\"'+' generic span=\"'+genericspan_par+'\"')

engine = iknowpy.iKnowEngine()
from os import walk

#
# function to write to output file
# 
def write_ln(file_,text_=""):
    file_.write((text_+"\r\n").encode('utf8'))

# function to read a complete text file
def read_text_file(file_name):
    with open(file_name, encoding="utf8") as f_text:
        return f_text.read()

# function to generate a list of single sentences from a text file
def generate_list_of_single_sentences(file_name,language_par):
    sentence_list = []
    #
    # reconstruct sentence literal from trace, except for Japanese : reconstruct with text offsets.
    #
    text = read_text_file(file_name)
    engine.index(text, language_par, traces=True)
    if language_par == "ja":
        for sent in engine.m_index['sentences']:
            cnt_entities = len(sent['entities']) # number of entities in the sentence
            sent_offset_start = sent['entities'][0]['offset_start']
            sent_offset_stop = sent['entities'][cnt_entities-1]['offset_stop']
            sentence_list.append(text[sent_offset_start:sent_offset_stop].replace("\n"," "))
    else:
        for trace in engine.m_traces:
            key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
                
            if (key == "SentenceFound"):
                sentence_composed = value.split('"')[7]
                if len(value.split('"')) > 9:  # i.e. if the sentence contains quotes (")
                    for i in range(8, len(value.split('"')) - 1):
                        sentence_composed = sentence_composed + '"' + value.split('"')[i]
                sentence_list.append(sentence_composed)

    return sentence_list

# write the sentence 'entity' types
def write_entity_types(sentence,file):
    for entity in sentence['entities']:
        if entity['type'] == 'Concept':
            file.write('C;' + entity['index'] + '\n')
        elif entity['type'] == 'Relation':
            file.write('R;' + entity['index'] + '\n')
        elif entity['type'] == 'PathRelevant':
            file.write('PR;' + entity['index'] + '\n')
        elif entity['type'] == 'NonRelevant':
            file.write('NR;' + entity['index'] + '\n')
        else:
            pass

# write the sentence 'certainty' attributes
def write_certainty_attributes(sentence,file):
    if (len(sentence['sent_attributes'])):
        certainty_attribute = ''

        for sent_attribute in sentence['sent_attributes']:
            if sent_attribute['type'] == 'Certainty':
                if len(sent_attribute['parameters']):
                    value = sent_attribute['parameters'][0][0] # the certainty level is the first parameter in list of pairs, hence [0][0]
                     # print('certainty level: ' + value)
                if value:
                    certainty_attribute = certainty_attribute + 'Cert;' + sent_attribute['marker'].lstrip() + ';level;' + value + '\n'
                else:
                    certainty_attribute = certainty_attribute + 'Cert;' + sent_attribute['marker'].lstrip() + '\n'

        if (certainty_attribute != ''):
            file.write(certainty_attribute)

# write the sentence 'measurement' attributes
def write_measurement_attributes(sentence,file):
    if (len(sentence['sent_attributes'])):
        measurement_attribute = ''

        for sent_attribute in sentence['sent_attributes']:
            if sent_attribute['type'] == 'Measurement':
                if (measurement_attribute == ''):
                    if len(sent_attribute['parameters']):
                        #print(sent_attribute['parameters'])
                        cnt_value_unit_pairs = 0
                        value_unit_pairs = ''
                        for valunit_parameter in sent_attribute['parameters']:
                            cnt_string = ''
                            if cnt_value_unit_pairs:
                                cnt_string = str(cnt_value_unit_pairs + 1)
                            cnt_value_unit_pairs = cnt_value_unit_pairs + 1
                            # value/unit pairs
                            value = valunit_parameter[0]
                            unit = valunit_parameter[1]
                            if value:
                                value_unit_pairs = value_unit_pairs + ';value' + cnt_string + ';' + value
                            if unit:
                                value_unit_pairs = value_unit_pairs + ';unit' + cnt_string + ';' + unit
                        measurement_attribute = measurement_attribute + 'Meas;' + sent_attribute['marker'].lstrip() + value_unit_pairs + '\n'
                else:
                    if len(sent_attribute['parameters']):
                        #print(sent_attribute['parameters'])
                        cnt_value_unit_pairs = 0
                        value_unit_pairs = ''
                        for valunit_parameter in sent_attribute['parameters']:
                            cnt_string = ''
                            if cnt_value_unit_pairs:
                                cnt_string = str(cnt_value_unit_pairs + 1)
                            cnt_value_unit_pairs = cnt_value_unit_pairs + 1
                            # value/unit pairs
                            value = valunit_parameter[0]
                            unit = valunit_parameter[1]
                            if value:
                                value_unit_pairs = value_unit_pairs + ';value' + cnt_string + ';' + value
                            if unit:
                                value_unit_pairs = value_unit_pairs + ';unit' + cnt_string + ';' + unit
                        measurement_attribute = measurement_attribute + 'Meas;' + sent_attribute['marker'].lstrip() + value_unit_pairs + '\n'

        if (measurement_attribute != ''):
            file.write(measurement_attribute)

# write the sentiment 'negation' atribute
def write_negation_attributes(sentence,file):
    if (len(sentence['sent_attributes'])):
        negation_attribute = ''

        for sent_attribute in sentence['sent_attributes']:
            if (sent_attribute['type'] == 'Negation'):
                negation_attribute = negation_attribute + 'Neg;' + sent_attribute['marker'].lstrip() + '\n'

            if (negation_attribute != ''):
                file.write(negation_attribute)

# write the sentence 'sentiment' attributes
def write_sentiment_attributes(sentence,file):
    if (len(sentence['sent_attributes'])):
        negsentiment_attribute = possentiment_attribute = ''

        for sent_attribute in sentence['sent_attributes']:
            if (sent_attribute['type'] == 'NegativeSentiment'):
                negsentiment_attribute = negsentiment_attribute + 'NegSent;' + sent_attribute['marker'].lstrip() + '\n'
            elif (sent_attribute['type'] == 'PositiveSentiment'):
                possentiment_attribute = possentiment_attribute + 'PosSent;' + sent_attribute['marker'].lstrip() + '\n'

        if (negsentiment_attribute != ''):
            file.write(negsentiment_attribute)
        if (possentiment_attribute != ''):
            file.write(possentiment_attribute)


# write the sentence 'time' attributes 
def write_time_attributes(sentence,file):
    if (len(sentence['sent_attributes'])):
        duration_attribute = frequency_attribute = time_attribute = ''

        for sent_attribute in sentence['sent_attributes']:
            if (sent_attribute['type'] == 'Duration'):
                duration_attribute = duration_attribute + 'Dur;' + sent_attribute['marker'].lstrip() + '\n'
            elif (sent_attribute['type'] == 'Frequency'):
                frequency_attribute = frequency_attribute + 'Freq;' + sent_attribute['marker'].lstrip() + '\n'
            elif (sent_attribute['type'] == 'DateTime'):
                time_attribute = time_attribute + 'Time;' + sent_attribute['marker'].lstrip() + '\n'

        if (duration_attribute != ''):
            file.write(duration_attribute)
        if (frequency_attribute != ''):
            file.write(frequency_attribute)
        if (time_attribute != ''):
            file.write(time_attribute)

# write the sentence 'generic' attributes
def write_generic_attributes(sentence,file):
    if (len(sentence['sent_attributes'])):
        generic1_attribute = generic2_attribute = generic3_attribute = ''

        for sent_attribute in sentence['sent_attributes']:
            if (sent_attribute['type'] == 'Generic1'):
                generic1_attribute = generic1_attribute + 'Gen1;' + sent_attribute['marker'].lstrip() + '\n'                                   
            elif (sent_attribute['type'] == 'Generic2'):
                generic2_attribute = generic2_attribute + 'Gen2;' + sent_attribute['marker'].lstrip() + '\n'
            elif (sent_attribute['type'] == 'Generic3'):
                generic3_attribute = generic3_attribute + 'Gen3;' + sent_attribute['marker'].lstrip() + '\n'

        if (generic1_attribute != ''):
            file.write(generic1_attribute)
        if (generic2_attribute != ''):
            file.write(generic2_attribute)
        if (generic3_attribute != ''):
            file.write(generic3_attribute)  

# write the sentence 'entity vector' attributes
def write_entity_vector_attributes(sentence,file,text):
    entity_vector_attribute = ''
    if (len(sentence['sent_attributes'])): 
        for sent_attribute in sentence['sent_attributes']:
            if sent_attribute['type'] == 'EntityVector':
                entity_vector_attribute = 'Vec; '
                for sent_index in sent_attribute['entity_vector']:                                    
                    entity = sent['entities'][sent_index]
                    lit_text = text[entity['offset_start']:entity['offset_stop']]
                    entity_vector_attribute = entity_vector_attribute + ' \"' + lit_text + '\"'

    if (entity_vector_attribute != ''):                
        file.write(entity_vector_attribute.rstrip() + '\n')  

# write the path 'certainty' span
def write_path_certainty_span(sentence,file):
    if (len(sentence['path_attributes'])):
        certainty_span = ''

        for path_attribute in sentence['path_attributes']:
            attr_name = path_attribute['type']
            start_position = path_attribute['pos']
            attribute_span = path_attribute['span']
            attr_path = sent['path'][start_position:start_position+attribute_span]
            if attr_name == 'Certainty':
                certainty_span = certainty_span + '\nCertSpan;'
                for sent_index in attr_path:
                    certainty_span = certainty_span + sent['entities'][sent_index]['index'].lstrip() + ' '                                        
                
        if (certainty_span != ''):
            certainty_span = certainty_span.replace(' \n','\n')
            file.write(certainty_span.strip() + '\n')

# write the path 'measurement' span
def write_path_measurement_span(sentence,file):
    if (len(sentence['path_attributes'])):
        measurement_span = ''

        for path_attribute in sentence['path_attributes']:
            attr_name = path_attribute['type']
            start_position = path_attribute['pos']
            attribute_span = path_attribute['span']
            attr_path = sent['path'][start_position:start_position+attribute_span]
            if attr_name == 'Measurement':
                measurement_span = measurement_span + '\nMeasSpan;'
                for sent_index in attr_path:
                    measurement_span = measurement_span + sent['entities'][sent_index]['index'].lstrip() + ' '

        if (measurement_span != ''):
            measurement_span = measurement_span.replace(' \n','\n')
            file.write(measurement_span.strip() + '\n')

# write the path 'negation' span
def write_path_negation_span(sentence,file):
    if (len(sentence['path_attributes'])):
        negation_span = ''

        for path_attribute in sentence['path_attributes']:
            attr_name = path_attribute['type']
            start_position = path_attribute['pos']
            attribute_span = path_attribute['span']
            attr_path = sent['path'][start_position:start_position+attribute_span]
            if attr_name == 'Negation':
                negation_span = negation_span + '\nNegSpan;'
                for sent_index in attr_path:
                    negation_span = negation_span + sent['entities'][sent_index]['index'].lstrip() + ' ' 

        if (negation_span != ''):
            negation_span = negation_span.replace(' \n','\n')
            file.write(negation_span.strip() + '\n')

# write the path 'sentiment' span
def write_path_sentiment_span(sentence,file):
    if (len(sentence['path_attributes'])):
        negsentiment_span = possentiment_span = ''

        for path_attribute in sentence['path_attributes']:
            attr_name = path_attribute['type']
            start_position = path_attribute['pos']
            attribute_span = path_attribute['span']
            attr_path = sent['path'][start_position:start_position+attribute_span]
            if attr_name == 'NegativeSentiment':
                negsentiment_span = negsentiment_span + '\nNegSentSpan;'
                for sent_index in attr_path:
                    negsentiment_span = negsentiment_span + sent['entities'][sent_index]['index'].lstrip() + ' '
            elif attr_name == 'PositiveSentiment':
                possentiment_span = possentiment_span + '\nPosSentSpan;'
                for sent_index in attr_path:
                    possentiment_span = possentiment_span + sent['entities'][sent_index]['index'].lstrip() + ' '

        if (negsentiment_span != ''):
            negsentiment_span = negsentiment_span.replace(' \n','\n')
            file.write(negsentiment_span.strip() + '\n')
        if (possentiment_span != ''):
            possentiment_span = possentiment_span.replace(' \n','\n')
            file.write(possentiment_span.strip() + '\n')

# write the path 'time' span
def write_path_time_span(sentence,file):
    if (len(sentence['path_attributes'])):
        duration_span = frequency_span = time_span = ''

        for path_attribute in sentence['path_attributes']:
            attr_name = path_attribute['type']
            start_position = path_attribute['pos']
            attribute_span = path_attribute['span']
            attr_path = sent['path'][start_position:start_position+attribute_span]
            if attr_name == 'Duration':
                duration_span = duration_span + '\nDurSpan;'
                for sent_index in attr_path:
                    duration_span = duration_span + sent['entities'][sent_index]['index'].lstrip() + ' '
            elif attr_name == 'Frequency':
                frequency_span = frequency_span + '\nFreqSpan;'
                for sent_index in attr_path:
                    frequency_span = frequency_span + sent['entities'][sent_index]['index'].lstrip() + ' '
            elif attr_name == 'DateTime':
                time_span = time_span + '\nTimeSpan;'
                for sent_index in attr_path:
                    time_span = time_span + sent['entities'][sent_index]['index'].lstrip() + ' '

        if (duration_span != ''):
            duration_span = duration_span.replace(' \n','\n')
            file.write(duration_span.strip() + '\n')
        if (frequency_span != ''):
            frequency_span = frequency_span.replace(' \n','\n')
            file.write(frequency_span.strip() + '\n')
        if (time_span != ''):
            time_span = time_span.replace(' \n','\n')
            file.write(time_span.strip() + '\n')

# write the path 'generic attribute' span
def write_path_generic_span(sentence,file):
    if (len(sentence['path_attributes'])):
        generic1_span = generic2_span = generic3_span = ''

        for path_attribute in sentence['path_attributes']:
            attr_name = path_attribute['type']
            start_position = path_attribute['pos']
            attribute_span = path_attribute['span']
            attr_path = sent['path'][start_position:start_position+attribute_span]
            if attr_name == 'Generic1':
                generic1_span = generic1_span + '\nGen1Span;'
                for sent_index in attr_path:
                    Gen1_span = Gen1_span + sent['entities'][sent_index]['index'].lstrip() + ' '
            elif attr_name == 'Generic2':
                generic2_span = generic2_span + '\nGen2Span;'
                for sent_index in attr_path:
                     Gen2_span = Gen2_span + sent['entities'][sent_index]['index'].lstrip() + ' '
            elif attr_name == 'Generic3':
                generic3_span = generic3_span + '\nGen3Span;'
                for sent_index in attr_path:
                    Gen3_span = Gen3_span + sent['entities'][sent_index]['index'].lstrip() + ' '

        if (generic1_span != ''):
            generic1_span = generic1_span.replace(' \n','\n')
            file.write_ln(generic1_span.strip() + '\n')
        if (generic2_span != ''):
            generic2_span = generic2_span.replace(' \n','\n')
            file.write(generic2_span.strip() + '\n')
        if (generic3_span != ''):
            generic3_span = generic3_span.replace(' \n','\n')
            file.write(generic3_span.strip() + '\n')

#
# collect text documents in one of both input directories (depending on action parameter)
#
#
# warning if input directory doesn't exist
#
if not os.path.exists(in_path_par):
    print("input directory doesn't exist")
    
# PART 1: GENERATE CSV OUTPUT
#
# First option: create Gold Standard (action_par = 'create')
#
f_input = []  # non-recursive list of files, .txt only
for (dirpath, dirnames, filenames) in walk(in_path_par):
    for single_file in filenames:
        if (single_file.endswith('.txt')):
            f_input.append(os.path.join(dirpath, single_file))
    break

if action_par == 'create':
    # create output directories
    out_path_ready = os.path.join(out_path_par, 'gold_standard_ready') # Copy WIP to ready after manual editing
    out_path_wip = os.path.join(out_path_par, 'gold_standard_wip') # Work In Progress in CSV format
    out_path_wip_xml = os.path.join(out_path_par, 'xml_output') # Work In Progress in XML format

    if not os.path.exists(out_path_ready):
        os.mkdir(out_path_ready)
    if not os.path.exists(out_path_wip):
        os.mkdir(out_path_wip)
    if not os.path.exists(out_path_wip_xml):
        os.mkdir(out_path_wip_xml)

    for text_file in f_input:
        #
        # check whether output file for input file already exists, otherwise: process input
        #
        output_file = ntpath.basename(text_file).removesuffix('.txt') + '.csv'   # use ntpath to ensure compatibility with Windows and Linux
        #output_file = ntpath.basename(text_file) + '.csv'   # use ntpath to ensure compatibility with Windows and Linux

        if os.path.exists(os.path.join(out_path_wip, output_file)): 
            print('output file for ' + ntpath.basename(text_file) + ' already exists and processing for this file was skipped')

        else:
            print('processing ' + ntpath.basename(text_file) + ' for CSV output')
            with open(os.path.join(out_path_wip, output_file), 'w', encoding='utf-8-sig') as f_wip:
                #
                # count sentences per file
                #
                sentence_order = 0
                sentence_list = generate_list_of_single_sentences(text_file,language_par)
                #
                # write sentence
                #       
                f_wip.write('\n') # start file with whiteline, otherwise first sentence will not be found with 'finish' option because of BOM
                for single_sentence in sentence_list:
                    engine.index(single_sentence, language_par)
                    for sent in engine.m_index['sentences']:
                        sentence_order +=1        
                        sentence = 'Sentence ' + str(sentence_order) + ';' + single_sentence
                        f_wip.write(sentence+'\n')
                        #
                        # write entities
                        #
                        if type_par == 'type':
                            write_entity_types(sent,f_wip)
                        #
                        # sentence attributes
                        #
                        f_wip.write('***Attributes***\n')

                        if certainty_par == 'cert':
                            write_certainty_attributes(sent,f_wip)
                        if measurement_par == 'meas':
                            write_measurement_attributes(sent,f_wip)
                        if negation_par == 'neg':
                            write_negation_attributes(sent,f_wip)
                        if sentiment_par == 'sent':
                            write_sentiment_attributes(sent,f_wip)
                        if time_par == 'time':
                            write_time_attributes(sent,f_wip)
                        if generic_par == 'gen':
                            write_generic_attributes(sent,f_wip)                   
                        if entityvector_par == 'vec':  # only for Japanese
                            write_entity_vector_attributes(sent,f_wip,single_sentence)
                        #
                        # path attributes
                        #
                        f_wip.write('***Expansion***\n')

                        if certaintyspan_par == 'certspan':
                            write_path_certainty_span(sent,f_wip)
                        if measurementspan_par == 'measspan':
                            write_path_measurement_span(sent,f_wip)
                        if negationspan_par == 'negspan':
                            write_path_negation_span(sent,f_wip)
                        if sentimentspan_par == 'sentspan':
                            write_path_sentiment_span(sent,f_wip)
                        if timespan_par == 'timespan':
                            write_path_time_span(sent,f_wip)
                        if genericspan_par == 'genspan':
                            write_path_generic_span(sent,f_wip)
                        f_wip.write('\n\n')
                    
#
# Second option: generate Gold Standard input and output files ready for use to compare output with (action_par = 'finish')
#
if action_par == 'finish':
    f_ready = []  # non-recursive list of files, .csv only
    in_path_ready = os.path.join(in_path_par, 'gold_standard_ready') # input file is 'gold standard ready' output
    for (dirpath, dirnames, filenames) in walk(in_path_ready):
        for single_file in filenames:
            if (single_file.endswith('.csv')):
                f_ready.append(os.path.join(dirpath, single_file))
        break

    print('input directory is ' + in_path_ready)
    #    
    # create output directories   
    #
    out_path_gold_input = os.path.join(in_path_par, 'gold_input') # sentence divised input text
    out_path_gold_standard = os.path.join(in_path_par, 'gold_standard') # per text verified csv output

    if not os.path.exists(out_path_gold_input):
        os.mkdir(out_path_gold_input)
    if not os.path.exists(out_path_gold_standard):
        os.mkdir(out_path_gold_standard)    

    print('output directory for gold standard input is ' + out_path_gold_input)
    print('output directory for gold standard output is ' + out_path_gold_standard)
    #
    # check whether Gold Standard input already exists, otherwise: generated it
    #
    for csv_file in f_ready:
        gold_input_file = ntpath.basename(csv_file).removesuffix('.csv') + '_gold_input.txt'   # use ntpath to ensure compatibility with Windows and Linux
        #gold_input_file = ntpath.basename(csv_file) + '_gold_input.txt'   # use ntpath to ensure compatibility with Windows and Linux
        if os.path.exists(os.path.join(out_path_gold_input, gold_input_file)): 
            print('gold standard input file for ' + ntpath.basename(csv_file) + ' already exists and processing for this file was skipped')

        else:
            print('processing ' + ntpath.basename(csv_file) + ' to extract gold standard input')
            read_file = open(csv_file, encoding='utf-8')
            f_gold_input = open(os.path.join(out_path_gold_input, gold_input_file), 'w', encoding='utf-8-sig')
            for line in read_file:
                if line.startswith('Sentence'):
                    sentence_text = line.split(';',1)[1] # split at first ';' and only keep second part (= strip 'Sentence [number];')
                    f_gold_input.write(sentence_text + '\n') # double newline forces end_of_sentence condition

            read_file.close()
            f_gold_input.close()
    #
    # check whether Gold Standard output already exists, otherwise: generated it
    #
    for csv_file in f_ready:
        gold_standard_file = ntpath.basename(csv_file).removesuffix('.csv') + '_gold_standard.csv'   # use ntpath to ensure compatibility with Windows and Linux
        #gold_standard_file = ntpath.basename(csv_file) + '_gold_standard.csv'   # use ntpath to ensure compatibility with Windows and Linux
        if os.path.exists(os.path.join(out_path_gold_standard, gold_standard_file)): 
            print('gold standard output file for ' + ntpath.basename(csv_file) + ' already exists and processing for this file was skipped')

        else:
            print('processing ' + ntpath.basename(csv_file) + ' to create gold standard output')
            read_file = open(csv_file, encoding='utf-8')
            f_gold_standard = open(os.path.join(out_path_gold_standard, gold_standard_file), 'w', encoding='utf-8')
            for line in read_file:
                if line.startswith('Sentence'):
                    sentence_text = line.split(';',1)[1] # split at first ';' and only keep second part (= strip 'Sentence [number];')
                    f_gold_standard.write(sentence_text)
                else:
                    f_gold_standard.write(line)
            read_file.close()
            f_gold_standard.close()    
#
# Third option: compare output with Gold Standard (action_par = 'compare')
#
in_path_gold_input = os.path.join(out_path_par, 'gold_input')
f_gold_input = []  # non-recursive list of files, .txt only
for (dirpath, dirnames, filenames) in walk(in_path_gold_input):
    for single_file_gs in filenames:
        if (single_file_gs.endswith('.txt')):
            f_gold_input.append(os.path.join(dirpath, single_file_gs))
    break   

if action_par == 'compare':
    for text_file in f_gold_input:
        text = read_text_file(text_file)
        #
        # create output directory   
        #
        out_path_current = os.path.join(out_path_par, 'current_output')
        if not os.path.exists(out_path_current):
            os.mkdir(out_path_current)
        
        #
        # check whether output file for input file already exists, otherwise: process input
        #
        output_file = ntpath.basename(text_file).removesuffix('_gold_input.txt') + '_current_output.csv'   # use ntpath to ensure compatibility with Windows and Linux
        #output_file = ntpath.basename(text_file) + '_current_output.csv'   # use ntpath to ensure compatibility with Windows and Linux

        if os.path.exists(os.path.join(out_path_current, output_file)): 
            print('output file for ' + ntpath.basename(text_file) + ' already exists and processing for this file was skipped')

        else:
            print('processing ' + ntpath.basename(text_file) + ' for CSV output')
            with open(os.path.join(out_path_current, output_file), 'w', encoding='utf-8-sig') as f_current:
                #
                # list sentences per file
                #
                sentence_list = generate_list_of_single_sentences(text_file,language_par)
                #
                # write sentence
                #       
                f_current.write('\n') # start file with whiteline to match output of 'create' function
                for single_sentence in sentence_list:
                    engine.index(single_sentence, language_par)
                    for sent in engine.m_index['sentences']:
                        f_current.write(single_sentence + '\n')
                        #
                        # write entities
                        #
                        if type_par == 'type':
                            write_entity_types(sent,f_current)
                        #
                        # sentence attributes
                        #
                        f_current.write('***Attributes***\n')

                        if certainty_par == 'cert':
                            write_certainty_attributes(sent,f_current)
                        if measurement_par == 'meas':
                            write_measurement_attributes(sent,f_current)
                        if negation_par == 'neg':
                            write_negation_attributes(sent,f_current)
                        if sentiment_par == 'sent':
                            write_sentiment_attributes(sent,f_current)
                        if time_par == 'time':
                            write_time_attributes(sent,f_current)
                        if generic_par == 'gen':
                            write_generic_attributes(sent,f_current)
                        if entityvector_par == 'vec':  # only for Japanese
                            write_entity_vector_attributes(sent,f_current,single_sentence)
                        #
                        # path attributes
                        #
                        f_current.write('***Expansion***\n')

                        if certaintyspan_par == 'certspan':
                            write_path_certainty_span(sent,f_current)
                        if measurementspan_par == 'measspan':
                            write_path_measurement_span(sent,f_current)
                        if negationspan_par == 'negspan':
                            write_path_negation_span(sent,f_current)
                        if sentimentspan_par == 'sentspan':
                            write_path_sentiment_span(sent,f_current)
                        if timespan_par == 'timespan':
                            write_path_time_span(sent,f_current)
                        if genericspan_par == 'genspan':
                            write_path_generic_span(sent,f_current)  
                        f_current.write('\n\n')

# PART 2: GENERATE XML OUTPUT (only with action 'create')
#
# required style sheet to visualise the XML
#
style_sheet_par = os.path.join(os.getcwd(), 'iKnowXML.xsl') # style sheet should be in same directory as genGoldStandard.py
#
# define variables
#
sentence_order = 0
split_sentence = []
lexrep_info = []
#
# process files one by one
#
if action_par == 'create':
    for text_file in f_input:
        print('processing ' + ntpath.basename(text_file) + ' for XML output')
        text = read_text_file(text_file)
        #
        # create output file, write header
        #
        filename_xml = ntpath.basename(text_file) + '.xml'   # use ntpath to ensure compatibility with Windows and Linux
        out_path_xml = os.path.join(out_path_par, 'xml_output')
        #
        # print(filename_xml)
        #
        f_xml = open(os.path.join(out_path_xml, filename_xml), 'wb')

        f_xml.write(b'\xef\xbb\xbf') # Utf8 BOM
        write_ln(f_xml,'<?xml version=\"1.0\" encoding=\"utf-8\"?>')
        write_ln(f_xml,'<?xml-stylesheet  type=\"text/xsl\" href=\"' + style_sheet_par + '\"?>')
        write_ln(f_xml,'<Content>')
        #
        # process text with iKnow
        #
        engine.index(text, language_par)
        #
        # translate engine output into xml
        #
        for sent in engine.m_index['sentences']:
            #        
            # write xml tag <Sentence> with xml attribute 'order'
            #
            sentence_order +=1
            write_ln(f_xml, '  <Sentence ' + 'order=\"' + str(sentence_order) +'\">')
            ent_stop = ''
            #
            # link entities with attributes, generate xml
            #
            for entity in sent['entities']:
                #print(entity)
                has_attr = False
                ent_type = entity['type']
                lit_text = html.escape(text[entity['offset_start']:entity['offset_stop']])
                index_text = html.escape(entity['index'])
                
                # check for attribute markers in order to mark them as such
                for attr_marker in sent['sent_attributes']:
                    attr_type = str(attr_marker['type']).lower()
                
                    # first ignore Japanese entity vectors in this stage: they must not be marked in the sentence
                    if attr_type == 'entityvector':
                        pass
                                
                    # then process 'real' attributes 
                    elif entity['offset_start'] <= attr_marker['offset_start'] and attr_marker['offset_stop'] <= entity['offset_stop']:
                        attr_type =str(attr_marker['type']).lower()
                        attr_type = attr_type.replace('datetime','time')
                        attr_type = attr_type.replace('positivesentiment','sentpositive')
                        attr_type = attr_type.replace('negativesentiment','sentnegative')
                        if attr_type == 'certainty':
                            attr_prop = ' level=\"' + attr_marker['parameters'][0][0] + '\"' # level is the first parameter of the first pair, hence [0][0]
                        else:
                            attr_prop = ''
                        #print(attr_type)
                        
                        if has_attr == False: # first attribute of the entity
                            # distinguish markers from the rest of the entity
                            attr_marker_start = attr_marker['offset_start']
                            attr_marker_stop = attr_marker['offset_stop']
                            lit_text = ''
                            if entity['offset_start'] < attr_marker_start: # part of entity preceding the marker
                                lit_text = '<reg>' + html.escape(text[entity['offset_start']:attr_marker['offset_start']]) + '</reg> '
                            lit_text = lit_text + '<' + attr_type + attr_prop + '>' + html.escape(text[attr_marker['offset_start']:attr_marker['offset_stop']]).lstrip() + '</' + attr_type + '>' # the marker itself
                            if entity['offset_stop'] > attr_marker_stop: # part of the entity following the marker
                                lit_text = lit_text + ' <reg>' + html.escape(text[attr_marker['offset_stop']:entity['offset_stop']]).lstrip() + '</reg>'
                            has_attr = True
                        else:   # 2nd, 3rd,... attribute of the entity
                            attr_marker_start = attr_marker['offset_start']
                            attr_marker_stop = attr_marker['offset_stop']
                            lit_text = lit_text + '</Literal>\n      <Literal>'
                            if entity['offset_start'] < attr_marker_start:
                                lit_text = lit_text + '<reg> // ' + html.escape(text[entity['offset_start']:attr_marker['offset_start']]) + '</reg> '
                            if entity['offset_start'] == attr_marker_start:
                                lit_text = lit_text + '<reg> // </reg>' + '<' + attr_type + attr_prop + '>' + html.escape(text[attr_marker['offset_start']:attr_marker['offset_stop']]).lstrip() + '</' + attr_type + '>'
                            else:
                                lit_text = lit_text + '<' + attr_type + attr_prop + '>' + html.escape(text[attr_marker['offset_start']:attr_marker['offset_stop']]).lstrip() + '</' + attr_type + '>'
                            if entity['offset_stop'] > attr_marker_stop:
                                lit_text = lit_text + ' <reg>' + html.escape(text[attr_marker['offset_stop']:entity['offset_stop']]).lstrip() + '</reg>'
                
                
                # write to output file
                write_ln(f_xml, '    <' + ent_type + '>')
                write_ln(f_xml, '      <Index>' + index_text + '</Index>')
                if has_attr == True and '//' in lit_text:
                    write_ln(f_xml, '      <Literal><reg>{</reg>' + lit_text + '<reg>}</reg></Literal>')
                elif has_attr == True:
                    write_ln(f_xml, '      <Literal>' + lit_text + '</Literal>')
                else:
                    write_ln(f_xml, '      <Literal>')
                    write_ln(f_xml, '        <reg>' + lit_text + '</reg>')
                    write_ln(f_xml, '      </Literal>')
                write_ln(f_xml, '    </' + ent_type + '>')
            

            # write entity vector (Japanese only)
            if language_par == 'ja' and len(sent['sent_attributes']) > 0:
                ev = sent['sent_attributes'][-1]   # EntityVector is the last attribute in sent_attributes
                if str(ev['type'] == 'EntityVector'):
                    ev_text = ''
                    write_ln(f_xml, '    <entity_vector>')
                    for sent_index in attr_marker['entity_vector']:
                        ev_text = ev_text + '      <ent>' + html.escape(sent['entities'][sent_index]['index']) + '</ent>\n'
                    write_ln(f_xml, ev_text + '    </entity_vector>')
                    #print(ev)


            # write path
            if len(sent['path']):
                path_text = ''
                write_ln(f_xml, '    <path>')
                for sent_index in sent['path']:
                        path_text = path_text + ' ' + sent['entities'][sent_index]['index']
                path_text = html.escape(path_text)                    
                write_ln(f_xml, '      <value>' + path_text.lstrip() + '</value>')
                
                # calculate and write attribute spans
                # NOTE: the number of the entity in 'path_attributes'-'pos' does not necessarily correspond to the number of that entity in 'path'!
                #       In 'path_attributes', NonRelevant elements are not counted. In 'path', they are.
                if len(sent['path_attributes']):
                    for path_attribute in sent['path_attributes']:
                        #print(sent['path'])
                        #print(sent['path_attributes'])
                        
                        # retrieve attribute name and rewrite to what the style sheet expects, if needed
                        attr_path = ''
                        attr_name = path_attribute['type'].lower()
                        attr_name = attr_name.replace('positivesentiment','sentiment_positive')
                        attr_name = attr_name.replace('negativesentiment','sentiment_negative')
                        attr_name = attr_name.replace('datetime','time')
                        write_ln(f_xml, '      <' + attr_name + '>')

                        # calculate position of span
                        # initial values
                        attr_path_start = int(path_attribute['pos'])
                        attr_path_stop = attr_path_start + int(path_attribute['span'])

                        attr_path_span =''
                        i = 0
                        pre_attr_span = ''
                        post_attr_span = ''

                        # part of the path preceding the span:
                        while i < attr_path_start:
                            if i in sent['path']:  # check if the entity is 'relevant' (NonRelevant elements are not mentioned in 'path')
                                pre_attr_span = pre_attr_span + ' ' + sent['entities'][i]['index']
                                #print(str(i) + ' - pre_span: ' + pre_attr_span)
                            else:
                                attr_path_start +=1  # NonRelevant elements are not counted in 'path_attributes'-'pos', so the number has to be adjusted.
                                attr_path_stop +=1
                            i +=1
                        # span:
                        while i < attr_path_stop:
                            if i in sent['path']:
                                #print(str(i) + ' include in span: ' + sent['entities'][i]['index'])
                                attr_path_span = attr_path_span + ' ' + sent['entities'][i]['index']
                                i +=1
                            else:
                                #print(str(i) + ' nonrelevant in span: ' + sent['entities'][i]['index'])
                                if attr_path_stop < len(sent['entities']):
                                    attr_path_stop +=1
                                i +=1
                        # part of the path following the span:
                        while i < len(sent['entities']):
                            if i in sent['path']:
                                post_attr_span = post_attr_span + ' ' + sent['entities'][i]['index']
                            i +=1
                    
                        # write path with span:
                        if pre_attr_span != '':
                            pre_attr_span = html.escape(pre_attr_span)
                            write_ln(f_xml, '        <no>' + pre_attr_span.lstrip() + '</no>')
                        attr_path_span = html.escape(attr_path_span)
                        write_ln(f_xml, '        <expanded>' + attr_path_span.lstrip() + '</expanded>')
                        if post_attr_span != '':
                            post_attr_span = html.escape(post_attr_span)
                            write_ln(f_xml, '        <no>' + post_attr_span.lstrip() + '</no>')
                        write_ln(f_xml, '      </' + attr_name + '>')
                        
                write_ln(f_xml, '    </path>')
            
            write_ln(f_xml, '  </Sentence>')
                
        write_ln(f_xml,'</Content>')

        f_xml.close()

