﻿/*
** enginetest.cpp : testing the iKnow engine
*/
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <map>

#include <nlohmann/json.hpp>
using nlohmann::json;

#include "engine.h"
#include "iKnowUnitTests.h"

using namespace std;

using iknow::base::String;
using iknow::base::IkStringEncoding;
using namespace iknowdata;

inline String ucs2(const char* input_text) {
	return String(IkStringEncoding::UTF8ToBase(input_text));
}
String getSampleText(std::string language_code) { // must be ucs-2 encoded text.
	if (language_code == "en") return ucs2(u8"Be the change you want to see in life.");
	if (language_code == "de") return ucs2(u8"Oder die Erkundung der Natur - und zwar ohne Anleitung.");
	if (language_code == "ru") return ucs2(u8"Микротерминатор может развивать скорость до 30 сантиметров за секунду, пишут калининградские СМИ.");
	if (language_code == "es") return ucs2(u8"En Argentina no hay estudios previos reportados en cuanto a la elaboración de vinos cítricos ni de «vino de naranja».");
	if (language_code == "fr") return ucs2(u8"En pratique comment le faire ?");
	if (language_code == "ja") return ucs2(u8"こんな台本でプロットされては困る、と先生は言った。");
	if (language_code == "nl") return ucs2(u8"Op basis van de afzonderlijke evaluatieverslagen stelt de Commissie een synthese op communautair niveau op.");
	if (language_code == "pt") return ucs2(u8"Distingue-se o mercado de um produto ou serviço dos mercados de fatores de produção, capital e trabalho.");
	if (language_code == "sv") return ucs2(u8"Jag är bäst i klassen. Ingen gör efter mig, kan jag inte lämna. Var försiktig, är gräset alltid grönare på andra sidan.");
	if (language_code == "uk") return ucs2(u8"грошових зобов'язань, прийнятих на себе згідно з умов цього договору.");
	if (language_code == "cs") return ucs2(u8"Létající jaguár je novela spisovatele Josefa Formánka z roku 2004.");

	return ucs2(u8"Time flies like an arrow, fruit flies like a banana");
}

//
// Functor to show the type and index (normalized lexical representation) of the entity
//
struct handleEntity
{
	handleEntity(std::ofstream& os) : os_(os) {}

	void operator()(Entity& ent) {
		char t = ent.type_ == Entity::Concept ? 'c' : (ent.type_ == Entity::Relation ? 'r' : (ent.type_ == Entity::PathRelevant ? 'p' : 'n'));
		std::cout << t;
		os_ << "<" << t << ">" << ent.index_ << "</" << t << ">";

	}
	std::ofstream& os_;
};

//
// Functor that handles a sentence : basically a list of entities, each one is processed by "handleEntity"
//
struct handleSentence
{
	handleSentence(std::ofstream& os) : os_(os) {}

	void operator()(Sentence& sent) {
		std::cout << "S:" << sent.offset_start() << ":" << sent.offset_stop() << ";" << std::endl;
		handleEntity entity_handler(os_);
		for_each(sent.entities.begin(), sent.entities.end(), entity_handler);
		std::cout << std::endl;
		os_ << std::endl;
	}
	std::ofstream& os_;
};

//
// Functor that covers a specific language, a representative text is collected, and send to the iknow indexer. The resulting
// sentences are then processed by "handleSentence"
//
struct handleLanguage
{
	handleLanguage(iKnowEngine& engine, std::ofstream& os) : engine_(engine), os_(os) {}

	void operator()(const std::string& language) {
		std::cout << "Handling language = \"" << language << "\"" << std::endl;
		String text_source = getSampleText(language);
		bool b_traces = false;
#ifdef _DEBUG // debug version generates linguistic traces
		b_traces = true;
#endif
		engine_.index(text_source, language, b_traces); // main iKnow indexing : text as first parameter, language specifier as second.
		handleSentence sentence_handler(os_);
		for_each(engine_.m_index.sentences.begin(), engine_.m_index.sentences.end(), sentence_handler); // handle linguistic output generated by the iKnow engine.
		if (b_traces) { // write linguistic trace file
			std::ofstream os("iknowtrace.log", std::ofstream::app);
			if (os.is_open()) {
				os << "\xEF\xBB\xBF" << std::endl; // Force utf8 header, maybe utf8 is not the system codepage
				for_each(engine_.m_traces.begin(), engine_.m_traces.end(), [&os](string& trace_) { os << trace_ << endl; });
			}
			os.close();
		}
	}

	iKnowEngine& engine_;
	std::ofstream& os_;
};

//
// a_short_demo shows the step in indexing and retrieving results.
//
void a_short_demo(void);
void testing_json_c_interface(void);

int main(int argc, char* argv[])
{
	testing::iKnowUnitTests::runUnitTests(); // first, run existing unit tests
	
	// currently supported languages : { "en", "de", "ru", "es", "fr", "ja", "nl", "pt", "sv", "uk", "cs" };
	const std::set<std::string> languages_set = iKnowEngine::GetLanguagesSet();

	try {
		std::cout << "iKnowEngine test program" << std::endl << std::endl;
		std::ofstream os("iknowdata.log"); // logging file for indexing results
		os << "\xEF\xBB\xBF"; // Force utf8 header, maybe utf8 is not the system codepage, for std::cout, use "chcp 65001" to switch

		iKnowEngine myEngine; // iKnow engine constructor
		handleLanguage language_handler(myEngine,os);
		for_each(languages_set.begin(), languages_set.end(), language_handler);

		os.close();

		a_short_demo();
		testing_json_c_interface();

		cout << endl << "*** All tests passed succesfully ***" << endl;
	}
	catch (std::exception& e)
	{
		cerr << e.what() << endl;
	}
	catch (...) {
		cerr << "Smart Indexer failed..." << std::endl;
	}

	return 0;
}

void a_short_demo(void)
{
	std::cout << std::endl << "Indexing demo program" << std::endl << std::endl;

	std::string demo_text("This was not to be expected. The variance we measured was in between 5%-82% of the total amount. That was not a good sign !");
	//
	// The text source must be UCS-2 (Unicode 2byte) encoded, we call that "Base", the engine has utility functions for that kind of conversion.
	// So first we do the conversion into the iknow::base::String (wchar_t) type :
	//
	String Text_Source(IkStringEncoding::UTF8ToBase(demo_text));
	//
	// All text offsets will be based on this Text_Source.
	// Now we need an engine object :
	//
	iKnowEngine engine;
	//
	// The engine works synchronously, the text will be indexed as a unit, in one step. Results can only be retrieved afterwards.
	// It's also working single threaded, multiple threads will be muted, only one thread at the time will be processed.
	// Now we can index the text, using the English ("en") language model :
	//
	engine.index(Text_Source, "en");
	//
	// And after indexing, we can inspect the results:
	//
	int sentence_counter = 1;
	for (SentenceIterator it_sent = engine.m_index.sentences.begin(); it_sent != engine.m_index.sentences.end(); ++it_sent) { // loop over the sentences
		const Sentence& sent = *it_sent; // get a sentence reference

		const size_t start = sent.offset_start(); // start position of the sentence
		const size_t stop = sent.offset_stop(); // stop position of the sentence

		String SentenceText(&Text_Source[start], &Text_Source[stop]); // reconstruct the sentence
		std::string SentenceTextUtf8 = IkStringEncoding::BaseToUTF8(SentenceText); // convert it back to utf8
		std::cout << sentence_counter++ << ":\"" << SentenceTextUtf8 << "\"" << std::endl; // send it to the console

		//
		// The sentence has been split up in entities : concept, relation, path-relevant and non-relevant
		//
		std::cout << std::endl;
		for (EntityIterator it_entity = sent.entities.begin(); it_entity != sent.entities.end(); ++it_entity) { // loop over the entities
			const Entity& entity = *it_entity;

			// translate the entity type to 'c' for concept, 'r' for relation, 'p' for path-relevant and 'n' for non-relevant
			char t = entity.type_ == Entity::Concept ? 'c' : (entity.type_ == Entity::Relation ? 'r' : (entity.type_ == Entity::PathRelevant ? 'p' : 'n'));
			
			std::cout << " " << t << ":\"" << entity.index_ << "\"" << std::endl; // send type and normalized lexical representation to the console
		}
		std::cout << std::endl;

		//
		// If attributes are detected, they are collected in sent.attributes
		//
		for (AttributeMarkerIterator it_marker = sent.sent_attributes.begin(); it_marker != sent.sent_attributes.end(); ++it_marker) { // iterate over sentence attributes
			const Sent_Attribute& attribute = *it_marker;

			std::string a_type = AttributeName(attribute.type_); // translate the attribute type
			if (attribute.type_== Attribute::Measurement) {
				std::cout << a_type << ":\"" << attribute.marker_ << "\" ";

				string value = attribute.parameters_[0].first;
				string unit = attribute.parameters_[0].second;
				string value2 = attribute.parameters_[1].first;
				string unit2 = attribute.parameters_[1].second;

				std::cout << (!value.empty() ? "val=\"" + value + "\" ": "");
				std::cout << (!unit.empty() ? "unit=\"" + unit + "\" " : "");
				std::cout << (!value2.empty() ? "val2=\"" + value2 + "\" " : "");
				std::cout << (!unit2.empty() ? "unit2=\"" + unit2 + "\" " : "");
				std::cout << std::endl;
				continue; 
			}

			std::cout << a_type << ":\"" << attribute.marker_ << "\"" << std::endl;
		}
		std::cout << std::endl;

		//
		// Path output
		//
		std::cout << "Sentence path: ";
		for (PathIterator it_path = sent.path.begin(); it_path != sent.path.end(); ++it_path) { // iterate over the sentence path
			const Entity_Ref& entity_ref = *it_path;
			const Entity& path_entity = sent.entities[entity_ref];

			std::cout << "\"" << path_entity.index_ << "\" ";
		}
		std::cout << std::endl << std::endl;
		
		//
		// attribute marker path expansions
		//
		if (!sent.path_attributes.empty()) { // Path spans : attribute expansion
			std::cout << "Attribute Path Spans: " << std::endl;
			for (PathAttributeIterator it_path_attribute = sent.path_attributes.begin(); it_path_attribute != sent.path_attributes.end(); ++it_path_attribute) { // iterate over the attribute path expansion
				const Path_Attribute& attribute_expansion = *it_path_attribute;
				std::string a_type = AttributeName(attribute_expansion.type); // translate the attribute type
				std::cout << "Span_type:\"" << a_type << "\" span=";
				unsigned short head = attribute_expansion.pos; // starting position of attribute expansion
				Entity_Ref head_entity = sent.path[head]; // corresponding entity reference
				std::cout << "\"" << sent.entities[head_entity].index_ << "\""; // write head entity
				for (int i = 1; i < attribute_expansion.span; ++i) { // iterate attribute expansion span
					unsigned short trail = head + i; // next
					Entity_Ref trail_entity = sent.path[trail]; // next entity reference
					std::cout << " \"" << sent.entities[trail_entity].index_ << "\""; // write next entity
				}
			}
		}
		std::cout << std::endl;
	}
	//
	// proximity values
	//
	cout << "Text Source Proximity Overview :" << endl;
	// make a map of entity_id versus index string
	map<size_t, string> mapTextSource;
	for (SentenceIterator it_sent = engine.m_index.sentences.begin(); it_sent != engine.m_index.sentences.end(); ++it_sent) { // loop over the sentences
		for_each(it_sent->entities.begin(), it_sent->entities.end(), [&mapTextSource](const Entity& entity) { mapTextSource[entity.entity_id_] = entity.index_; }); // collect the entities
	}
	// plot the concept proximities
	// typedef std::pair<std::pair<EntityId, EntityId>, Proximity> ProximityPair_t; // single proximity pair
	for (Text_Source::Proximity::iterator itProx = engine.m_index.proximity.begin(); itProx != engine.m_index.proximity.end(); ++itProx) {
		size_t id1 = itProx->first.first;
		size_t id2 = itProx->first.second;
		double proximity = static_cast<double>(itProx->second);

		cout << "\"" << mapTextSource[id1] << "\":\"" << mapTextSource[id2] << "\"=" << proximity << endl;
	}

}

void testing_json_c_interface()
{
	const char* j_response;

	// static const std::set<std::string>& GetLanguagesSet(void);
	{
		json j_request;
		j_request["method"] = "GetLanguagesSet";
		cout << std::endl << std::endl << "request_json:" << std::endl;
		cout << j_request.dump() << std::endl;
		int ret = iknow_json(j_request.dump().c_str(), &j_response);
		cout << std::endl << "response_json:" << std::endl;
		if (ret == 0) {
			cout << j_response << std::endl;
		}
		if (ret == -1) {
			cout << "error in iknow_json..." << std::endl;
			cout << j_response << std::endl;
		}
	}
	// static std::string NormalizeText(const std::string & text_source, const std::string & language, bool bUserDct = false, bool bLowerCase = true, bool bStripPunct = true);
	{
		json j_request;
		j_request["method"] = "NormalizeText";
		j_request["text_source"] = u8"Risque d'exploitation";
		j_request["language"] = "fr";
		/* These are defaults...
		j_request["bUserDct"] = false;
		j_request["bLowerCase"] = true;
		j_request["bStripPunct"] = true;
		*/
		cout << std::endl << std::endl << "request_json:" << std::endl;
		cout << j_request.dump() << std::endl;
		int ret = iknow_json(j_request.dump().c_str(), &j_response);
		if (ret == 0) {
			cout << j_response << std::endl;
		}
		if (ret == -1) {
			cout << "error in iknow_json..." << std::endl;
			cout << j_response << std::endl;
		}
	}
	// static std::string IdentifyLanguage(const std::string& text_source, double& certainty);
	{
		json j_request;
		j_request["method"] = "IdentifyLanguage";
		j_request["text_source"] = u8"Микротерминатор может развивать скорость до 30 сантиметров за секунду, пишут калининградские СМИ.";
		cout << std::endl << std::endl << "request_json:" << std::endl;
		cout << j_request.dump() << std::endl;
		int ret = iknow_json(j_request.dump().c_str(), &j_response);
		if (ret == 0) {
			cout << j_response << std::endl;
		}
		if (ret == -1) {
			cout << "error in iknow_json..." << std::endl;
			cout << j_response << std::endl;
		}
	}
	// void index(iknow::base::String & text_source, const std::string & language, bool b_trace = false);
	{
		json j_request;
		j_request["method"] = "index";
		j_request["text_source"] = u8"This is a test of the Python interface to the iKnow engine. Be the change you want to see in life. Now, I have been on many walking holidays, but never on one where I have my bags ferried\nfrom hotel to hotel while I simply get on with the job of walkingand enjoying myself.";
		j_request["language"] = "en";
		// j_request["b_trace"] = true;
		cout << std::endl << std::endl << "request_json:" << std::endl;
		cout << j_request.dump() << std::endl;
		int ret = iknow_json(j_request.dump().c_str(), &j_response);
		if (ret == 0) {
			cout << j_response << std::endl;
		}
		if (ret == -1) {
			cout << "error in iknow_json..." << std::endl;
			cout << j_response << std::endl;
		}
	}
}
