# This Python file uses the following encoding: utf-8

import sys

# for local language development, adapt next line to your local situation, and uncomment next 2 lines 
# sys.path.insert(0, 'C:/Users/jdenys/source/repos/iknow/kit/x64/Release/bin')
# import engine as iknowpy

# for "pip install iknowpy", next line will do, outcomment for local language development
import iknowpy

def count_matches(traces, match_key, target_count = -1, debug = False) -> int:
    match_count = 0
    if debug:
        print("\nDEBUG counting : "+match_key)
    for trace in traces:
        key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
        if (key == match_key):
            if debug:
                print(value)
            match_count += 1
    if debug:
        print("DEBUG end\n")
    if (target_count >= 0) and ((target_count*2) != match_count):
        print("ERROR: Unexpected number of trace entries for key '%s'. Found %d instead of %d." % (match_key, match_count, target_count))
    else:
        print("Found %d trace entries for key '%s'" % (match_count/2, match_key))
    return match_count

# TODO: grab from command line args
debug = False
test_sentence = "some text Fr. w/o one concept and crap one relation that's great and awfull, magic number 3 Hg from future"

print("\nBuilding User Dictionary using API...")
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

if len(user_dictionary.entries) != 11:
    print("ERROR: UD not fully loaded!")

ret = engine.load_user_dictionary(user_dictionary)
engine.index(test_sentence, "en", True) # generate Traces
count_matches(engine.m_traces, 'UserDictionaryMatch', 11, debug)

print("\nUnloading User Dictionary...")
engine.unload_user_dictionary()
engine.index(test_sentence, "en", True) # generate Traces
count_matches(engine.m_traces, 'LexrepIdentified', -1, debug)
count_matches(engine.m_traces, 'UserDictionaryMatch', 0)

# test array load
print("\nBuilding User Dictionary from list...")
user_dictionary_2 = iknowpy.UserDictionary(user_dictionary.entries)
if len(user_dictionary_2.entries) != 11:
    print("ERROR: UD not fully loaded!")
engine.load_user_dictionary(user_dictionary_2)
engine.index(test_sentence, "en", True) # generate Traces
count_matches(engine.m_traces, 'UserDictionaryMatch', 11)


# test clear
print("\nClearing User Dictionary ...")
user_dictionary_2.clear()
if len(user_dictionary_2.entries) != 0:
    print("ERROR: UD not fully cleared!")
engine.unload_user_dictionary() # why is this needed?
engine.load_user_dictionary(user_dictionary_2)
engine.index(test_sentence, "en", True) # generate Traces
count_matches(engine.m_traces, 'UserDictionaryMatch', 0)

# test simple file load
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

print("\nBuilding User Dictionary from file...")
user_dictionary.clear()
read_udct_file("definition_terms_v2c.dct",user_dictionary)
ret = engine.load_user_dictionary(user_dictionary)
engine.index("This soltamox and estrogen receptor protein positive etc...", "en", True)
ret = engine.load_user_dictionary(user_dictionary)
count_matches(engine.m_traces, 'UserDictionaryMatch', 5, debug)


print("\nDone")