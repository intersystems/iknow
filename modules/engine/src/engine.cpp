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

typedef void(*OutputFunc)(iknow::core::IkIndexOutput*, iknow::core::IkIndexDebug<TraceListType>*, void*, Stemmer*);

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

using iknowdata::Text_Source;
using iknowdata::Sent_Attribute;
using iknowdata::Entity;
using iknowdata::Path_Attribute_Span;

struct UData
{
	UData(Text_Source::Sentences& sents, iknow::core::IkConceptProximity::ProximityPairVector_t& proximity) :
		iknow_sentences(sents),
		iknow_proximity(proximity)
	{}

	Text_Source::Sentences& iknow_sentences; // reference to sentence information
	iknow::core::IkConceptProximity::ProximityPairVector_t& iknow_proximity; // collection of proximity information
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

static void iKnowEngineOutputCallback(iknow::core::IkIndexOutput* data, iknow::core::IkIndexDebug<TraceListType>* debug, void* userdata, Stemmer* stemmer)
{
	UData udata = *((UData*)userdata);

	bool bIsIdeographic = data->IsJP();
	const String lexrep_separator = (bIsIdeographic ? String() : SpaceString()); // no separator char for Japanese
	const iknow::base::Char *pText = data->GetTextPointer(); // Get original text pointer

	typedef std::map<const IkMergedLexrep*, std::pair<unsigned short, unsigned short> > mapLexrep2Attribute_type;
	mapLexrep2Attribute_type mapLexrep2Attribute; // map lexreps to attribute markers.
	typedef std::map<const IkMergedLexrep*, std::pair<unsigned short, unsigned short> > mapLexrep2Entity_type;
	mapLexrep2Entity_type mapLexrep2Entity; // map lexreps to entities.

	for (Sentences::iterator i = data->SentencesBegin(); i != data->SentencesEnd(); ++i) {
		IkSentence* sentence = &(*i);
		const IkKnowledgebase* kb = sentence->GetLexrepsBegin()->LexrepsBegin()->GetKnowledgebase(); // KB does not change in a sentence.
		RegExHandler.swich_kb(kb); // Switch to the correct KB

		iknowdata::Sentence sentence_data;
		for (MergedLexreps::const_iterator j = sentence->GetLexrepsBegin(); j != sentence->GetLexrepsEnd(); ++j) { // iterate entities
			const IkMergedLexrep *lexrep = &(*j);

			// map lexrep to sentence and entity
			unsigned short idx_sentence = static_cast<unsigned short>(udata.iknow_sentences.size()); // sentence reference
			unsigned short idx_entity = static_cast<unsigned short>(sentence_data.entities.size()); // entity reference
			mapLexrep2Entity.insert(make_pair(lexrep, make_pair(idx_sentence, idx_entity))); // link ID to lexrep

			const Char* literal_start = lexrep->GetTextPointerBegin();
			const Char* literal_end = lexrep->GetTextPointerEnd();
			const size_t text_start = literal_start - pText;
			const size_t text_stop = literal_end - pText;

			for (IkMergedLexrep::const_iterator it = lexrep->LexrepsBegin(); it != lexrep->LexrepsEnd(); it++) { // Scan for label attributes : scroll the single lexreps
				bool is_measure = false, is_value = false, is_unit = false; // lexrep level
				const size_t label_count = it->NumberOfLabels();
				for (size_t i = 0; i < label_count; ++i) {
					FastLabelSet::Index idx_label = it->GetLabelIndexAt(i);
					size_t attribute_count = kb->GetAttributeCount(idx_label); // how many attributes on this label ?
					for (size_t cnt_attribute = 0; cnt_attribute < attribute_count; cnt_attribute++) { // iterate the attributes
						iknow::core::AttributeId type_attribute = kb->GetAttributeType(idx_label, cnt_attribute);
						String name_attribute(kb->AttributeNameForId(type_attribute).data, kb->AttributeNameForId(type_attribute).size);

						if (name_attribute == kEntityString) { // found an entity attribute, read the parameters							
							iknow::core::PropertyId id_property = 0; // first parameter is property
							bool is_value = is_unit = false; // extra parameters for measurement
							for (const iknow::core::AttributeId* ent_param = kb->GetAttributeParamsBegin(idx_label, cnt_attribute); ent_param != kb->GetAttributeParamsEnd(idx_label, cnt_attribute); ++ent_param) {
								String param_attribute(kb->AttributeNameForId(*ent_param).data, kb->AttributeNameForId(*ent_param).size);

								if (param_attribute == kMeasurementString) id_property = Sent_Attribute::Measurement;
								if (!id_property) id_property = kb->PropertyIdForName(param_attribute);
								if (param_attribute == kMeasurementValueString) { // new, measurement value attribute type
									is_value = true;
								}
								if (param_attribute == kMeasurementUnitString) { // new, measurement unit attribute type
									is_unit = true;
								}
							}
							Sent_Attribute::aType a_type = static_cast<Sent_Attribute::aType>(id_property);
							std::string a_index = iknow::base::IkStringEncoding::BaseToUTF8(it->GetNormalizedText());
							unsigned short idx_sentence = static_cast<unsigned short>(udata.iknow_sentences.size()); // sentence reference
							unsigned short idx_attribute = static_cast<unsigned short>(sentence_data.sent_attributes.size()); // attribute reference
							mapLexrep2Attribute.insert(make_pair(lexrep, make_pair(idx_sentence, idx_attribute))); // link ID to lexrep

							sentence_data.sent_attributes.push_back(Sent_Attribute(a_type, it->GetTextPointerBegin() - pText, it->GetTextPointerEnd() - pText, a_index));
							sentence_data.sent_attributes.back().entity_ref = static_cast<unsigned short>(sentence_data.entities.size()); // connect sentence attribute to entity

							if (id_property == Sent_Attribute::Measurement) { // <attr type = "measurement" literal = "5%-82%;" token = "5%-82%;" value = "5" unit = "%" value2 = "82" unit2 = "%">
								String at_value, at_unit, at_value2, at_unit2; // measurement attribute properties
								String at_token = it->GetValue();
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
								Sent_Attribute& ref = sentence_data.sent_attributes.back(); // reference the newly added measuremnt attribute

								// ref.optional_parameters.measurement.value_ = 
								if (at_value.length()) ref.value_ = iknow::base::IkStringEncoding::BaseToUTF8(at_value);
								if (at_unit.length()) ref.unit_ = iknow::base::IkStringEncoding::BaseToUTF8(at_unit);
								if (at_value2.length()) ref.value2_ = iknow::base::IkStringEncoding::BaseToUTF8(at_value2);
								if (at_unit2.length()) ref.unit2_ = iknow::base::IkStringEncoding::BaseToUTF8(at_unit2);
							}								
						}
					}
				}				
			}
			std::string index_value = IkStringEncoding::BaseToUTF8(lexrep->GetNormalizedText());
			iknow::core::IkIndexOutput::EntityId ent_id = data->GetEntityID(lexrep);
			iknowdata::Entity::eType e_type = get_ent_type(lexrep);
			sentence_data.entities.push_back(Entity(e_type, text_start, text_stop, index_value, data->GetEntityDominance(lexrep), ent_id));
		}
		// collect sentence path data
		for (IkSentence::Paths::const_iterator j = sentence->GetPathsBegin(); j != sentence->GetPathsEnd(); ++j) { // iterate paths
			const IkPath* path = &(*j);
			for (Offsets::const_iterator k = path->OffsetsBegin(); k != path->OffsetsEnd(); ++k) {
				IkMergedLexrep* lexrep = iknow::core::path::CRC::OffsetToLexrep(*k, sentence->GetLexrepsBegin()); // Sentence offset to lexrep.
				unsigned short entity_id = mapLexrep2Entity[lexrep].second; // entity id from lexrep
				sentence_data.path.push_back(entity_id); // reference to sentence entities
			}
		}
		udata.iknow_sentences.push_back(sentence_data); // Collect single sentence data
	}
	data->GetProximityPairVector(udata.iknow_proximity); // Proximity is document related

	// treat attribute paths
	for (iknow::core::IkIndexOutput::vecAttributePaths::iterator itAPaths = data->AttributePathsBegin(); itAPaths != data->AttributePathsEnd(); ++itAPaths) {
		Path_Attribute_Span path_attribute_span;

		Sent_Attribute::aType a_type = static_cast<Sent_Attribute::aType>(itAPaths->first);

		std::vector<const IkMergedLexrep*>& lexreps_vec = itAPaths->second; // attribute path expansion
		const IkMergedLexrep* start = *(lexreps_vec.begin());
		const IkMergedLexrep* stop = *(lexreps_vec.end() - 1);
		unsigned short idx_sentence = mapLexrep2Entity[start].first;
		path_attribute_span.entity_start_ref = mapLexrep2Entity[start].second;
		path_attribute_span.entity_stop_ref = mapLexrep2Entity[stop].second;

		// Where is the marker ?
		for (std::vector<const IkMergedLexrep*>::iterator itLexreps = lexreps_vec.begin(); itLexreps != lexreps_vec.end(); ++itLexreps) {
			mapLexrep2Attribute_type::iterator itLexrep = mapLexrep2Attribute.find(*itLexreps);
			if (itLexrep != mapLexrep2Attribute.end()) {
				unsigned short idx_sentence = (itLexrep->second).first;
				path_attribute_span.sent_attribute_ref = (itLexrep->second).second;

				udata.iknow_sentences[idx_sentence].path_attributes.push_back(path_attribute_span); // store the expanded path attribute.
				break;
			}
		}
	}
}

iKnowEngine::iKnowEngine() // Constructor
{
}

iKnowEngine::~iKnowEngine() // Destructor
{
}

typedef std::map<iknow::base::String, iknow::core::IkKnowledgebase*> KnowledgebaseMap;

extern "C" {
	extern const unsigned char kb_en_data[];
	extern const unsigned char kb_de_data[];
	extern const unsigned char kb_ru_data[];
	extern const unsigned char kb_es_data[];
	extern const unsigned char kb_fr_data[];
	extern const unsigned char kb_ja_data[];
	extern const unsigned char kb_nl_data[];
	extern const unsigned char kb_pt_data[];
	extern const unsigned char kb_sv_data[];
	extern const unsigned char kb_uk_data[];
	extern const unsigned char kb_cs_data[];
}

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

	m_index.sentences.clear();
	m_index.proximity.clear();
	UData udata(m_index.sentences, m_index.proximity);

	SharedMemoryKnowledgebase skb = language_code_map.Lookup(utf8language);
	CompiledKnowledgebase ckb(&skb, utf8language);
	CProcess::type_languageKbMap temp_map;
	temp_map.insert(CProcess::type_languageKbMap::value_type(IkStringEncoding::UTF8ToBase(utf8language), &ckb));
	CProcess process(temp_map);
	iknow::core::IkIndexInput Input(&text_input);
#ifdef _DEBUG // debug version generates a linguistic trace file.
	bool b_generate_trace_file = true;
#else
	bool b_generate_trace_file = false;
#endif
	lck.lock(); // critical section (exclusive access to IndexFunc by locking lck):
	process.IndexFunc(Input, iKnowEngineOutputCallback, &udata, true, b_generate_trace_file);
	lck.unlock();
}

void iKnowEngine::index(const std::string& text_source, const std::string& language) {
	String text_source_ucs2(IkStringEncoding::UTF8ToBase(text_source));
	index(text_source_ucs2, language);
}
