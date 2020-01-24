#pragma once

#ifdef WIN32
#ifdef IKNOW_EXPORTS
#define IKNOW_API __declspec(dllexport)
#else
#define IKNOW_API __declspec(dllimport)
#endif
#else
#define IKNOW_API
#endif

#include <string>
#include <vector>
#include <map>

#include "unicode/utypes.h"
#include "unicode/unistr.h"

#include "IkTypes.h"
#include "IkLabel.h"
#include "IkIndexInput.h"
#include "IkIndexOutput.h"

// iknowdata collects the output of the text indexer.
namespace iknowdata {
	// after indexing, a document is a list of sentence data, the latter is a list of "struct Entity" objects.
	struct Entity
	{
		enum eType { NonRelevant = 0, Concept, Relation, PathRelevant }; // The 4 entity base types

		// the constructors are invoked by the indexer, to store linguistic data, for the client to "read" 
		Entity() : type_(NonRelevant) // The default one, for historical reasons
		{}
		Entity(eType type, size_t start, size_t stop, std::string& index) : type_(type), offset_start_(start), offset_stop_(stop), index_value_(index), entity_id_(NonRelevant)
		{}
		Entity(eType type, size_t start, size_t stop, std::string& index, double dominance) : type_(type), offset_start_(start), offset_stop_(stop), index_value_(index), dominance_value_(dominance), entity_id_(NonRelevant)
		{}
		Entity(eType type, size_t start, size_t stop, std::string& index, double dominance, size_t entity_id) : type_(type), offset_start_(start), offset_stop_(stop), index_value_(index), dominance_value_(dominance), entity_id_(entity_id)
		{}

		eType type_;	// type of entity, see eType definition
		size_t offset_start_;	// text offset of entity start position
		size_t offset_stop_;	// text offset of entity stop position
		// the index_value is the normalized form of the entity text
		std::string /*const*/ index_value_; // utf8 encoded, g++ refuses 'const' because of needed copy constructor in vector
		double dominance_value_;
		size_t entity_id_; // if not concept, this value equals -1
	};

	// the definition of a sentence : a vector of entities
	typedef std::vector<Entity> iKnow_Sentence;

	// utility functions to retrieve the starting and end offset of a sentence
	inline size_t sentence_offset_start(iKnow_Sentence& sentence) { return sentence.begin()->offset_start_; }
	inline size_t sentence_offset_stop(iKnow_Sentence& sentence) { return (sentence.end() - 1)->offset_stop_; }

	// the text source (document) after indexing : a vector of iKnow_Sentence objects
	typedef std::vector<iKnow_Sentence> iKnow_Sentences;
	// the definition of a path : a (reduced) vector of entities : non-relevants are filtered
	typedef std::vector<Entity> iKnow_Path;
	// the text document as a collection of iKnow_Path objects
	typedef std::vector<iKnow_Path> iKnow_Paths;
	// the text document as a collection of a per sentence CRC list
	typedef std::vector<std::vector<Entity> > iKnow_CRCs;
}

	struct Attribute
	{
		enum aType { Negation, PositiveSentiment, NegativeSentiment, Measurement, Other };

		Attribute(aType att_type, size_t start, size_t stop, std::string& level) : type_(att_type), offset_start_(start), offset_stop_(stop), level_(level), is_marker_(true), SCOPE(0)
		{}
		Attribute(aType att_type, size_t start, size_t stop) : type_(att_type), offset_start_(start), offset_stop_(stop), is_marker_(false), SCOPE(0)
		{}
		aType type_;
		size_t offset_start_;
		size_t offset_stop_;
		std::string level_; // the attribute lexreps
		bool is_marker_;
		size_t ID; // id number
		size_t SCOPE; // scope id, if any, otherwise 0
		std::string value_, unit_, value2_, unit2_; // properties for measurement attributes
	};

	
	typedef std::vector<Attribute> iKnow_Attributes; // document based, not per sentence
// }

class IKNOW_API iKnowEngine
{
public:
	// Utility function : retrieves the set of supported languages : ("en", "de", "ru", "es", "fr", "ja", "nl", "pt", "sv", "uk", "cs")
	static const std::set<std::string>& GetLanguagesSet(void);

	enum errcodes {
		iknow_language_not_supported = -1 // unsupported language
	};
	// Constructor and destructor
	iKnowEngine();
	~iKnowEngine();
	
	// The main indexing function :
	//	- text_source : the text input for indexing, must be Unicode 2byte (UCS2) encoded.
	//	- the language parameter, see supported languages
	// Current limitations : 
	//	- works synchronous : the complete text_source is indexed, after return, use iKnowEngine methods to retrieve indexing information.
	//	- works single threaded : a mutex protects multithread functioning. Use multiprocess to bypass this (current) limitation.
	void index(iknow::base::String& text_source, const std::string& language);

	// Adds User Dictionary annotations for customizing purposes
	void addUdctAnnotation(size_t start, size_t stop, const char* UdctLabel) {
		// std::cout << start << ":" << stop << ":" << UdctLabel << std::endl;
		m_map_udct_annotations.insert(std::make_pair(start, iknow::core::IkIndexInput::IknowAnnotation(start, stop, UdctLabel)));
	}
	iknowdata::iKnow_Sentences& sentences() { return m_sentences; }
	iknowdata::iKnow_Paths m_paths;
	iknowdata::iKnow_CRCs m_crcs;
	iKnow_Attributes m_attributes;
	iknow::core::IkConceptProximity::ProximityPairVector_t m_proximity;

private:
	iknowdata::iKnow_Sentences m_sentences;
	iknow::core::IkIndexInput::mapInputAnnotations_t m_map_udct_annotations;

};



