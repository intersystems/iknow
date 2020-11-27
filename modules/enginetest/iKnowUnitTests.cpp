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
			if (attribute.value_ != string("60") || attribute.unit_ != string("mg"))
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
			if (attribute.value_ != string("8") || attribute.unit_ != string("%") || attribute.value2_ != string("10") || attribute.unit2_ != string("%"))
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
			if (attribute.value_ != string("two") || attribute.unit_ != string("") || attribute.value2_ != string("twenty") || attribute.unit2_ != string("milligram"))
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
			if (attribute.value_ != string("7") || attribute.unit_ != string("pounds") || attribute.value2_ != string("7") || attribute.unit2_ != "ounces")
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

		if (attribute.type_ == Certainty && attribute.value_ != string("0"))
			throw std::runtime_error("Certainty attribute has no level 0 defined !" + string(pMessage));
	}
}

/*
* If Furigana handling in Japanese is switched off ("FuriganaHandling;off" in metadata.csv), this test will ***FAIL***
*/
void iKnowUnitTests::test7(const char* pMessage) // https://github.com/intersystems/iknow/issues/31
{
	iKnowEngine engine;

	String text_source = IkStringEncoding::UTF8ToBase(u8"北海道・阿寒（あかん）湖温泉で自然体験ツアーに出かけた。"); //  = > All Hiragana
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
		string value = attribute.value_;
		if (value != "hundreds of")
			throw std::runtime_error(string(pMessage));
		string unit = attribute.unit_;
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
	if (engine.m_index.sentences[0].path.empty()) {
		throw std::runtime_error(std::string(pMessage));
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
