# This Python file uses the following encoding: utf-8

import sys

# for local language development, adapt next line to your local situation, and uncomment next 2 lines 
# sys.path.insert(0, 'C:/Users/jdenys/source/repos/iknow/kit/x64/Release/bin')
# import engine as iknowpy

# for "pip install iknowpy", next line will do, outcomment for local language development
import iknowpy

engine = iknowpy.iKnowEngine()
user_dictionary = iknowpy.UserDictionary()
user_dictionary.add_label("some text", "UDUnit")
# following line will throw an exception : Unkown label
# user_dictionary.add_label("tik tok", "UDUnknown")
user_dictionary.add_sent_end_condition("Fr.", False)

user_dictionary.add_concept("one concept")
user_dictionary.add_relation("one relation")
user_dictionary.add_non_relevant("crap")
user_dictionary.add_negation("w/o")
user_dictionary.add_positive_sentiment("great")
user_dictionary.add_negative_sentiment("awfull")
user_dictionary.add_unit("Hg")
user_dictionary.add_number("magic number")
user_dictionary.add_time("future")

ret = engine.load_user_dictionary(user_dictionary)
engine.index("some text Fr. w/o one concept and crap one relation that's great and awfull, magic number 3 Hg from future", "en", True) # generate Traces
for trace in engine.m_traces:
    key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
    if (key=='UserDictionaryMatch'):
        print(value)

engine.unload_user_dictionary()
engine.index("some text Fr. w/o one concept and crap one relation that's great and awfull, magic number 3 Hg from future", "en", True) # generate Traces
for trace in engine.m_traces:
    key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
    if (key=='LexrepIdentified'):
        print(value)

def read_udct_file(file_,udct_):
    f_udct = open(file_,"r",True,"utf8")
    for txt_line in f_udct:
        # print(txt_line)
        txt_line = txt_line.rstrip()

        txt_list = txt_line.split(',')
        if (txt_list[0] == ''): # empty input
            continue
        if (txt_list[0] == '/*'): # starting with comment
            continue

        lexrep, action = txt_list[0], txt_list[1]
        if (lexrep[0] == '@'):
            literal = lexrep[1:]
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

user_dictionary.clear()
read_udct_file("definition_terms_v2c.dct",user_dictionary)
ret = engine.load_user_dictionary(user_dictionary)
engine.index("This soltamox and estrogen receptor protein positive etc...", "en", True)
for trace in engine.m_traces:
    key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
    if (key=='UserDictionaryMatch'):
        print(value)

