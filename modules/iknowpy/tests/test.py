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

text_source = "Be the change you want to see in life."
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "en":
	raise ValueError(f'Language "en" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "Oder die Erkundung der Natur - und zwar ohne Anleitung."
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "de":
	raise ValueError(f'Language "de" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "Микротерминатор может развивать скорость до 30 сантиметров за секунду, пишут калининградские СМИ.";
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "ru":
	raise ValueError(f'Language "ru" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "En Argentina no hay estudios previos reportados en cuanto a la elaboración de vinos cítricos ni de «vino de naranja».";
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "es":
	raise ValueError(f'Language "es" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "En pratique comment le faire ?";
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "fr":
	raise ValueError(f'Language "fr" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "こんな台本でプロットされては困る、と先生は言った。";
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "":
	raise ValueError(f'Language "" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "Op basis van de afzonderlijke evaluatieverslagen stelt de Commissie een synthese op communautair niveau op."
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "nl":
	raise ValueError(f'Language "nl" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "Distingue-se o mercado de um produto ou serviço dos mercados de fatores de produção, capital e trabalho."
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "pt":
	raise ValueError(f'Language "pt" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "Jag är bäst i klassen. Ingen gör efter mig, kan jag inte lämna. Var försiktig, är gräset alltid grönare på andra sidan."
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "sv":
	raise ValueError(f'Language "sv" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "грошових зобов'язань, прийнятих на себе згідно з умов цього договору."
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "uk":
	raise ValueError(f'Language "uk" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

text_source = "Létající jaguár je novela spisovatele Josefa Formánka z roku 2004."
language_identified = engine.IdentifyLanguage(text_source)
if language_identified[0] != "cs":
	raise ValueError(f'Language "cs" is not recognised.')
else:
	print(f'Language {language_identified[0]!r} detected with certainty {language_identified[1]!r}.')

#void iKnowUnitTests::ALI(const char* pMessage) {
text_source = "Ceci n'est pas une pipe. This is not a paper plane."
engine.index(text_source, "en|fr", traces=True)
switch_count = 0
for trace in engine.m_traces:
	key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
	if (key=='SwitchKnowledgebase'):
		switch_count += 1

if switch_count != 2:
	raise ValueError(f'2 language switches expected: en:fr and fr:en !')

engine.index(text_source, "nl|pt|fr", traces=True)
for trace in engine.m_traces:
	key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
	if (key=='SwitchKnowledgebase'):
		raise ValueError(f'no switching from language is expected !')

#void iKnowUnitTests::SourceVersusSentenceALI(const char* pMessage)
text_source = "Ceci n'est pas une pipe. This is not a paper plane."
engine.index(text_source, "en|fr", traces=True, detect_language_at="document")
switch_count = 0
for trace in engine.m_traces:
	key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
	if (key=='SwitchKnowledgebase'):
		switch_count += 1

if switch_count != 1:
	raise ValueError(f'Only ONE language switch expected !')

# Try all languages
switch_count = 0
engine.index(text_source, "*", traces=True)
for trace in engine.m_traces:
	key, value = trace.split(':', 1)[0],trace.split(':', 1)[1]
	if (key=='SwitchKnowledgebase'):
		switch_count += 1

if switch_count != 2:
	raise ValueError(f'2 language switches expected in "all languages" request !')



