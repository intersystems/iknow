#include "engine.h"

// #include <iostream>
#include <numeric>
#include <mutex>

#include "Process.h"
#include "SharedMemoryKnowledgebase.h"
#include "CompiledKnowledgebase.h"
#include "IkSummarizer.h"
#include "IkPath.h"
#include "IkIndexInput.h"
#include "RegExServices.h"

const std::set<std::string>& iKnowEngine::GetLanguagesSet(void) {
	static const std::set<std::string> iknow_languages = { "en", "de", "ru", "es", "fr", "ja", "nl", "pt", "sv", "uk", "cs" };
	return iknow_languages;
}

inline void outMessage(const char* msg)
{
#if defined(VERBOSE_MODE)
#if defined(WIN32)
	std::cout << msg << "(" << GetCurrentProcessId() << "," << GetCurrentThreadId() << ")" << std::endl;
#endif
#endif
}

typedef void(*OutputFunc)(iknow::core::IkIndexOutput*, iknow::core::IkIndexDebug*, void*, Stemmer*);

using iknow::shell::CProcess;
using iknow::shell::SharedMemoryKnowledgebase;
using iknow::shell::CompiledKnowledgebase;
using iknow::base::String;
using iknow::base::SpaceString;
using iknow::base::Char;
using iknow::base::IkStringEncoding;
using iknow::core::IkSummarizer;
using iknow::core::IkKnowledgebase;
using iknow::core::IkIndexProcess;
using iknow::core::Sentences;
using iknow::core::IkSentence;
using iknow::core::IkMergedLexrep;
using iknow::core::MergedLexreps;
using iknow::core::IkLexrep;
using iknow::core::IkPath;
using iknow::core::FastLabelSet;
using iknow::base::PoolAllocator;
using iknow::core::path::Offsets;
using iknow::core::path::CRC;
using iknow::core::path::CRCs;

using iknowdata::iKnow_Sentences;
using iknowdata::iKnow_Paths;
using iknowdata::iKnow_CRCs;

struct UData
{
	UData(iKnow_Sentences& sents, iKnow_Paths& paths, iKnow_CRCs& crcs, iKnow_Attributes& attributes, iknow::core::IkConceptProximity::ProximityPairVector_t& proximity) :
		u_sentences(sents), 
		u_paths(paths), 
		u_crcs(crcs), 
		u_attributes(attributes),
		u_proximity(proximity) 
	{}

	iKnow_Sentences& u_sentences; // reference to sentence information
	iKnow_Paths& u_paths; // reference to path information
	iKnow_CRCs& u_crcs; // reference to crc information
	iKnow_Attributes& u_attributes; // reference to attribute information
	iknow::core::IkConceptProximity::ProximityPairVector_t& u_proximity; // collection of proximity information
};

typedef unsigned short PropertyId;

static iknow::core::RegExServices RegExHandler; // static is fine, API blocks multiple threads...

static const String kEntityString = IkStringEncoding::UTF8ToBase("Entity");
static const String kNegationString = IkStringEncoding::UTF8ToBase("Negation");
static const String kPositiveSentimentString = IkStringEncoding::UTF8ToBase("PositiveSentiment");
static const String kNegativeSentimentString = IkStringEncoding::UTF8ToBase("NegativeSentiment");
static const String kMeasurementString = IkStringEncoding::UTF8ToBase("Measurement"); // Entity(Measurement, Value, Unit), "Value", "Unit", "Value"+"Unit"
static const String kMeasurementValueString = IkStringEncoding::UTF8ToBase("Value");
static const String kMeasurementUnitString = IkStringEncoding::UTF8ToBase("Unit");


// 
// helper function translates internal iknow::core::IkLabel::Type to iknowdata::Entity::eType
// 
static inline iknowdata::Entity::eType get_ent_type(const IkMergedLexrep *lexrep) {
	switch (lexrep->GetLexrepType()) {
	case iknow::core::IkLabel::Relation: return iknowdata::Entity::Relation;
	case iknow::core::IkLabel::Concept: return iknowdata::Entity::Concept;
	case iknow::core::IkLabel::PathRelevant: return iknowdata::Entity::PathRelevant;
	default: return iknowdata::Entity::NonRelevant;
	}
}

static void iKnowEngineOutputCallback(iknow::core::IkIndexOutput* data, iknow::core::IkIndexDebug* debug, void* userdata, Stemmer* stemmer)
{
	UData udata = *((UData*)userdata);

	bool bIsIdeographic = data->IsJP();
	const String lexrep_separator = (bIsIdeographic ? String() : SpaceString()); // no separator char for Japanese
	const iknow::base::Char *pText = data->GetTextPointer(); // Get original text pointer

	typedef std::map<const IkMergedLexrep*, size_t> mapLexrep2Attribute_type;
	mapLexrep2Attribute_type mapLexrep2Attribute; // map lexreps to attribute markers.

	Sentences::iterator sentences_begin = data->SentencesBegin();
	Sentences::iterator sentences_end = data->SentencesEnd();

	size_t sentences_size = sentences_end - sentences_begin;
	for (Sentences::iterator i = sentences_begin; i != sentences_end; ++i) {
		IkSentence* sentence = &(*i);
		const IkKnowledgebase* kb = sentence->GetLexrepsBegin()->LexrepsBegin()->GetKnowledgebase(); // KB does not change in a sentence.
		RegExHandler.swich_kb(kb); // Switch to the correct KB
		const iknow::core::AttributeId entity_attribute_type = kb->AttributeIdForName(kEntityString);
		const iknow::core::AttributeId negation_attribute_type = kb->AttributeIdForName(kNegationString);
		const iknow::core::AttributeId positive_sentiment_attribute_type = kb->AttributeIdForName(kPositiveSentimentString);
		const iknow::core::AttributeId negative_sentiment_attribute_type = kb->AttributeIdForName(kNegativeSentimentString);
		const iknow::core::AttributeId measurement_attribute_type = kb->AttributeIdForName(kMeasurementString);
		const iknow::core::AttributeId m_value_attribute_type = kb->AttributeIdForName(kMeasurementValueString);
		const iknow::core::AttributeId m_unit_attribute_type = kb->AttributeIdForName(kMeasurementUnitString);

		iknowdata::iKnow_Sentence single_sentence;
		for (MergedLexreps::const_iterator j = sentence->GetLexrepsBegin(); j != sentence->GetLexrepsEnd(); ++j) { // iterate entities
			const IkMergedLexrep *lexrep = &(*j);
			const Char* literal_start = lexrep->GetTextPointerBegin();
			const Char* literal_end = lexrep->GetTextPointerEnd();
			const size_t text_start = literal_start - pText;
			const size_t text_stop = literal_end - pText;
			// std::cout << iknow::base::IkStringEncoding::BaseToUTF8(lexrep->GetNormalizedText()) << std::endl;

			{  // Scan for label attributes
				bool at_measure = false;
				String at_token, at_value, at_unit, at_value2, at_unit2; // measurement attribute properties

				for (IkMergedLexrep::const_iterator it = lexrep->LexrepsBegin(); it != lexrep->LexrepsEnd(); it++) { // scroll the single lexreps
					bool is_measure = false, is_value = false, is_unit = false; // lexrep level
					const size_t label_count = it->NumberOfLabels();
					for (size_t i = 0; i < label_count; ++i) {
						FastLabelSet::Index idx_label = it->GetLabelIndexAt(i);
						size_t attribute_count = kb->GetAttributeCount(idx_label); // how many attributes on this label ?
						for (size_t cnt_attribute = 0; cnt_attribute < attribute_count; cnt_attribute++) { // iterate the attributes
							iknow::core::AttributeId type_attribute = kb->GetAttributeType(idx_label, cnt_attribute);
							if (type_attribute == entity_attribute_type) { // we found an entity attribute
								for (const iknow::core::AttributeId* ent_param = kb->GetAttributeParamsBegin(idx_label, cnt_attribute); ent_param != kb->GetAttributeParamsEnd(idx_label, cnt_attribute); ++ent_param) {
									if (*ent_param == measurement_attribute_type) { // new, measurement attribute type
										at_measure = is_measure = true;
									}
									if (*ent_param == m_value_attribute_type) { // new, measurement value attribute type
										is_value = true;
									}
									if (*ent_param == m_unit_attribute_type) { // new, measurement unit attribute type
										is_unit = true;
									}
									Attribute::aType a_type = (*ent_param == positive_sentiment_attribute_type ? Attribute::PositiveSentiment : (*ent_param == negative_sentiment_attribute_type ? Attribute::NegativeSentiment : (*ent_param == negation_attribute_type ? Attribute::Negation : Attribute::Other)));
									if (a_type != Attribute::Other) { // emit token for sentiment & negation
										std::string a_index = iknow::base::IkStringEncoding::BaseToUTF8(it->GetNormalizedText());
										// store marker : a_index
										udata.u_attributes.push_back(Attribute(a_type, it->GetTextPointerBegin() - pText, it->GetTextPointerEnd() - pText, a_index));
										udata.u_attributes.back().ID = udata.u_attributes.size(); // set ID to offset in vector
										mapLexrep2Attribute.insert(make_pair(lexrep, udata.u_attributes.size())); // link ID to lexrep
									}
								}
							}
						}
					}
					// std::cout << (is_measure ? "m" : "") << (is_value ? "v" : "") << (is_unit ? "u" : "") << std::endl;
					if (is_measure) { // this is the token
						at_token = it->GetValue();
						if (is_value && is_unit) { // separate value & unit properties
							if (!RegExHandler.SplitValueUnit(it->GetNormalizedText(), at_value, at_unit)) {  // failed to separate
								int parts = RegExHandler.Parser2(at_token, at_value, at_unit, at_value2, at_unit2); // refined value/unit parser
								if (parts == 0) at_value = it->GetNormalizedText(); // set as Value
							}
						}
						else {
							if (is_value) at_value = it->GetNormalizedText();
							if (is_unit) at_unit = it->GetNormalizedText();
						}
					}
				}
				// <attr type = "measurement" literal = "5%-82%;" token = "5%-82%;" value = "5" unit = "%" value2 = "82" unit2 = "%">
				if (at_measure) { // Measurement attribute
					std::string measurement_marker = iknow::base::IkStringEncoding::BaseToUTF8(at_value);
					udata.u_attributes.push_back(Attribute(Attribute::Measurement, text_start, text_stop, measurement_marker));
					if (at_value.length()) udata.u_attributes.back().value_ = iknow::base::IkStringEncoding::BaseToUTF8(at_value);
					if (at_unit.length()) udata.u_attributes.back().unit_ = iknow::base::IkStringEncoding::BaseToUTF8(at_unit);
					if (at_value2.length()) udata.u_attributes.back().value2_ = iknow::base::IkStringEncoding::BaseToUTF8(at_value2);
					if (at_unit2.length()) udata.u_attributes.back().unit2_ = iknow::base::IkStringEncoding::BaseToUTF8(at_unit2);

					udata.u_attributes.back().ID = static_cast<int>(udata.u_attributes.size()); // set ID to offset in vector
					mapLexrep2Attribute.insert(make_pair(lexrep, udata.u_attributes.size())); // link ID to lexrep
				}
			}
			// UnicodeString index_value(lexrep->GetNormalizedText().c_str());
			std::string index_value = IkStringEncoding::BaseToUTF8(lexrep->GetNormalizedText());
			iknow::core::IkIndexOutput::EntityId ent_id = data->GetEntityID(lexrep);

			iknowdata::Entity::eType e_type = get_ent_type(lexrep);
			single_sentence.push_back(iknowdata::Entity(e_type, text_start, text_stop, index_value, data->GetEntityDominance(lexrep), ent_id));
		}
		udata.u_sentences.push_back(single_sentence);

		for (IkSentence::Paths::const_iterator j = sentence->GetPathsBegin(); j != sentence->GetPathsEnd(); ++j) { // iterate paths
			const IkPath* path = &(*j);
			iknowdata::iKnow_Path single_path;
			for (Offsets::const_iterator k = path->OffsetsBegin(); k != path->OffsetsEnd(); ++k) {
				IkMergedLexrep* lexrep = iknow::core::path::CRC::OffsetToLexrep(*k, sentence->GetLexrepsBegin()); // Sentence offset to lexrep.
				// UnicodeString index_value(lexrep->GetNormalizedText().c_str());
				std::string index_value = IkStringEncoding::BaseToUTF8(lexrep->GetNormalizedText());
				iknowdata::Entity::eType e_type = get_ent_type(lexrep);
				iknowdata::Entity entity(e_type, lexrep->GetTextPointerBegin() - pText, lexrep->GetTextPointerEnd() - pText, index_value);
				single_path.push_back(entity);
			}
			udata.u_paths.push_back(single_path);
		}
	}

	data->GetProximityPairVector(udata.u_proximity); // Proximity is document related

	// treat attribute paths
	for (iknow::core::IkIndexOutput::vecAttributePaths::iterator itAPaths = data->AttributePathsBegin(); itAPaths != data->AttributePathsEnd(); ++itAPaths) {
		PropertyId id = itAPaths->first;
		std::vector<const IkMergedLexrep*>& lexreps_vec = itAPaths->second;
		const IkMergedLexrep* start = *(lexreps_vec.begin());
		const IkMergedLexrep* stop = *(lexreps_vec.end() - 1);
		const IkKnowledgebase* kb = start->LexrepsBegin()->GetKnowledgebase();
		const iknow::core::AttributeId negation_attribute_type = kb->AttributeIdForName(kNegationString);
		const iknow::core::AttributeId positive_sentiment_attribute_type = kb->AttributeIdForName(kPositiveSentimentString);
		const iknow::core::AttributeId negative_sentiment_attribute_type = kb->AttributeIdForName(kNegativeSentimentString);

		Attribute::aType a_type = (id == positive_sentiment_attribute_type ? Attribute::PositiveSentiment : (id == negative_sentiment_attribute_type ? Attribute::NegativeSentiment : (id == negation_attribute_type ? Attribute::Negation : Attribute::Other)));
		if (a_type == Attribute::Other) // only sentiment and negation paths are currently "annotated"
			continue;

		// store attribute entity
		udata.u_attributes.push_back(Attribute(a_type, start->GetTextPointerBegin() - pText, stop->GetTextPointerEnd() - pText));
		size_t id_attribute_path = udata.u_attributes.back().ID = udata.u_attributes.size(); // set ID to offset in vector

		// Where is the marker ?
		for (std::vector<const IkMergedLexrep*>::iterator itLexreps = lexreps_vec.begin(); itLexreps != lexreps_vec.end(); ++itLexreps) {
			mapLexrep2Attribute_type::iterator itLexrep = mapLexrep2Attribute.find(*itLexreps);
			if (itLexrep != mapLexrep2Attribute.end()) udata.u_attributes[itLexrep->second - 1].SCOPE = id_attribute_path; // marker scope
		}
	}

}

iKnowEngine::iKnowEngine()
{
	outMessage("iKnowEngine: Constructor");
}

iKnowEngine::~iKnowEngine()
{
	outMessage("iKnowEngine: Destructor");
}

typedef std::map<iknow::base::String, iknow::core::IkKnowledgebase*> KnowledgebaseMap;

#include "../language_data/kb_en_data.h"
#include "../language_data/kb_de_data.h"
#include "../language_data/kb_ru_data.h"
#include "../language_data/kb_es_data.h"
#include "../language_data/kb_fr_data.h"
#include "../language_data/kb_ja_data.h"
#include "../language_data/kb_nl_data.h"
#include "../language_data/kb_pt_data.h"
#include "../language_data/kb_sv_data.h"
#include "../language_data/kb_uk_data.h"
#include "../language_data/kb_cs_data.h"

struct LanguageCodeMap {
	LanguageCodeMap() {
		map.insert(CodeMap::value_type("en", const_cast<unsigned char *>(&(kb_en_data[0]))));
		map.insert(CodeMap::value_type("de", const_cast<unsigned char *>(&(kb_de_data[0]))));
		map.insert(CodeMap::value_type("ru", const_cast<unsigned char *>(&(kb_ru_data[0]))));
		map.insert(CodeMap::value_type("es", const_cast<unsigned char *>(&(kb_es_data[0]))));
		map.insert(CodeMap::value_type("fr", const_cast<unsigned char *>(&(kb_fr_data[0]))));
		map.insert(CodeMap::value_type("ja", const_cast<unsigned char *>(&(kb_ja_data[0]))));
		map.insert(CodeMap::value_type("nl", const_cast<unsigned char *>(&(kb_nl_data[0]))));
		map.insert(CodeMap::value_type("pt", const_cast<unsigned char *>(&(kb_pt_data[0]))));
		map.insert(CodeMap::value_type("sv", const_cast<unsigned char *>(&(kb_sv_data[0]))));
		map.insert(CodeMap::value_type("uk", const_cast<unsigned char *>(&(kb_uk_data[0]))));
		map.insert(CodeMap::value_type("cs", const_cast<unsigned char *>(&(kb_cs_data[0]))));
	}
	unsigned char *Lookup(const std::string& language_name) const {
		CodeMap::const_iterator i = map.find(language_name);
		return ((i == map.end()) ? NULL : i->second);
	}

	typedef std::map<std::string, unsigned char *> CodeMap;
	CodeMap map;
};

const static LanguageCodeMap language_code_map;
static std::mutex mtx;           // mutex for process.IndexFunc critical section

void iKnowEngine::index(iknow::base::String& text_input, const std::string& utf8language)
{
	std::unique_lock<std::mutex> lck(mtx, std::defer_lock);

	m_sentences.clear();
	m_paths.clear();
	m_crcs.clear();
	m_attributes.clear();
	m_proximity.clear();
	UData udata(m_sentences, m_paths, m_crcs, m_attributes, m_proximity);
	try {
		SharedMemoryKnowledgebase skb = language_code_map.Lookup(utf8language);
		CompiledKnowledgebase ckb(&skb, utf8language);
		CProcess::type_languageKbMap temp_map;
		temp_map.insert(CProcess::type_languageKbMap::value_type(IkStringEncoding::UTF8ToBase(utf8language), &ckb));
		CProcess process(temp_map);
		outMessage("Calling the iKnow Indexer");

		iknow::core::IkIndexInput Input(&text_input);
		lck.lock(); // critical section (exclusive access to IndexFunc by locking lck):
		process.IndexFunc(Input, iKnowEngineOutputCallback, &udata, true);
		lck.unlock();

		outMessage("Succesfull return of the iKnow Indexer");
	}
	catch (...) {
		outMessage("Exception called from iKnow Engine");
	}
}

