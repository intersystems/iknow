import iknowpy

def strip_negation(text, language="en", iknow=iknowpy.iKnowEngine()):
    iknow.index(text,language)
    stripped = ""
    for s in iknow.m_index['sentences']:
        pos = 0
        negated = []
        for a in s['path_attributes']:
            if a['type']=="Negation":
                #print(a)
                for p in range(pos,a['pos']-1):
                    negated.append(0)
                for p in range(a['pos'],a['pos']+a['span']):
                    negated.append(1)
                pos = a['pos']+a['span']
        for p in range(pos, len(s['entities'])):
            negated.append(0)
        pos = 0
        for e in s['entities']:
            if e['type'] == 'NonRelevant': continue
            if negated[pos] != 1: 
                stripped += e['index']
                stripped+= " "
            pos = pos+1
        stripped += '\n'
    return stripped


import sys
import glob

# read file pattern argument and process the contents, writing directly to stdout (for piping)
# note that file patterns need to be wrapped in quotes or they will be "applied" before this hits python
# usage:
#    $ python strip_negation.py test.txt
#    $ python strip_negation.py "*.txt" | grep fix
for path in glob.glob(sys.argv[1]):
    with open(path, 'r') as file:
        for line in file:
            print(strip_negation(line))

#
# variation: this reads piped text straight from stdin
# usage
#    $ cat test.txt | python strip_negation.py
#    $ cat test.txt | python strip_negation.py | grep "progress"
#for line in sys.stdin:
#    print(strip_negation(line))