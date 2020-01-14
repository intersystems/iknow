/*
** enginetest.cpp : testing the iKnow engine
*/
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "engine.h"

using iknow::base::String;

String getSampleText(std::string language_code) { // must be utf-8 encoded text.
	if (language_code == "en") return String(u"Be the change you want to see in life.");
	if (language_code == "de") return String(u"Oder die Erkundung der Natur - und zwar ohne Anleitung.");
	if (language_code == "ru") return String(u"Микротерминатор может развивать скорость до 30 сантиметров за секунду, пишут калининградские СМИ.");
	if (language_code == "es") return String(u"En Argentina no hay estudios previos reportados en cuanto a la elaboración de vinos cítricos ni de «vino de naranja».");
	if (language_code == "fr") return String(u"En pratique comment le faire ?");
	if (language_code == "ja") return String(u"こんな台本でプロットされては困る、と先生は言った。");
	if (language_code == "nl") return String(u"Op basis van de afzonderlijke evaluatieverslagen stelt de Commissie een synthese op communautair niveau op.");
	if (language_code == "pt") return String(u"Distingue-se o mercado de um produto ou serviço dos mercados de fatores de produção, capital e trabalho.");
	if (language_code == "sv") return String(u"Jag är bäst i klassen. Ingen gör efter mig, kan jag inte lämna. Var försiktig, är gräset alltid grönare på andra sidan.");
	if (language_code == "uk") return String(u"грошових зобов'язань, прийнятих на себе згідно з умов цього договору.");
	if (language_code == "cs") return String(u"Létající jaguár je novela spisovatele Josefa Formánka z roku 2004.");

	return String(u"Time flies like an arrow, fruit flies like a banana");
}

int main(int argc, char* argv[])
{
	const char *default_lang = "en";
	const std::set<std::string> languages_set = iKnowEngine::GetLanguagesSet();

	try {
		std::cout << "iKnowEngine driver" << std::endl << std::endl;
		std::ofstream os("iknowdata.log");
		os << "\xEF\xBB\xBF"; // Force utf8 header, maybe utf8 is not the system codepage

		iKnowEngine myEngine;
		for (std::set<std::string>::iterator it_lang = languages_set.begin(); it_lang != languages_set.end(); ++it_lang) {
			if (languages_set.find(*it_lang) == languages_set.end()) return iKnowEngine::iknow_language_not_supported;
			String text_source = getSampleText(*it_lang);
			std::cout << "Indexing " << *it_lang << std::endl;
			myEngine.index(text_source, *it_lang);

			using namespace iknowdata;
			for (iKnow_Sentences::iterator it = myEngine.m_sentences.begin(); it != myEngine.m_sentences.end(); ++it)
			{
				iKnow_Sentence& sent = *it; 

				// start and stop offset of the sentence
				std::cout << "S:" << sentence_offset_start(sent) << ":" << sentence_offset_stop(sent) << ";" << std::endl;

				// sentence entity sequence, per 4 base types : concept, relation, path-relevant, non-relevant.
				for (iKnow_Sentence::iterator it_entity = sent.begin(); it_entity != sent.end(); ++it_entity) {
					char t = it_entity->type_ == Entity::Concept ? 'c' : (it_entity->type_ == Entity::Relation ? 'r' : (it_entity->type_ == Entity::PathRelevant ? 'p' : 'n'));
					std::cout << t;
					os << "<" << t << ">" << it_entity->index_value_ << "</" << t << ">";
				}
				std::cout << std::endl;
				os << std::endl;
			}
		}
		os.close();
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
