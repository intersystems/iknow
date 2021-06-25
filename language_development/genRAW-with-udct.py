# This Python file uses the following encoding: utf-8
''' genRAW.py tool for generating linguistic reference material
    Usage: "python genRAW-with-udct.py <text files directory> <output directory> <language> <user dictionary with path>"
    Example (on Windows): "python genRAW.py C:/TextCorpus/English/Financial/ C:/Raw/English/ en
'''

# import the usual suspects...
import sys, time

# run "pip install iknowpy" if "import iknowpy" fails.
import iknowpy

#
# Following are default runtime parameters if no command line parameters are present.
#
in_path_par = "C:/P4/Users/jdenys/text_input_data/en/"  # input directory with text files
out_path_par = "C:/tmp/"                                # output directory to write the RAW file
language_par = "en"                                     # language selector
user_dct_par = "../reference_materials/udct_test_dictionaries/en_udct.txt"
OldStyle = True                                         # mimics the old-style RAW file format

# print(sys.argv)
if (len(sys.argv)>1):
    in_path_par = sys.argv[1]
if (len(sys.argv)>2):
    out_path_par = sys.argv[2]
if (len(sys.argv)>3):
    language_par = sys.argv[3]
if (len(sys.argv)>4):
    user_dct_par = sys.argv[4]

print('genRAW input_dir=\"'+in_path_par+'\" output_dir=\"'+out_path_par+'\" language=\"' + language_par + '\" user dictionary=\"' + user_dct_par + '\"')

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


# load user dictionary
user_dictionary = iknowpy.UserDictionary()
read_udct_file(user_dct_par, user_dictionary)
ret = engine.load_user_dictionary(user_dictionary)

#
# process the input and create output
#

# filename and header
f_raw = open(out_path_par + "i.Know." + time.strftime("%Y.%m.%d (%Hu%M)", time.gmtime()) + ".raw", 'wb')
f_raw.write(b'\xef\xbb\xbf') # Utf8 BOM
write_ln(f_raw,'#')
write_ln(f_raw,"# in_path_par:"+in_path_par)
write_ln(f_raw,"# out_path_par:"+out_path_par)
write_ln(f_raw,"# language_par:"+language_par)
write_ln(f_raw,"# generated with " + __file__)
write_ln(f_raw,"#\n#")

# start processing
for text_file in f_rec:
    print('processing: ' + text_file)
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
#        print(sent['sent_attributes'])
#        print(sent['entities'])

        if OldStyle:
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
    
            else: # other languages
                for entity in sent['entities']:
                    #print(entity)
                    ent_type = entity['type']
                    lit_text = text[entity['offset_start']:entity['offset_stop']].replace("\n"," ") # literal representation of sentence, replace newline by space
                    lit_text = lit_text.replace('\r',' ')  # replace return by space
                    while "  " in lit_text:   # get rid of double spaces, caused by replaced newlines and returns
                        lit_text = lit_text.replace("  ", " ")
                    ent_start = entity['offset_start']
                    if ent_type == 'NonRelevant':
                        if (ent_start != ent_stop):
                            sentence_raw = sentence_raw + lit_text
                        else:
                            sentence_raw = sentence_raw + lit_text
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
                lit_text = text[lit_start:lit_stop].replace("\n","") # literal representation of sentence, with newlines removed
                sentence_raw = sentence_raw + ent_type + '= \"' + lit_text + '\"  '

            sentence_raw = sentence_raw + '>'

        write_ln(f_raw, sentence_raw)
        #
        # sentence attributes
        #
        if (len(sent['sent_attributes'])):
            for sent_attribute in sent['sent_attributes']:
                attr_name = sent_attribute['type'].lower()

                attr_entity_literal = text[sent['entities'][sent_attribute['entity_ref']]['offset_start']:sent['entities'][sent_attribute['entity_ref']]['offset_stop']].replace("\n"," ") # corresponding entity index literal value, replace newline by space
                attr_entity_literal = attr_entity_literal.replace('\r',' ')  # replace return by space
                while "  " in attr_entity_literal:   # get rid of double spaces, caused by replaced newlines and returns
                        attr_entity_literal = attr_entity_literal.replace("  ", " ")
                if (attr_name == 'datetime'):
                    attr_name = 'time'
                sent_attribute_raw = '<attr type=\"' + attr_name + '\" literal=\"' + attr_entity_literal + ('\" marker=\"' if OldStyle==False else '\" token=\"') + sent_attribute['marker'].lstrip() + '\"'
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
        if (len(sent['path']) and language_par != 'ja'):
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

user_dictionary.clear()

f_raw.close()


