#include "iKnowUnitTests.h"
#include "engine.h"

#include <cctype>
#include <stdexcept>
#include <iostream>
#include <map>
#include <vector>

using namespace testing;
using namespace std;

using iknow::base::String;
using iknow::base::IkStringEncoding;
using namespace iknowdata;

void iKnowUnitTests::runUnitTests(void)
{
	const char* pError = NULL;
	try {
		iKnowUnitTests test_collection;
		pError = "Japanese output must generate entity vectors";
		test_collection.test1(pError);
		pError = "Only one measurement attribute in example English text";
		test_collection.test2(pError);
		pError = "4 Sentence Attribute markers in sentence";
		test_collection.test3(pError);
		pError = "Test on SBegin/SEnd labels";
		test_collection.test4(pError);
		pError = "Test on User Dictionary (UDCT)";
		test_collection.test5(pError);
		pError = "Test on Text Normalizer";
		test_collection.test6(pError);
		pError = "Issue31 : https://github.com/intersystems/iknow/issues/31";
		test_collection.test7(pError);
		pError = "Issue37 : https://github.com/intersystems/iknow/issues/37";
		test_collection.test8(pError);
		pError = "Issue41 : https://github.com/intersystems/iknow/issues/41";
		test_collection.Issue41(pError);
		pError = "Issue39 : https://github.com/intersystems/iknow/issues/39";
		test_collection.Issue39(pError);
		pError = "Issue42 : https://github.com/intersystems/iknow/issues/42";
		test_collection.Issue42(pError);
		pError = "Saskia1";
		test_collection.Saskia1(pError);
		pError = "Saskia2";
		test_collection.Saskia2(pError);
		pError = "Saskia3";
		test_collection.Saskia3(pError);
		pError = "Saskia4";
		test_collection.Saskia4(pError);
		pError = "Issue64 : https://github.com/intersystems/iknow/issues/64";
		test_collection.Issue64(pError);
		pError = "Issue70 : https://github.com/intersystems/iknow/issues/70";
		test_collection.Issue70(pError);
		pError = "DP-402269 : https://usjira.iscinternal.com/browse/DP-402269";
		test_collection.DP402269(pError);
		pError = "Issue86 : https://github.com/intersystems/iknow/issues/86";
		test_collection.Issue86(pError);
		pError = "Issue104 : https://github.com/intersystems/iknow/issues/104";
		test_collection.Issue104(pError);
		pError = "Normalizer test";
		test_collection.Benjamin1(pError);

	}
	catch (std::exception& e) {
		cerr << "*** Unit Test Failure ***" << endl;
		cerr << e.what() << endl;
		exit(-1);
	}
	catch (...) {
		cerr << "Unit Test \"" << pError << "\" failed !" << endl;
		exit(-1);
	}
}

void iKnowUnitTests::Benjamin1(const char* pMessage) {
	string text_source_utf8 = u8"Risque d'exploitation";

	string normalized = iKnowEngine::NormalizeText(text_source_utf8, "fr");
	if (normalized != u8"risque d' exploitation")
		throw runtime_error("Benjamin1 : Wrong normalization for French.");
}

/// <summary>
/// After adding some PathRelevant entities and simple path expansion, I’ve compared the outputs between IRIS NLP and iknowpy. I’ve found one difference which seems to result from different ways that IRIS & iknowpy identiy lexreps.
/*
Sentence: また、大川小のある釜谷地区では住民と在勤者、来訪者計232人のうち、181人が犠牲となったとの調査結果を報告。

Lexrep identification for the part "232人のうち、181人が" in IRIS :
Lexrep("232") = Numeric
Lexrep("人") = JPCon + JPCount + JPRule3437 + Lit_人
Lexrep("のうち") = JPParticlePREPO
Lexrep("、") = JPComma + Lit_、
Lexrep("181") = Numeric
Lexrep("人") = JPCon + JPCount + JPRule3437 + Lit_人
Lexrep("が") = JPga + Lit_が

Lexrep identification for the same part in iknowpy :
LexrepIdentified:232 : Numeric;
LexrepIdentified:人:JPCon; JPRule3437; JPCount; Lit_人;
LexrepIdentified:のうち:JPParticlePREPO;
LexrepIdentified:、:JPComma; Lit_、;
LexrepIdentified:181人 : JPCon; JPNumber; Lit_1人;
LexrepIdentified:が:JPga; Lit_が;

As can be seen, “181人” is identified differently : IRIS identifies the whole chunk of numbers “181” first, whereas iknowpy identifies the lexrep “1人” first.This difference results in different indexing results for the character "が", now that it can sometimes be PathRelevant rather than NonRelevant.With the general left - to - right principle, the IRIS way should be kept.
*/
/// </summary>
/// <param name="pMessage"></param>
/*
+[125]	"LexrepIdentified:<lexrep id=29 type=Concept value=\"232\" index=\"232\" labels=\"Numeric;\" />;"	std::string
+[126]	"LexrepIdentified:<lexrep id=30 type=Unknown value=\"äºº\" index=\"äºº\" labels=\"JPCon;JPRule3437;JPCount;Lit_äºº;\" />;"	std::string
+[127]	"LexrepIdentified:<lexrep id=61 type=Unknown value=\"ã®ã†ã¡\" index=\"ã®ã†ã¡\" labels=\"JPParticlePREPO;\" />;"	std::string
+[128]	"LexrepIdentified:<lexrep id=34 type=Unknown value=\"ã€\" index=\"ã€\" labels=\"JPComma;Lit_ã€;\" />;"	std::string
+[129]	"LexrepIdentified:<lexrep id=35 type=Concept value=\"181\" index=\"181\" labels=\"Numeric;\" />;"	std::string
*/
void iKnowUnitTests::Issue104(const char* pMessage)
{
	iKnowEngine engine;
	String text_source = IkStringEncoding::UTF8ToBase(u8"また、大川小のある釜谷地区では住民と在勤者、来訪者計232人のうち、181人が犠牲となったとの調査結果を報告。");

	engine.index(text_source, "ja", true);
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
		if (it->find("LexrepIdentified") != string::npos && it->find("value=\"181\"") != string::npos) return;
	}
	throw std::runtime_error("Issue104 : wrong lookup for Japanese numericals !" + string(pMessage));
}

void iKnowUnitTests::Issue86(const char* pMessage) { // UDCertainty test with certainty level
	string text_source_utf8 = u8"he suggests that maybe we will be certain.";
	String text_source(IkStringEncoding::UTF8ToBase(text_source_utf8));

	iKnowEngine engine;
	UserDictionary user_dictionary;

	if (iKnowEngine::iknow_certainty_level_out_of_range == user_dictionary.addCertaintyLevel("suggests", 4)) // new: certaintly level
		throw std::runtime_error(string("Certainty Level out of range"));
	if (iKnowEngine::iknow_certainty_level_out_of_range == user_dictionary.addCertaintyLevel("maybe", 2)) // new: certaintly level
		throw std::runtime_error(string("Certainty Level out of range"));
	if (iKnowEngine::iknow_certainty_level_out_of_range == user_dictionary.addCertaintyLevel("certain", 9)) // new: certaintly level
		throw std::runtime_error(string("Certainty Level out of range"));

	engine.loadUserDictionary(user_dictionary);
	engine.index(text_source, "en", true); // traces should show UDPosSentiment

	if (engine.m_index.sentences.size() > 1) // "Fr." must not split the sentence
		throw std::runtime_error(string(pMessage));

	// Check for Positive Sentiment markers
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
		// cout << *it << endl;
		// +[12]	"UserDictionaryMatch:<lexrep id=6 type=Unknown value=\"er/pr\" index=\"er/pr\" labels=\"UDPosSentiment;ENCon;\" />;"	std::string
		// +[13]	"UserDictionaryMatch:<lexrep id=7 type=Unknown value=\"positive.\" index=\"positive\" labels=\"UDPosSentiment;ENCon;\" />;"	std::string
		if (it->find("UserDictionaryMatch") != string::npos) {
			string& trace_userdct = (*it);
			if (trace_userdct.find("suggests") != string::npos) {
				if (trace_userdct.find("UDCertainty") == string::npos)
					throw std::runtime_error(string(pMessage));
			}
			if (trace_userdct.find("maybe") != string::npos) {
				if (trace_userdct.find("UDCertainty") == string::npos)
					throw std::runtime_error(string(pMessage));
			}
			if (trace_userdct.find("certain") != string::npos) {
				if (trace_userdct.find("UDCertainty") == string::npos)
					throw std::runtime_error(string(pMessage));
			}
		}
	}
	engine.unloadUserDictionary();
}

void iKnowUnitTests::DP402269(const char* pMessage)
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"資源第一本部長（北米住友商事グループ北米資源・エネルギーグループ長）村井俊朗▽アジア大洋州総支配人補佐兼アジア大洋州住友商事グループアジア大洋州資源・化学品・メディア事業ユニット長兼アジア大洋州住友商事シンガポールユニット長（石油化学品）岡田卓也▽環境・インフラプロジェクト事業本部長（環境・インフラプロジェクト事業本部副本部長兼環境エネルギー事業第二）山埜英樹▽基礎化学品・エレクトロニクス本部副本部長、坂本好之▽ライフスタイル・リテイル事業本部副本部長兼リテイル＆ウェルネス事業部長（自動車リース事業部長）佐藤計▽電力インフラ事業本部副本部長兼電力事業第一、電池事業開発・野中紀彦▽軽金属・特殊鋼板本部副本部長、軽金属事業部長塩見圭吾▽中東支配人補佐兼イラン住友商事社長（地域総括部長）田中竹千代▽東アジア総代表補佐兼韓国住友商事社長（鋼板・建材本部長）若島浩▽米州総支配人補佐兼北米住友商事グループ北米化学品・エレクトロニクスグループ長兼米国住友商事に出向（ライフサイエンス本部長）須藤龍也▽同兼北米住友商事グループ北米資源・エネルギーグループ長兼米国住友商事に出向、林薫▽広報（ライフスタイル・リテイル事業本部長）新森健之▽電力インフラ事業本部長（電力インフラ事業本部副本部長兼タンジュン・ジャティＢプロジェクト）秋元勉▽ライフサイエンス本部長（中国住友商事グループ中国化学品・エレクトロニクス部門長兼上海住友商事蘇州事務所長）祐源通延▽欧阿中東ＣＩＳ総支配人補佐兼ＣＩＳ支配人兼ＣＩＳ住友商事社長（環境・インフラプロジェクト事業本部長）池村圭司▽地域総括部長（関西ブロック総括部長）出口雅敏▽建設不動産本部長兼総合建設開発（建設不動産本部副本部長兼不動産戦略事業部長）安藤伸樹▽船舶・航空宇宙・車輌事業本部副本部長兼船舶事業第二（欧州住友商事グループ欧州輸送機部門長）山口真▽ライフスタイル・リテイル事業本部長（ライフスタイル・リテイル事業本部副本部長兼ダイレクトマーケティング＆ソーシング事業部長）田中恵次▽アジア大洋州総支配人補佐兼インドネシア住友商事社長兼スラバヤ支店長（電力インフラ事業本部副本部長兼電力事業第一兼電力事業第二）佐橋明三▽東アジア総代表補佐兼中国住友商事グループ中国金属部門長兼上海住友商事社長、戸倉健夫▽基礎化学品・エレクトロニクス本部長（基礎化学品・エレクトロニクス本部副本部長兼資源性ケミカル第一）三輪聡▽自動車リース事業部長、自動車事業第一本部長中島正樹");
	engine.index(text_source, "ja");
	if (engine.m_index.sentences.size() == 0) {
		throw std::runtime_error("Long messages should not be skipped in non binary mode !" + string(pMessage));
	}
}

/* Issue#70
Example 1: rule 2158 in the English language model
2158; 50; typeRelation | .ENArtPosspron | *typeConcept+^ENList | ENComma | .ENArtPosspron | typeConcept + ^ ENNegation | ENAndOrBut+^"but" | .ENArtPosspron | typeConcept + ^ ENNegation | ENColon:SEnd; | +ENList | +ENList | +ENList | +ENList | +ENList | -ENNegStop + ENList | +ENList | +ENList | ;;

This rule has 10 elements(9 + SEnd), 3 of which are optional.
The rule fires for
"of sneezing, a sore throat and fatigue." -> 9 elements(8 + SEnd)
but not for:
"of sneezing, a headache and fatigue." -> 8 elements(7 + SEnd)

Example 2 : rule 2377 in the English language model
2377; 65; ENCertainty | .ENNegation | ENPBegin+ENCertStop+^ENConj | .^ENPBegin+^SEnd | ENPBegin:SEnd; || -ENCertStop | *| +ENCertStop;;

This rules has 5 elements, 2 of which are optional.
The rule fires for
"perhaps what else" -> 4 elements(3 + SEnd)
but not for:
"perhaps what" -> 3 elements(2 + SEnd)
*/
void iKnowUnitTests::Issue70(const char* pMessage)
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"perhaps what else");
	engine.index(text_source, "en", true);
	bool bRuleFires = false;
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces

		if (it->find("RuleApplication:rule_id=1261") != string::npos) {
			bRuleFires = true;
		}
	}
	if (bRuleFires) {
		bRuleFires = false;
		text_source = IkStringEncoding::UTF8ToBase(u8"perhaps what");
		engine.index(text_source, "en", true);
		for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces

			if (it->find("RuleApplication:rule_id=1261") != string::npos) {
				bRuleFires = true;
			}
		}
		if (!bRuleFires)
			throw std::runtime_error("Rule 1261 does *not* fire !" + string(pMessage));
	}
}

/* Issue#64 :
The input below contains some Greek characters which seem to mess up the detection of word boundaries. Boundaries (spaces) appear at the wrong positions, causing splitting and incomplete words. This is especially clear at the end of the sentence: the first 3 characters of the second sentence become part of the first sentence. The shift continues until the end of the input file.
The input file is UTF-8 encoded, as required.

input:
Syloïde blijkt een vergelijkbaar of zelfs groter effect te hebben op sommige parameters (bijv. 𝑎2, 𝑎3, 1 𝑡1 en 1 𝑡3) van de compressievergelijking. Dit vergelijkbare effect wordt echter vaak alleen bereikt bij een hogere concentratie Syloid in vergelijking met magnesiumstearaat.

output:
S1: Syloïde blijkt een vergelijkbaar of zelfs groter effect te hebben op sommige parameters (bijv. 𝑎2, 3, 1 𝑡1 en 1 3) van de om ressievergelijking. Dit
S2: ver elijkbare effect wor t ech er vaa < all> en ber ikt bij een hog re concentratie Syloid in ergelijking met mag esiumstearaat.
*/
void iKnowUnitTests::Issue64(const char* pMessage)
{
	iKnowEngine engine;

	// String text_source = IkStringEncoding::UTF8ToBase(u8"Syloïde blijkt een vergelijkbaar of zelfs groter effect te hebben op sommige parameters (bijv. 𝑎2, 𝑎3, 1 𝑡1 en 1 𝑡3) van de compressievergelijking. Dit vergelijkbare effect wordt echter vaak alleen bereikt bij een hogere concentratie Syloid in vergelijking met magnesiumstearaat.");
	String text_source = IkStringEncoding::UTF8ToBase(u8"𝑎2, 𝑎3, 1 𝑡1 en 1 𝑡3)");
	engine.index(text_source, "nl");
	for (auto it_sent = engine.m_index.sentences.begin(); it_sent != engine.m_index.sentences.end(); ++it_sent) {
		const Sentence& sent = *it_sent; // get sentence reference

		String SentenceText(&text_source[sent.offset_start()], &text_source[sent.offset_stop()]); // reconstruct the sentence
		std::string SentenceTextUtf8 = IkStringEncoding::BaseToUTF8(SentenceText); // convert it back to utf8
		if (SentenceTextUtf8 != u8"𝑎2, 𝑎3, 1 𝑡1 en 1 𝑡3)")
			throw std::runtime_error("Failed to reconstruct surrogate pair symbols !" + string(pMessage));

		/*
		for (auto it_entity = sent.entities.begin(); it_entity != sent.entities.end(); ++it_entity) {
			const Entity& entity = *it_entity;

			std::cout << entity.index_ << std::endl;
			String EntityText(&text_source[entity.offset_start_], &text_source[entity.offset_stop_]); // reconstruct the entity
			std::string EntityTextUtf8 = IkStringEncoding::BaseToUTF8(EntityText); // convert it back to utf8
			std::cout << EntityTextUtf8 << std::endl;
		}
		*/
	}
}

/* Reported by Saskia
In measurement attributes, if the first language to work with is not English, and later the engine switches to English, the value/unit extraction did not work anymore. This was due to a pointer comparison that did not work the way I expected.
Now the language code is consulted to detect a language switch.
*/
void iKnowUnitTests::Saskia4(const char* pMessage)
{
	iKnowEngine engine;

	String text_source_cs = IkStringEncoding::UTF8ToBase(u8"Do retrospektivní studie bylo zařazeno 167 pacientů s ASJ.");
	engine.index(text_source_cs, "cs");
	String text_source_en = IkStringEncoding::UTF8ToBase(u8"Cambridge police said the search was for a 31-year-old former Harvard student and a wanted suspect in a Philadelphia murder who was believed to be in the Cambridge area.");
	engine.index(text_source_en, "en");

	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = engine.m_index.sentences.begin()->sent_attributes.begin(); it_marker != engine.m_index.sentences.begin()->sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Measurement) {
			// if (attribute.value_ != string("31") || attribute.unit_ != string("year-old"))
			if (attribute.parameters_[0].first != string("31") || attribute.parameters_[0].second != string("year-old"))
				throw std::runtime_error("Measurement attribute not correct !" + string(pMessage));
		}
	}
}

/* Reported by Saskia
"This is a conviction under an old, old New York law that's not used much for criminal purposes," said Stephen Neal, a lawyer for McNenney.
IR: <attr type = "negation" literal = "that's not used" token = "not">
PY : <attr type = "negation" literal = "that's not used" token = "'s not">
*/
void iKnowUnitTests::Saskia3(const char* pMessage)
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"\"This is a conviction under an old, old New York law that's not used much for criminal purposes,\" said Stephen Neal, a lawyer for McNenney.");
	engine.index(text_source, "en");
	const Sentence& sent1 = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent1.sent_attributes.begin(); it_marker != sent1.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Negation) {
			if (attribute.marker_ != string("not"))
				throw std::runtime_error("Negation marker \"not\" not correct !" + string(pMessage));
		}
	}
}


/* Reported by Saskia
One day, while selling corn, Phiona looks through the crooked planks of a youth ministry, run by Robert Katende(David Oyelowo).
IR: <attr type="time" literal="One day," token="One day,">
PY: <attr type="time" literal="One day," token="One day, One day,">
*/
void iKnowUnitTests::Saskia2(const char* pMessage)
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"One day, while selling corn, Phiona looks through the crooked planks of a youth ministry, run by Robert Katende(David Oyelowo).");
	engine.index(text_source, "en");
	const Sentence& sent1 = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent1.sent_attributes.begin(); it_marker != sent1.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == DateTime) {
			if (attribute.marker_ != string("One day,"))
				throw std::runtime_error("DateTime marker \"One day,\" not correct !" + string(pMessage));
		}
	}

}

/* Reported by Saskia
S C3aR(-/-) mice on HFD are transiently resistant to diet-induced obesity during a 8 week period.[1]
IR: <attr type = "duration" literal = "8 week period." token = "8 week period.">
PY : <attr type = "duration" literal = "8 week period." token = "8 week">
*/
void iKnowUnitTests::Saskia1(const char* pMessage)
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"C3aR(-/-)\nmice on HFD are transiently resistant to diet - induced obesity during a 8 week\nperiod.");
	engine.index(text_source, "en");

	const Sentence& sent1 = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent1.sent_attributes.begin(); it_marker != sent1.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Duration) {
			if (attribute.marker_ != string("8 week period."))
				throw std::runtime_error("Duration marker \"8 week period.\" not correct !" + string(pMessage));
		}
	}
}

/* Issue#42
When an entity contains more than one marker of the same type, e.g.two Negation markers or two DateTime markers, the m_index property in the Python interface outputs them as two separate items.It would be better to collect them into one item.

Example 1 : Il n'y avaient jamais des chiens.
concerned entity : n'y avaient pas
attribute output :
[{'type': 'Negation', 'offset_start' : 5, 'offset_stop' : 8, 'marker' : "n'y", 'value' : '', 'unit' : '', 'value2' : '', 'unit2' : '', 'entity_ref' : 1},
{ 'type': 'Negation', 'offset_start' : 17, 'offset_stop' : 23, 'marker' : 'jamais', 'value' : '', 'unit' : '', 'value2' : '', 'unit2' : '', 'entity_ref' : 1 }]

Example 2 : These reports are for the 1997 - 1998 academic year.
concerned entity : 1997 - 1998 academic year
attribute output :
[{'type': 'DateTime', 'offset_start' : 28, 'offset_stop' : 37, 'marker' : '1997-1998', 'value' : '', 'unit' : '', 'value2' : '', 'unit2' : '', 'entity_ref' : 4}, 
{ 'type': 'DateTime', 'offset_start' : 47, 'offset_stop' : 52, 'marker' : 'year.', 'value' : '', 'unit' : '', 'value2' : '', 'unit2' : '', 'entity_ref' : 4 }]
*/
void iKnowUnitTests::Issue42(const char* pMessage) // https://github.com/intersystems/iknow/issues/42
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"Il n'y avaient jamais des chiens.");
	engine.index(text_source, "fr");
	const Sentence& sent1 = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent1.sent_attributes.begin(); it_marker != sent1.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Negation) {
			if (attribute.marker_ != string("n'y jamais"))
				throw std::runtime_error("Negation marker not correct !" + string(pMessage));
		}
	}
	text_source = IkStringEncoding::UTF8ToBase(u8"These reports are for the 1997 - 1998 academic year.");
	engine.index(text_source, "en");
	const Sentence& sent2 = *engine.m_index.sentences.begin(); // get sentence reference
	count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent2.sent_attributes.begin(); it_marker != sent2.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == DateTime) {
			if (!(attribute.marker_ == string("1997") || attribute.marker_ == string("1998 year.")))
				throw std::runtime_error("DateTime marker not correct !" + string(pMessage));
		}
	}
}


/* Issue#39
* The Frequency attribute for 'daily' is missing in the RAW output for the following example:
input:
60 mg daily
-> attributes: attr type="measurement" literal="60 mg daily" token="60 mg" value="60" unit="mg"

While avoiding giving specific sales figures, Bond told reporters that Asda's 2007 total sales came in at 8%-10%, and about 1% ahead of its target.
IR: <attr type="measurement" literal="8%-10%," token="8%-10%," value="8" unit="%" value2="10" unit2="%">
GH: <attr type="measurement" literal="8%-10%," token="8%-10%," value="8%-10%">

On   31-MAY-2001,   after   over a year   without  any  pain killers,  the  patient   took   two "twenty milligram" tablets   and  the  next day,   "all hell   broke   loose."
IR: <attr type="measurement" literal="two "twenty milligram" tablets" token="two "twenty milligram"" value="two" value2="twenty" unit2="milligram">
GH: <attr type="measurement" literal="two "twenty milligram" tablets" token="two "twenty milligram"" value="two" unit="milligram" value2="twenty">

*/
void iKnowUnitTests::Issue39(const char* pMessage) // https://github.com/intersystems/iknow/issues/39
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"60 mg daily");
	engine.index(text_source, "en");
	const Sentence& sent = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent.sent_attributes.begin(); it_marker != sent.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Measurement) {
			// if (attribute.value_ != string("60") || attribute.unit_ != string("mg"))
			if (attribute.parameters_[0].first != string("60") || attribute.parameters_[0].second != string("mg"))
				throw std::runtime_error("Measurement attribute not correct !" + string(pMessage));
		}
		if (attribute.type_ == Frequency) {
			if (attribute.marker_ != string("daily"))
				throw std::runtime_error("Frequency attribute not correct !" + string(pMessage));
		}
	}
	String text2_source = IkStringEncoding::UTF8ToBase(u8"While avoiding giving specific sales figures, Bond told reporters that Asda's 2007 total sales came in at 8%-10%, and about 1% ahead of its target.");
	engine.index(text2_source, "en");
	const Sentence& sent2 = *engine.m_index.sentences.begin(); // get sentence reference
	count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent2.sent_attributes.begin(); it_marker != sent2.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Measurement) {
			// if (attribute.value_ != string("8") || attribute.unit_ != string("%") || attribute.value2_ != string("10") || attribute.unit2_ != string("%"))
			if (attribute.parameters_[0].first != string("8") || attribute.parameters_[0].second != string("%") || attribute.parameters_[1].first != string("10") || attribute.parameters_[1].second != string("%"))
				throw std::runtime_error("Measurement attribute not correct !" + string(pMessage));
			break;
		}
	}
	String text3_source = IkStringEncoding::UTF8ToBase(u8"On   31-MAY-2001,   after   over a year   without  any  pain killers,  the  patient   took   two \"twenty milligram\" tablets   and  the  next day,   \"all hell   broke   loose.\"");
	engine.index(text3_source, "en");
	const Sentence& sent3 = *engine.m_index.sentences.begin(); // get sentence reference
	count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent3.sent_attributes.begin(); it_marker != sent3.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Measurement) {
			// if (attribute.value_ != string("two") || attribute.unit_ != string("") || attribute.value2_ != string("twenty") || attribute.unit2_ != string("milligram"))
			if (attribute.parameters_[0].first != string("two") || attribute.parameters_[0].second != string("") || attribute.parameters_[1].first != string("twenty") || attribute.parameters_[1].second != string("milligram"))
				throw std::runtime_error("Measurement attribute not correct !" + string(pMessage));
			break;
		}
	}

}

/* Issue#41
SThe baby weighs 7 pounds 7 ounces.
<attr type = "measurement" literal = "7 pounds 7 ounces." token = "7 pounds 7 ounces." value = "7" unit = "pounds" value2 = "7" unit2 = "ounces">
*/
void iKnowUnitTests::Issue41(const char* pMessage) // https://github.com/intersystems/iknow/issues/41
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"The baby weighs 7 pounds 7 ounces.");
	engine.index(text_source, "en");
	const Sentence& sent = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent.sent_attributes.begin(); it_marker != sent.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Measurement) {
			// if (attribute.value_ != string("7") || attribute.unit_ != string("pounds") || attribute.value2_ != string("7") || attribute.unit2_ != "ounces")
			if (attribute.parameters_[0].first != string("7") || attribute.parameters_[0].second != string("pounds") || attribute.parameters_[1].first != string("7") || attribute.parameters_[1].second != "ounces")
				throw std::runtime_error("Measurement attribute not correct !" + string(pMessage));
		}
	}
}

/* Issue#37
The Certainty attribute in the English language model has a marker, spanand level.When using the m_index property in the Python interface, the marker can be found through['sent_attributes'], the span through['path_attributes'].The level, currently either 0 (uncertain) or 9 (certain), should be in['sent_attributes'] too, but it is missing.
Example :
	Input = "This might be a problem."
	['sent_attributes'] = [{'type': 'Certainty', 'offset_start' : 7, 'offset_stop' : 12, 'marker' : 'might', 'value' : '', 'unit' : '', 'value2' : '', 'unit2' : '', 'entity_ref' : 1}]
*/
void iKnowUnitTests::test8(const char* pMessage) // https://github.com/intersystems/iknow/issues/37
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"This might be a problem.");
	engine.index(text_source, "en");
	const Sentence& sent = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent.sent_attributes.begin(); it_marker != sent.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		if (attribute.type_ == Certainty && attribute.parameters_[0].first != string("0"))
			throw std::runtime_error("Certainty attribute has no level 0 defined !" + string(pMessage));
	}
}

/*
* If Furigana handling in Japanese is switched off ("FuriganaHandling;off" in metadata.csv), this test will ***FAIL***
*/
void iKnowUnitTests::test7(const char* pMessage) // https://github.com/intersystems/iknow/issues/31
{
	iKnowEngine engine;

	string text_source_utf8 = u8"北海道・阿寒（あかん）湖温泉で自然体験ツアーに出かけた。";
	String text_source = IkStringEncoding::UTF8ToBase(text_source_utf8); //  = > All Hiragana
	engine.index(text_source, "ja", true);
	bool bFurigana = false;
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
		if (it->find("LexrepCreated") != string::npos && it->find("type=Nonrelevant") != string::npos) {
			if (it->find(u8"index=\"（あかん）\"") != string::npos)
				bFurigana = true;
		}
	}
	if (bFurigana == false)
		throw std::runtime_error("Furigana ***not*** detected !" + string(pMessage));

	text_source = IkStringEncoding::UTF8ToBase(u8"黎智英（ジミー・ライ）氏や民主活動家の周庭（アグネス・チョウ）氏が逮捕された。"); //  = > All Katakana
	bFurigana = false;
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
		if (it->find("LexrepCreated") != string::npos && it->find("type=Nonrelevant") != string::npos) {
			if (it->find(u8"index=\"（ジミー・ライ）\"") != string::npos)
				bFurigana = true;
		}
	}
	if (bFurigana == true)
		throw std::runtime_error("Furigana detected in all Katakana text!" + string(pMessage));

	text_source = IkStringEncoding::UTF8ToBase(u8"将棋の高校生プロ、藤井聡太棋聖（18）がまたしても金字塔を打ち立てた。"); // = > All Numbers
	bFurigana = false;
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
		if (it->find("LexrepCreated") != string::npos && it->find("type=Nonrelevant") != string::npos) {
			if (it->find(u8"index=\"（18）\"") != string::npos)
				bFurigana = true;
		}
	}
	if (bFurigana == true)
		throw std::runtime_error("Furigana detected in all Numeric text!" + string(pMessage));
}
/*
The Certainty attribute in the English language model has a marker, spanand level.When using the m_index property in the Python interface, the marker can be found through['sent_attributes'], the span through['path_attributes'].The level, currently either 0 (uncertain) or 9 (certain), should be in['sent_attributes'] too, but it is missing.
Example :
	Input = "This might be a problem."
	['sent_attributes'] = [{'type': 'Certainty', 'offset_start' : 7, 'offset_stop' : 12, 'marker' : 'might', 'value' : '', 'unit' : '', 'value2' : '', 'unit2' : '', 'entity_ref' : 1}]
*/
// std::string NormalizeText(std::string& text_source, const std::string& language, bool bUserDct = false, bool bLowerCase = true, bool bStripPunct = true);
void iKnowUnitTests::test6(const char* pMessage) {
	string text_source = u8"WE WANT THIS TEXT LOWERCASED !";
	string text_lowercased = iKnowEngine::NormalizeText(text_source, "en");

	if (std::count_if(text_lowercased.begin(), text_lowercased.end(), [](unsigned char c) { return isupper(c); }) > 0) // 
		throw std::runtime_error("NormalizeText does not work correctly");
}

void iKnowUnitTests::test5(const char* pMessage) { // User DCT test 
	string text_source_utf8 = u8"The Fr. test was er/pr positive after a long week.";
	String text_source(IkStringEncoding::UTF8ToBase(text_source_utf8));

	iKnowEngine engine;
	UserDictionary user_dictionary;
	user_dictionary.addLabel("er/pr positive", "UDPosSentiment"); // : @er/pr positive,UDPosSentiment
	if (user_dictionary.addLabel("some text", "LabelThatDoesNotExist") != iKnowEngine::iknow_unknown_label)
		throw std::runtime_error(string("Unknow label *not* triggered !"));
	user_dictionary.addLabel("a long week", "UDConcept;UDTime"); // : multiple UD labels can be combined

	user_dictionary.addSEndCondition("Fr.", false); // ;;Ph.D.;0;
	engine.loadUserDictionary(user_dictionary);
	engine.index(text_source, "en", true); // traces should show UDPosSentiment

	if (engine.m_index.sentences.size() > 1) // "Fr." must not split the sentence
		throw std::runtime_error(string(pMessage));

	// Check for Positive Sentiment markers
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
		// cout << *it << endl;
		// +[12]	"UserDictionaryMatch:<lexrep id=6 type=Unknown value=\"er/pr\" index=\"er/pr\" labels=\"UDPosSentiment;ENCon;\" />;"	std::string
		// +[13]	"UserDictionaryMatch:<lexrep id=7 type=Unknown value=\"positive.\" index=\"positive\" labels=\"UDPosSentiment;ENCon;\" />;"	std::string
		if (it->find("UserDictionaryMatch") != string::npos) {
			string& trace_userdct = (*it);
			if (trace_userdct.find("value=\"er/pr\"") != string::npos) {
				if (trace_userdct.find("UDPosSentiment") == string::npos)
					throw std::runtime_error(string(pMessage));
			}
			if (trace_userdct.find("value=\"positive.\"") != string::npos) {
				if (trace_userdct.find("UDPosSentiment") == string::npos)
					throw std::runtime_error(string(pMessage));
			}
			if (trace_userdct.find("a long week") != string::npos) {
				if (trace_userdct.find("UDConcept;UDTime;") == string::npos)
					throw std::runtime_error(string(pMessage));
			}
		}
	}
	engine.unloadUserDictionary();

	user_dictionary.clear();
	user_dictionary.addLabel("some text", "UDUnit");
	user_dictionary.addSEndCondition("Fr.", false);
	user_dictionary.addConceptTerm("one concept");
	user_dictionary.addRelationTerm("one relation");
	user_dictionary.addNonrelevantTerm("crap");
	user_dictionary.addNegationTerm("w/o");
	user_dictionary.addPositiveSentimentTerm("great");
	user_dictionary.addNegativeSentimentTerm("awfull");
	user_dictionary.addUnitTerm("Hg");
	user_dictionary.addNumberTerm("magic number");
	user_dictionary.addTimeTerm("future");

	engine.loadUserDictionary(user_dictionary);
	String text_source2(IkStringEncoding::UTF8ToBase("some text Fr. w/o one concept and crap one relation that's great and awfull, magic number 3 Hg from future"));

	engine.index(text_source2, "en", true); // generate Traces
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
			if (it->find("UserDictionaryMatch") != string::npos) {
				string& trace_userdct = (*it);
				if (trace_userdct.find("value=\"w/o\"") != string::npos) {
					if (trace_userdct.find("UDNegation") == string::npos)
						throw std::runtime_error(string(pMessage));
				}
				if (trace_userdct.find("value=\"awfull,\"") != string::npos) {
					if (trace_userdct.find("UDNegSentiment") == string::npos)
						throw std::runtime_error(string(pMessage));
				}
				if (trace_userdct.find("value=\"Hg\"") != string::npos) {
					if (trace_userdct.find("UDUnit") == string::npos)
						throw std::runtime_error(string(pMessage));
				}
			}
	}

	// User dictionary is static, if not unloaded, it stays active.
	engine.unloadUserDictionary();
}

void iKnowUnitTests::test4(const char* pMessage) { // Naomi detects missing SBegin/SEnd labels
	string text_source_utf8 = u8"The position of the window made it very unlikely that this was a random passerby.";
	String text_source(IkStringEncoding::UTF8ToBase(text_source_utf8));
	iKnowEngine engine;
	engine.index(text_source, "en", true); // traces should contain SBegin/SEnd labels
	for (auto it = engine.m_traces.begin(); it != engine.m_traces.end(); ++it) { // scan the traces
		if (it->find("LexrepIdentified") != string::npos) {
			string& trace_sbegin = (*it);
			if (trace_sbegin.find("labels=\"SBegin;\"") == string::npos) // first should be SBegin
				throw std::runtime_error(string(pMessage));
			while ((it + 1)->find("LexrepIdentified") != string::npos)
				++it;
			string& trace_send = (*it);
			if (trace_send.find("labels=\"SEnd;\"") == string::npos) // last should be SEnd
				throw std::runtime_error(string(pMessage));
		}
	}
	const Sentence& sent = *engine.m_index.sentences.begin(); // get the sentence reference
	const Path_Attribute& attribute_expansion = *sent.path_attributes.begin(); // path attribute
	if (attribute_expansion.type != Certainty)
		throw std::runtime_error(string("No certainty attribute detected !"));
	if (attribute_expansion.pos != 5)
		throw std::runtime_error(string("Position of certainty attribute must be 5 !"));
	if (attribute_expansion.span != 5) // TODO: this value is *not* correct.
		throw std::runtime_error(string("Span of certainty attribute must be 5 !"));
}

void iKnowUnitTests::test3(const char* pMessage) { // Only one measurement attribute in example text : verify correctness
	string text_source_utf8 = "Now, I have been on many walking holidays, but never on one where I have my bags ferried\nfrom hotel to hotel while I simply get on with the job of walkingand enjoying myself.";
	// <attr type = "measurement" literal = "hundreds of feet" token = "hundreds of feet" value = "hundreds of" unit = "feet">
	String text_source(IkStringEncoding::UTF8ToBase(text_source_utf8));
	iKnowEngine engine;
	engine.index(text_source, "en");

	const Sentence& sent = *engine.m_index.sentences.begin(); // get sentence reference
	int count_attributes = 0;
	for (AttributeMarkerIterator it_marker = sent.sent_attributes.begin(); it_marker != sent.sent_attributes.end(); ++it_marker, ++count_attributes) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		size_t start_literal = attribute.offset_start_;
		size_t stop_literal = attribute.offset_stop_;
		String literal(&text_source[start_literal], &text_source[stop_literal]);

		string strMarker = attribute.marker_;
		int count_upper = (int) std::count_if(strMarker.begin(), strMarker.end(), [](unsigned char c) { return std::isupper(c); }); // Useless, I know...
	}
	if (count_attributes!=4) {
		throw std::runtime_error(string(pMessage));
	}

}

void iKnowUnitTests::test2(const char* pMessage) { // Only one measurement attribute in example text : verify correctness
	string text_source_utf8 = "The only sound as we followed the narrow hillside path through the firs and pines was that of the sparkling green Soca river crashing its way through the gorge hundreds of feet below. ";
	// <attr type = "measurement" literal = "hundreds of feet" token = "hundreds of feet" value = "hundreds of" unit = "feet">
	String text_source(IkStringEncoding::UTF8ToBase(text_source_utf8));
	iKnowEngine engine;
	engine.index(text_source, "en");

	const Sentence& sent = *engine.m_index.sentences.begin(); // get sentence reference
	for (AttributeMarkerIterator it_marker = sent.sent_attributes.begin(); it_marker != sent.sent_attributes.end(); ++it_marker) { // iterate over sentence attributes
		const Sent_Attribute& attribute = *it_marker;

		size_t start_literal = attribute.offset_start_;
		size_t stop_literal = attribute.offset_stop_;
		String literal(&text_source[start_literal], &text_source[stop_literal]);

		if (literal != u"hundreds of feet")
			throw std::runtime_error(string(pMessage));
		string marker = attribute.marker_;
		if (marker != "hundreds of feet")
			throw std::runtime_error(string(pMessage));
		string value = attribute.parameters_[0].first;
		if (value != "hundreds of")
			throw std::runtime_error(string(pMessage));
		string unit = attribute.parameters_[0].second;
		if (unit != "feet")
			throw std::runtime_error(string(pMessage));
	}

}
void iKnowUnitTests::test1(const char *pMessage) { // Japanese text should produce entity vectors
	String text_source = {
		0x3053, 0x3093, 0x306A, 0x53F0, 0x672C, 0x3067, 0x30D7, 0x30ED, 0x30C3, 0x30C8, 0x3055, 0x308C, 0x3066, 0x306F, 0x56F0, 0x308B, 0x3001, 0x3068, 0x5148, 0x751F, 0x8A00, 0x3063, 0x305F, 0x3002, 0x5FD7, 0x671B, 0x5B66, 0x90E8, 0x306E, 0x6C7A, 0x5B9A, 0x6642,	0x671F,
		0x306B, 0x3064, 0x3044, 0x7D4C, 0x55B6, 0x95A2, 0x3057, 0x8868, 0xFF08, 0xFF09, 0x6E08, 0x793A, 0x3059, 0x3060, 0x5916, 0x56FD, 0x4EBA, 0x5165, 0x8A66, 0x30B9, 0x30DD, 0x30FC, 0x30C4, 0x63A8, 0x85A6, 0x6A19, 0x5927, 0x304D, 0x304C, 0x5C0F, 0x304F,	0x30DF, 0x30EA,
		0x30C7, 0x30A3, 0x30F3, 0x30B0, 0x3092, 0x907F, 0x3051, 0x3081, 0x9664, 0x3042, 0x6982, 0x89B3, 0x5206, 0x304B, 0x3069, 0x533A, 0x304A, 0x3082, 0x9AD8, 0x6821, 0x5E74, 0x6700, 0x666E, 0x901A, 0x70B9, 0x4E00, 0x65B9, 0x822C, 0x30BB, 0x30BF,	0x5229, 0x7528, 0x5408,
		0x683C, 0x5F8C, 0x3084, 0x3046, 0x7FA4, 0x7387, 0x307F, 0x975E, 0x5E38, 0x9054, 0x3089, 0x89E3, 0x91C8, 0x6CE8, 0x610F, 0x8981, 0x6570, 0x5024, 0x4EE5, 0x4E0A, 0x53D7, 0x9A13, 0x6BB5, 0x968E, 0x4F75, 0x9858, 0x3088, 0x308A, 0x767A,	0x8005, 0x3061, 0x4E2D, 0x5275,
		0x4FA1, 0x3070, 0x826F, 0x8003, 0x3048, 0x300C, 0x307E, 0x6765, 0x300D, 0x591A, 0x5B58, 0x5728, 0x30EB, 0x52C9, 0x5BFE, 0x554F, 0x984C, 0x62B1, 0x53EF, 0x80FD, 0x6027, 0x5341, 0x529B, 0x30EC, 0x30D9, 0x52D5, 0x6A5F, 0x9762,	0x898B, 0x5023, 0x308D, 0x0000 
	};
	/*
	std::string SentenceTextUtf8 = IkStringEncoding::BaseToUTF8(text_source); // convert it back to utf8
	cout << "Text Source:" << endl << SentenceTextUtf8 << endl;
	*/
	iKnowEngine engine;
	engine.index(text_source, "ja");
	if (engine.m_index.sentences.empty())
		throw std::runtime_error(std::string(pMessage));
	if (engine.m_index.sentences[0].sent_attributes[0].type_ != EntityVector)
		throw std::runtime_error(std::string(pMessage));
	if (engine.m_index.sentences[0].path.empty()) {
		throw std::runtime_error(std::string("Engine *must* generate Path data, even for Japanese"));
	}
	map<size_t, string> mapTextSource;
	map<size_t, double> mapDominantConcepts;
	for (SentenceIterator it_sent = engine.m_index.sentences.begin(); it_sent != engine.m_index.sentences.end(); ++it_sent) { // loop over the sentences
		for_each(it_sent->entities.begin(), it_sent->entities.end(), 
			[&mapTextSource, &mapDominantConcepts](const Entity& entity) { mapTextSource[entity.entity_id_] = entity.index_; mapDominantConcepts[entity.entity_id_] = entity.dominance_value_; }); // collect the entities
	}
	// cout << "Text Source Proximity Overview :" << endl;
	// typedef std::pair<std::pair<EntityId, EntityId>, Proximity> ProximityPair_t; // single proximity pair
	if (engine.m_index.proximity.empty())
		throw std::runtime_error(string("*** Missing Proximity data ***"));

	for (Text_Source::Proximity::iterator itProx = engine.m_index.proximity.begin(); itProx != engine.m_index.proximity.end(); ++itProx) {
		size_t id1 = itProx->first.first;
		size_t id2 = itProx->first.second;
		double proximity = static_cast<double>(itProx->second);

		// cout << "\"" << mapTextSource[id1] << "\":\"" << mapTextSource[id2] << "\"=" << proximity << endl;

		if (itProx - engine.m_index.proximity.begin() > 12) // limit proximity output
			break;
	}
	// cout << "Text Source Proximity Overview :" << endl;
	// Top 10 dominant terms :
	typedef pair<int, double> EntDomType;
	vector<EntDomType> vecDominantConcepts;
	for_each(mapDominantConcepts.begin(), mapDominantConcepts.end(), [&vecDominantConcepts](pair<const size_t,double>& ent_par) { vecDominantConcepts.push_back(make_pair((int)ent_par.first, (double)ent_par.second)); });
	sort(vecDominantConcepts.begin(), vecDominantConcepts.end(), [](EntDomType& a, EntDomType& b) { return a.second > b.second;  });
	for (vector<EntDomType>::iterator itDom = vecDominantConcepts.begin(); itDom != vecDominantConcepts.end(); ++itDom) {
		// cout << "\"" << mapTextSource[itDom->first] << "\" DOM=" << itDom->second << endl;

		if (itDom - vecDominantConcepts.begin() == (10 - 1)) // limit to 10
			break;
	}
	if (vecDominantConcepts.empty())
		throw std::runtime_error(string("*** Missing Dominance data ***"));
}
