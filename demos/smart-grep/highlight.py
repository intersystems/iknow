import iknowpy
from colorama import init, Fore, Style

def highlight(text, language="en", iknow=iknowpy.iKnowEngine()):
    init() # init colorama
    iknow.index(text,language)
    for s in iknow.m_index['sentences']:
        # first figure out where negation spans are and tag those entities
        for a in s['path_attributes']:
            if a['type']=="Negation":
                # these ara path positions!
                for p in range(a['pos'],a['span']):
                    ent = s['path'][p]
                    s['entities'][ent]['Negated'] = 1
        for e in s['entities']:
            colour = 30
            style = 0
            if "Negated" in e:
                colour = 31
            if e['type'] == 'Concept':
                style = 1
            print("\033[" + str(colour) + ";" + str(style) + "m" + text[e['offset_start']:e['offset_stop']], end=' ')



import sys

highlight(sys.argv[1])
