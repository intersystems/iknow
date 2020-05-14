import pprint
import iknowpy

engine = iknowpy.iKnowEngine()

print('Languages Set:')
print(engine.get_languages_set(), end='\n\n')

print('Input text:')
text = 'This is a test of the Python interface to the iKnow engine.'
print(text, end='\n\n')

engine.index(text, 'en')

print('Index:')
pp = pprint.PrettyPrinter()
pp.pprint(engine.m_index)
