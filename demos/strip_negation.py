import iknowpy

def strip_negation(text, language="en", iknow=iknowpy.iKnowEngine()):
    
    iknow.index(text, language)
    stripped = ""

    for s in iknow.m_index['sentences']:
        
        # first figure out where negation spans are and tag those entities
        for a in s['path_attributes']:
            
            # path attributes are expressed as positions within s['path'],
            # which in turn keys into the s['entities'] array
            if a['type']=="Negation":
                for ent in range(s['path'][a['pos']], 
                                 s['path'][a['pos']+a['span']-1]+1):
                    s['entities'][ent]['neg'] = 1
                    
        for e in s['entities']:
            if "neg" in e:
                continue
            stripped += text[e['offset_start']:e['offset_stop']] + " "

    return stripped

# command-line processing
import sys, glob

lang = "en"
if len(sys.argv)>2:
    lang = sys.argv[2]
iknow = iknowpy.iKnowEngine()

# read file pattern argument and process the contents, writing directly to stdout (for piping)	
# note that file patterns need to be wrapped in quotes or they will be "applied" before this hits python	
# usage:	
#    $ python strip_negation.py test.txt	
#    $ python strip_negation.py test.txt "fr"
#    $ python strip_negation.py "*.txt" | grep fix	
for path in glob.glob(sys.argv[1]):	
    with open(path, 'r') as file:
        for line in file:	
            print(strip_negation(line,lang,iknow))	

#	
# variation: this reads piped text straight from stdin	
# usage	
#    $ cat test.txt | python strip_negation.py	
#    $ cat test.txt | python strip_negation.py | grep "progress"	
#for line in sys.stdin:	
#    print(strip_negation(line,lang,iknow)) 