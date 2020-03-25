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

//
// stl library includes
//
#include <string>
#include <vector>
#include <map>

// 
// ICU dependency, engine is build with release 65-1, look for headers and binaries on github :
// https://github.com/unicode-org/icu/releases/tag/release-65-1
//
// Unzip the files in a directory, store that directory name into environment variable ICUDIR (eg: ICUDIR=C:\thirdparty\icu4c-65_1-Win64-MSVC2017)
// Dependencies file for Visual Studio 2019 solution (modules\Dependencies.props) uses this variable to set correct ICU include and library directories.
//
#include "unicode/utypes.h"
#include "unicode/unistr.h"

//
// includes from the iKnow sources : "..\engine\src, ..\base\src\headers, ..\shell\src, ..\core\src\headers"
//
#include "IkTypes.h"
#include "IkIndexInput.h"
#include "IkIndexOutput.h"

namespace iknowdata { // to bundle all generated data

	typedef unsigned short Entity_Ref; // reference to entity vector, max number of entities in a sentence is 1028, so unsigned short should be enough
	typedef unsigned short Attribute_Ref; // reference to sentence attribute vector, is less (or equal) Entity_Ref.

	//
	// Basic functionality of iknow indexing is splitup of text in entities : 4
	//
	struct Entity
	{
		static const size_t kNoConcept = static_cast<size_t>(-1);	// An concept entity receives a unique index in the text source, the others: kNoConcept
		enum eType { NonRelevant = 0, Concept, Relation, PathRelevant }; // The 4 base entity base types : concept, relation, path-relevant and non-relevant

		Entity(eType type, 
			size_t start, size_t stop, 
			std::string& index, 
			double dominance = 0.0, 
			size_t entity_id = kNoConcept
		) : type_(type), offset_start_(start), offset_stop_(stop), index_(index), dominance_value_(dominance), entity_id_(entity_id) {}

		eType type_; // defines the entity type
		size_t offset_start_, offset_stop_; // these refer to offsets in the text, "start" is where the textual representation starts, "stop" is where it stops.
		std::string index_; // the normalized entity textual representation, utf8 encoded
		double dominance_value_; // a dominance value for each concept in the source document is calculated, most important concepts have highest score.
		size_t entity_id_; // unique concept index in the source document, if not concept, this value equals kNoConcept
	};
	
	struct Sent_Attribute // sentence attribute
	{
		enum aType { Negation=1, DateTime=2, PositiveSentiment=5, NegativeSentiment=6, Frequency=8, Duration=9, Measurement=10, Certainty=11, Other }; // Supported attributes : negation marker, sentiment marker & measurement

		Sent_Attribute(aType att_type, 
			size_t start, size_t stop, 
			std::string& marker
		) : type_(att_type), offset_start_(start), offset_stop_(stop), marker_(marker) {}

		aType type_;
		size_t offset_start_, offset_stop_; // these refer to offsets in the text, "start" is where the textual representation starts, "stop" is where it stops.
		std::string marker_; // the normalized attribute textual representation, utf8 encoded
		std::string value_, unit_, value2_, unit2_; // optional properties for measurement attribute

		Entity_Ref entity_ref; // reference to entity vector, max number of entities in a sentence is 1028, so unsigned short should be enough
	};

	struct Path_Attribute_Span // path attribute span : expresses range of attribute
	{
		Attribute_Ref sent_attribute_ref; // reference to sentence attribute
		Entity_Ref entity_start_ref, entity_stop_ref; // reference to entity vector range, expressing the attribute expansion.
	};

	struct Sentence
	{
		typedef std::vector<Entity> Entities;
		typedef std::vector<Sent_Attribute> Sent_Attributes;
		typedef std::vector<Entity_Ref> Path;	// unsigned short indexes the Entity in the iKnow_Entities vector 
		typedef std::vector<Path_Attribute_Span> Path_Attributes;	// expanded 

		Entities			entities;	// the sentence entities
		Sent_Attributes		sent_attributes;	// the sentence attributes
		Path				path;		// the sentence path
		Path_Attributes		path_attributes;	// expanded attributes in the sentence

		// utility functions : return text source offsets of the sentence : start and stop.
		size_t offset_start() const { return entities.begin()->offset_start_; }
		size_t offset_stop() const { return (entities.end() - 1)->offset_stop_; }
	};

	struct Text_Source
	{
		typedef iknow::core::IkConceptProximity::ProximityPairVector_t Proximity;
		typedef std::vector<Sentence> Sentences;

		Sentences	sentences;	// All sentence data collected from the text source
		Proximity	proximity;	// Proximity data collected
	};

	typedef Text_Source::Sentences::const_iterator SentenceIterator;
	typedef Sentence::Entities::const_iterator	EntityIterator;
	typedef Sentence::Sent_Attributes::const_iterator AttributeMarkerIterator;
	typedef Sentence::Path::const_iterator PathIterator;
	typedef Sentence::Path_Attributes::const_iterator PathAttributeIterator;

}

class IKNOW_API iKnowEngine
{
public:
	static const std::set<std::string>& GetLanguagesSet(void);

	enum errcodes {
		iknow_language_not_supported = -1 // unsupported language
	};
	iKnowEngine();
	~iKnowEngine();
	
	// The main indexing function :
	//	- text_source : the text input for indexing, must be Unicode 2byte (UCS2) encoded.
	//	- the language parameter, see supported languages
	// Current limitations : 
	//	- works synchronous : the complete text_source is indexed, after return, use iKnowEngine methods to retrieve indexing information.
	//	- works single threaded : a mutex protects multithread functioning. Use multiprocess to bypass this (current) limitation.
	void index(iknow::base::String& text_source, const std::string& language);
	
	// Wrapper for indexing function that accepts UTF-8 encoded string instead. The offsets generated will be Unicode character
	// offsets, not byte offsets in text_source.
	void index(const std::string& text_source, const std::string& language);

	// Adds User Dictionary annotations for customizing purposes
	void addUdctAnnotation(size_t start, size_t stop, const char* UdctLabel) {
		m_map_udct_annotations.insert(std::make_pair(start, iknow::core::IkIndexInput::IknowAnnotation(start, stop, UdctLabel)));
	}

	iknowdata::Text_Source m_index; // this is where all iKnow indexed information is stored after calling the "index" method.

private:
	iknow::core::IkIndexInput::mapInputAnnotations_t m_map_udct_annotations;

};



