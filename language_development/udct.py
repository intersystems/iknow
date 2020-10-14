# This Python file uses the following encoding: utf-8

import sys
from collections import namedtuple

class UserDictionary(object):

    # constants
    # role labels
    CONCEPT = "UDConcept"
    RELATION = "UDRelation"
    NONRELEVANT = "UDNonRelevant"

    # attribute labels
    NEGATION = "UDNegation"
    UNIT = "UDUnit"
    SENTIMENT_POSITIVE = "UDPosSentiment"
    SENTIMENT_NEGATIVE = "UDNegSentiment"
    NUMBER = "UDNumber"
    TIME = "UDTime"

    # constructor
    def __init__(self, entries=[]):
        self.entries = entries

    # generic method (make private?)
    def add_label(self, string, label):
        # declare Entry named tuple for convenience
        Entry = namedtuple("Entry", ["string", "label"])
        self.entries.append(Entry(string, label))

    # example of specific (public) method
    def add_concept(self, string):
        self.add_label(string, UserDictionary.CONCEPT)

    def add_relation(self, string):
        self.add_label(string, UserDictionary.RELATION)

    def add_nonrelevant(self, string):
        self.add_label(string, UserDictionary.NONRELEVANT)

    def add_negation_term(self, string):
        self.add_label(string, UserDictionary.NEGATION)

    def add_unit_term(self, string):
        self.add_label(string, UserDictionary.UNIT)

    def add_positive_sentiment_term(self, string):
        self.add_label(string, UserDictionary.SENTIMENT_POSITIVE)

    def add_negative_sentiment_term(self, string):
        self.add_label(string, UserDictionary.SENTIMENT_NEGATIVE)

    def add_number_term(self, string):
        self.add_label(string, UserDictionary.NUMBER)

    def add_time_term(self, string):
        self.add_label(string, UserDictionary.TIME)


# for local language development, adapt next line to your local situation, and uncomment next 2 lines 
sys.path.insert(0, 'C:/Users/jdenys/source/repos/iknow/kit/x64/Release/bin')
import engine as iknowpy

# for "pip install iknowpy", next line will do, outcomment for local language development
# import iknowpy

engine = iknowpy.iKnowEngine()
user_dictionary = iknowpy.iKnowUserDictionary()

user_dictionary.add_label("some text", "UDMeasurement")

raw_text = "Ik ben Fr. dus niet OVERHEERLIJK !"
norm_text = engine.normalize_text(raw_text, "nl")

dict = UserDictionary()
dict.add_negation_term("w/o")
dict.add_label("one concept", dict.CONCEPT)    # CONCEPT is a constant for "UDConcept"
dict.add_label("some text", dict.MEASUREMENT)  # MEASUREMENT is a constant for "UDMeasurement"
engine.load_dictionary(dict)
engine.index("some text w/o one concept", "en")

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

