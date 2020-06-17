# This Python file uses the following encoding: utf-8

import pprint
import iknowpy

engine = iknowpy.iKnowEngine()

print('Languages Set:')
print(engine.get_languages_set(), end='\n\n')

print('Input text:')
text = '志望学部の決定時期について経営学部生に関しては表7（a）（b）に、経済学部生に関しては表8（a）（b）に示す。ただし、外国人入試とスポーツ推薦入試に関しては標本の大きさが小さく、ミスリーディングを避けるため除いてある。概観して分かることは、どの入試区分においても高校3年生の時に学部を決めたとするのが最も普通のことである点である。一方、一般入試とセンター利用入試で合格後にやっと志望学部を決定したという学生群の率をみると、経済学部2年のセンター利用入試で79%と非常に高い率に達している。ただしこれは大きさ15の小標本からであるので解釈に注意を要する。経営学部の最も高い数値を示すのは2年生の一般入試で39%である。以上の学生群は受験の段階で併願により受験した群であり、合格発表後に学部の決定がなされた者たちである。この学生群の中には創価大学であればどの学部でも良いと考えている学生達や、「たまたま受かったから来た」「ここしか受からなかった」という学生が多数存在するグループである。このような学生群が入学後の勉学に対して問題を抱えている可能性は十分考えられる。センター利用入試の学生達も学力レベルは高いとしても勉学動機の面では問題群と見倣した方が良。'
print(text, end='\n\n')

engine.index(text, 'ja')
doc_iknow = engine.m_index  # engine data in dictionary format

#print('Index:')
#pp = pprint.PrettyPrinter()
#pp.pprint(x)

#
# first build the sentence source map (entity_id to entity_index), and collect dominance values
#
mapTextSource = {}  #  dictionary to map id's to text indexes
dictDominantConcepts = {} # dictionary to store dominance values
for sent in doc_iknow['sentences']:
    for entity in sent['entities']:
        mapTextSource[entity['entity_id']]=entity['index']  # map entity_id to index value
        dictDominantConcepts[entity['entity_id']]=entity['dominance_value'] # collect dominance values

#
# generate a sorted list of dominance values
#
listDominanceSorted = [] # empty list to collect dominance values
for id, dom in dictDominantConcepts.items():
    listDominanceSorted.append((dom,id))
listDominanceSorted = sorted(listDominanceSorted,reverse=True)

#
# print top20%
#
len_dominance_list = len(listDominanceSorted)
if (len_dominance_list > 0):
    print('Top 20% Dominance Entities:',end='\n\n')
    top_20p = len_dominance_list/5
    i = 1
    for dom in listDominanceSorted:
        print('\"',mapTextSource[dom[1]],'\" >>Dominance=',dom[0],end='\n')
        i = i + 1
        if (i > top_20p):
            break

print("\n\n")

#
# Proximity values: print top20%
#
len_proximity_list = len(doc_iknow["proximity"])
if (len_proximity_list > 0):
    print('Top 20% Proximity entities:',end='\n\n')
    top_20p = len_proximity_list/5
    i = 1
    for prox in doc_iknow["proximity"]: # prox is a double tuple = ((entity_1,entity_2),proximity_value), the list is sorted by the C++ engine
        print('\"',mapTextSource[prox[0][0]],'\":\"',mapTextSource[prox[0][1]],'\"  >>Proximity=',prox[1],end='\n')
        i = i + 1
        if (i > top_20p):
            break 

print("\n\n")

#
# print Entity Vectors
for sent in doc_iknow["sentences"]:

    for entity in sent['entities']:
        e_type = entity['type']
        # print(entity['index'])

    # print(sent['sent_attributes'])
    print('Entity vectors for sentence: ', sent['path'])
    for sent_index in sent['path']:
        print(sent['entities'][sent_index]['index'])
    print('\n')
    # print(sent['path_attributes'])
