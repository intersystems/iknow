# This Python file uses the following encoding: utf-8
''' genRAW.py tool for generating linguistic reference material
    Usage: "python genRAW.py <text files directory> <output directory> <language>"
    Example (on Windows): "python genRAW.py C:/TextCorpus/English/Financial/ C:/Raw/English/ en
'''

import sys

# run "pip install iknowpy" if "import iknowpy" fails.
import iknowpy

import os
import pprint
import time

#
# Following are default runtime parameters if no command line parameters are present.
#
in_path_par = "C:/P4/Users/jdenys/text_input_data/en/"  # input directory with text files
out_path_par = "C:/tmp/"                                # output directory to write the RAW file
language_par = "en"                                     # language selector
OldStyle = True                                         # mimics the old-style RAW file format

# print(sys.argv)
if (len(sys.argv)>1):
    in_path_par = sys.argv[1]
if (len(sys.argv)>2):
    out_path_par = sys.argv[2]
if (len(sys.argv)>3):
    language_par = sys.argv[3]

print('genRAW input_dir=\"'+in_path_par+'\" output_dir=\"'+out_path_par+'\" language=\"'+language_par+'\"')

def write_ln(file_,text_):
    file_.write((text_+"\r\n").encode('utf8'))

#
# collect text documents in 'in_path_par'
#
from os import walk

f = []  # non-recursive list of files, .txt only
for (dirpath, dirnames, filenames) in walk(in_path_par):
    for single_file in filenames:
        if (single_file.endswith('.txt')):
            full_path = dirpath + single_file
            f.append(full_path)
    break

f_rec = []  # recursive list of files, .txt only
def collect_files_recursive(in_path_par):
    for (dirpath, dirnames, filenames) in walk(in_path_par):
        for single_file in filenames:
            if (single_file.endswith('.txt')):
                full_path = dirpath + single_file
                f_rec.append(full_path)
        for single_dir in dirnames:
            full_dir = dirpath + single_dir + "/"
            collect_files_recursive(full_dir)
        break

collect_files_recursive(in_path_par)

engine = iknowpy.iKnowEngine()

f_raw = open(out_path_par + "i.Know." + time.strftime("%Y.%m.%d (%Hu%M)", time.gmtime()) + ".raw", 'wb')
f_raw.write(b'\xef\xbb\xbf') # Utf8 BOM
write_ln(f_raw,'#')
write_ln(f_raw,"# in_path_par:"+in_path_par)
write_ln(f_raw,"# out_path_par:"+out_path_par)
write_ln(f_raw,"# language_par:"+language_par)
write_ln(f_raw,"#\n#\n#")

for text_file in f_rec:
    print(text_file)
    # do file.WriteLine("D"_$char(1)_$p(name,tSep,$l(name,tSep))_$char(1)_filename)
    if OldStyle:
        write_ln(f_raw,'\nD\x01'+text_file+'\x01'+in_path_par+text_file)
    else:
        write_ln(f_raw,'\n<D name=\"'+text_file+'\" file=\"'+in_path_par+text_file+'\">') # D050_sentences.txtC:\P4\Users\jdenys\text_input_data\ja\050_sentences.txt
    
    f_text = open(text_file, "rb")
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
        if OldStyle:
            sentence_raw = 'S\x01'
            ent_stop = ''
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
        else:
            sentence_raw = '<S '
            for entity in sent['entities']:
                ent_type = entity['type']
                lit_start = entity['offset_start']
                lit_stop = entity['offset_stop']
                lit_text = text[lit_start:lit_stop]
                sentence_raw = sentence_raw + ent_type + '= \"' + lit_text + '\"  '

            sentence_raw = sentence_raw + '>'

        write_ln(f_raw, sentence_raw)
        #
        # sentence attributes
        #
        if (len(sent['sent_attributes'])):
            for sent_attribute in sent['sent_attributes']:
                attr_name = sent_attribute['type'].lower()
                attr_marker = sent_attribute['marker'] # corresponds to lexreps.csv match 
                attr_entity = sent['entities'][sent_attribute['entity_ref']]['index'] # corresponding entity index value

                attr_marker_literal = text[sent_attribute['offset_start']:sent_attribute['offset_stop']] # literal version of the marker
                attr_entity_literal = text[sent['entities'][sent_attribute['entity_ref']]['offset_start']:sent['entities'][sent_attribute['entity_ref']]['offset_stop']] # corresponding entity index literal value

                if (attr_name == 'datetime'):
                    attr_name = 'time'
                sent_attribute_raw = '<attr type=\"' + attr_name + '\" literal=\"' + text[sent['entities'][sent_attribute['entity_ref']]['offset_start']:sent['entities'][sent_attribute['entity_ref']]['offset_stop']] + ('\" marker=\"' if OldStyle==False else '\" token=\"') + sent_attribute['marker'].lstrip() + '\"'
                if sent_attribute['value']:
                   sent_attribute_raw = sent_attribute_raw + ' value=\"' + sent_attribute['value'] + '\"'
                if sent_attribute['unit']:
                   sent_attribute_raw = sent_attribute_raw + ' unit=\"' + sent_attribute['unit'] + '\"'

                sent_attribute_raw = sent_attribute_raw + '>'
                # print(sent_attribute_raw)
                write_ln(f_raw, sent_attribute_raw)
        #
        # if path not empty and language is Japanese, emit as attribute : "entity_vector" 
        #
        if (len(sent['path']) and language_par=='ja'):
            # <attr type="time" literal="経済学部2年のセンター利用入試" token="2年">
            entity_vec_raw = '<attr type=\"entity_vector\"'
            for sent_index in sent['path']:
                entity = sent['entities'][sent_index]
                lit_text = text[entity['offset_start']:entity['offset_stop']]
                entity_vec_raw = entity_vec_raw + ' \"' + lit_text + '\"'
            entity_vec_raw = entity_vec_raw + '>'
            write_ln(f_raw, entity_vec_raw)
        #
        # write entities
        #
        for entity in sent['entities']:
            if OldStyle:
                if entity['type'] == 'Concept': # C2only sound
                    write_ln(f_raw, 'C\x01' + str(entity['index'].count(' ')+1) + '\x01' + entity['index'])
                if entity['type'] == 'Relation':
                    write_ln(f_raw, 'R\x01' + entity['index'])
                if entity['type'] == 'PathRelevant': # PR<we>
                    write_ln(f_raw, 'PR<' + entity['index'] + '>')
            else:
                write_ln(f_raw, "<"+entity['type']+'  \"'+entity['index']+'\">')            
        #
        # write path
        #
        if (len(sent['path'])):
            if OldStyle:
                path_raw = 'P\x01'
                for sent_index in sent['path']:
                    path_raw = path_raw + sent['entities'][sent_index]['index'] + ' '
            else:
                path_raw = '<P '
                for sent_index in sent['path']:
                    path_raw = path_raw + ' \"' + sent['entities'][sent_index]['index'] + '\"'
                path_raw = path_raw + '>'

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


