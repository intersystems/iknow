import os
import sys
import pprint
import traceback

# On Python >= 3.8 on Windows, we can specify where the iknowpy dependencies are if they are not in the same directory
# as the iknowpy module.
if sys.version_info >= (3, 8) and os.name == 'nt':
	if 'ICUDIR' in os.environ:
		icudir = os.environ['ICUDIR']
	else:
		icudir = os.path.join(os.getcwd(), '../../thirdparty/icu')
	os.add_dll_directory(os.path.join(icudir, 'bin64'))
	os.add_dll_directory(os.path.join(os.getcwd(), '../../kit/x64/Release/bin'))

try:
	import iknowpy
except ImportError:
	traceback.print_exc()
	print('iknowpy dependencies not found.')
	sys.exit(1)

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
