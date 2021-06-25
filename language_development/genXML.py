# -*- coding: utf-8 -*-
"""
Updated on Mon Feb 15 2021
@author: ISC-SDE

This tool translates the output of the iKnow engine into XML files.
For the visualisation of the XML, make sure the style sheet iKnowXML.xsl is in the same directory as this script. 

Usage: "python genXML.py <text files directory> <output directory> <language>"
Example (on Windows): "python genXML.py C:/TextCorpus/English/Financial/ C:/Raw/English/ en

For each txt file in the input directory, a corresponding xml file will be generated in the output directory.

"""

# import the usual suspects...
import os, sys, ntpath
import html

# do "pip install iknowpy" if iknowpy is not installed
import iknowpy

#
# Following are default runtime parameters if no command line parameters are present.
#
in_path_par = "C:/P4/Users/jdenys/text_input_data/en/"  # input directory with text files
out_path_par = "C:/tmp/"                                # output directory to write the RAW file
language_par = "en"                                     # language selector

#
# Command line parameters
#
if (len(sys.argv)>1):
    in_path_par = sys.argv[1]
if (len(sys.argv)>2):
    out_path_par = sys.argv[2]
if (len(sys.argv)>3):
    language_par = sys.argv[3]

#
# Required style sheet to visualise the XML
#
style_sheet_par = os.path.join(os.getcwd(), 'iKnowXML.xsl')


#
# function to write to output file
# 
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


#
# define variables
#
engine = iknowpy.iKnowEngine()
sentence_order = 0
split_sentence = []
lexrep_info = []


#
# process files one by one
#
for text_file in f:
    print('processing ' + text_file)
    f_text = open(text_file, "rb")
    header = f_text.read(3)
    if (header == b'\xef\xbb\xbf'): #Utf8 BOM
        header = b''    # remove BOM
    text = header + f_text.read() # read text, must be utf8 encoded
    text = text.decode('utf8') # decode text to Unicode
    f_text.close()
    
    # create output file, write header
    filename_xml = ntpath.basename(text_file) + '.xml'   # use ntpath to ensure compatibility with Windows and Linux

    # print(filename_xml)
    f_xml = open(os.path.join(out_path_par, filename_xml), 'wb')

    f_xml.write(b'\xef\xbb\xbf') # Utf8 BOM
    write_ln(f_xml,'<?xml version=\"1.0\" encoding=\"utf-8\"?>')
    write_ln(f_xml,'<?xml-stylesheet  type=\"text/xsl\" href=\"' + style_sheet_par + '\"?>')
    write_ln(f_xml,'<Content>')
    
    # process text with iKnow
    engine.index(text, language_par)
    
    # translate engine output into xml
    for sent in engine.m_index['sentences']:
        #print(sent)
        #print(sent['sent_attributes'])
        
        # write xml tag <Sentence> with xml attribute 'order'
        sentence_order +=1
        write_ln(f_xml, '  <Sentence ' + 'order=\"' + str(sentence_order) +'\">')
        ent_stop = ''
        
        # link entities with attributes, generate xml
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
    

