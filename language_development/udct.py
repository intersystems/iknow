# This Python file uses the following encoding: utf-8

import sys

# for local language development, adapt next line to your local situation, and uncomment next 2 lines 
sys.path.insert(0, 'C:/Users/jdenys/source/repos/iknow/kit/x64/Release/bin')
import engine as iknowpy
# for "pip install iknowpy", next line will do, outcomment for local language development
# import iknowpy

engine = iknowpy.iKnowEngine()

ret = engine.udct_addLabel('Literal (1)', 'UDNegation') # assign a UD label
ret = engine.udct_addLabel('Literal (2)', 'Illegal Label') # returns -2 : label does not exist
ret = engine.udct_addLabel('Literal (3)', 'UDPosSentiment') 

# add an End/NoEnd condtion to the user dictionary.
ret = engine.udct_addSEndCondition("Fr.", False)

# add a Negation term to the user dictionary.
ret = engine.udct_addNegationTerm("NEEE")

# add a Positive Sentiment term to the user dictionary.
ret = engine.udct_addPositiveSentimentTerm("WAUW")

# add a Negative Sentiment term to the user dictionary.
ret = engine.udct_addNegativeSentimentTerm("BELACHELIJK")

engine.udct_use(True) # do use user dictionary

engine.index("This is Fr. Literal (1) nee and Literal (2) and wauw Literal (3) all belachelijk together.", "en", True)
for trace in engine.m_traces:
    key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
    if (key=='UserDictionaryMatch'):
        print(value)

def read_udct_file(file_):
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
            ret = engine.udct_addLabel(literal,action)
            if (ret == -2):
                print('label ' + action + ' not valid !')

    f_udct.close()


read_udct_file("definition_terms_v2c.dct")
engine.index("This soltamox and estrogen receptor protein positive etc...", "en", True)
for trace in engine.m_traces:
    key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
    if (key=='UserDictionaryMatch'):
        print(value)

