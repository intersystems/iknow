import iknowpy
from colorama import init, Fore, Style

init() # init colorama

def highlight(text, language="en", iknow=iknowpy.iKnowEngine()):
    
    iknow.index(text, language)
    
    for s in iknow.m_index['sentences']:
        
        # first figure out where negation spans are and tag those entities
        for a in s['path_attributes']:
            
            # path attributes are expressed as positions within s['path'],
            # which in turn keys into the s['entities'] array
            for ent in range(s['path'][a['pos']], 
                             s['path'][a['pos']+a['span']-1]+1):
                if a['type']=="Negation":
                    s['entities'][ent]['colour'] = Fore.RED
                if a['type']=="Certainty":
                    s['entities'][ent]['colour'] = Fore.CYAN
                    
        for e in s['entities']:
            colour = Fore.BLACK
            style = Style.NORMAL
            
            if "colour" in e:
                colour = e["colour"]
                
            if (e['type'] == 'Concept'):
                style = Style.BRIGHT
            if (e['type'] == 'NonRelevant') | (e['type'] == 'PathRelevant'):
                style = Style.DIM
                
            print(colour + style + text[e['offset_start']:e['offset_stop']], end=' ')
            
        print("\n")


import sys

highlight(sys.argv[1])
