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

//
// Attribute Values from IRIS Public interface
//
#define IKATTNEGATION		1
#define IKATTTIME			2
#define IKATTMODIFIER		3
#define IKATTNONSEMANTIC	4
#define IKATTSENPOSITIVE	5
#define IKATTSENNEGATIVE	6
#define IKATTENTITYVECTOR	7
#define IKATTTOPIC			8
#define IKATTFREQ			9 
#define IKATTDURATION 		10
#define IKATTMEASURE		11
#define IKATTCERTAINTY		12

namespace iknowdata { // to bundle all generated data

	typedef unsigned short Entity_Ref; // reference to entity vector, max number of entities in a sentence is 1028, so unsigned short should be enough
	typedef unsigned short Attribute_Ref; // reference to sentence attribute vector, is less (or equal) Entity_Ref.

	enum /*class*/ Attribute { // Supported attributes in NLP-Fx, should be an "enum class", but Cython refuses to build
		Negation = IKATTNEGATION,
		DateTime = IKATTTIME,
		PositiveSentiment = IKATTSENPOSITIVE,
		NegativeSentiment = IKATTSENNEGATIVE,
		Frequency = IKATTFREQ,
		Duration = IKATTDURATION,
		Measurement = IKATTMEASURE,
		Certainty = IKATTCERTAINTY
	};
	inline std::string AttributeName(Attribute attribute) { // translate the attribute type
		switch (attribute) {
		case Attribute::Negation:			return "negation";
		case Attribute::DateTime:			return "date_time";
		case Attribute::PositiveSentiment:	return "positive_sentiment";
		case Attribute::NegativeSentiment:	return "negative_sentiment";
		case Attribute::Frequency:			return "frequency";
		case Attribute::Duration:			return "duration";
		case Attribute::Measurement:		return "measurement";
		case Attribute::Certainty:			return "certainty";
		default:							return "unknown";
		}
	}
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
		Sent_Attribute(Attribute att_type,
			size_t start, size_t stop, 
			std::string& marker
		) : type_(att_type), offset_start_(start), offset_stop_(stop), marker_(marker) {}

		Attribute type_;
		size_t offset_start_, offset_stop_; // these refer to offsets in the text, "start" is where the textual representation starts, "stop" is where it stops.
		std::string marker_; // the normalized attribute textual representation, utf8 encoded
		std::string value_, unit_, value2_, unit2_; // optional properties for measurement attribute

		Entity_Ref entity_ref; // reference to entity vector, max number of entities in a sentence is 1028, so unsigned short should be enough
	};

	/*
	**	Path_Attribute represents the expansion of a semantic attribute (https://github.com/intersystems/iknow/wiki/Attributes)
	**  "type" : represents the attribute type
	**  "pos" : starting position in the path (a path is a vector of entity references)
	**  "span" : number of consecutive path entities 
	*/
	struct Path_Attribute // path attribute : expresses range of attributes in path
	{
		Attribute type; // attribute type
		unsigned short pos; // start position in path
		unsigned short span; // attibute span (number of path entities)
	};

	struct Sentence
	{
		typedef std::vector<Entity> Entities;
		typedef std::vector<Sent_Attribute> Sent_Attributes;
		typedef std::vector<Entity_Ref> Path;	// unsigned short indexes the Entity in the iKnow_Entities vector 
		typedef std::vector<Path_Attribute> Path_Attributes;	// expanded attributes in path 

		Entities			entities;	// the sentence entities
		Sent_Attributes		sent_attributes;	// the sentence attributes
		Path				path;		// the sentence path
		Path_Attributes		path_attributes;	// expanded attributes in the path

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
	void index(iknow::base::String& text_source, const std::string& language, bool b_trace=false);
	
	// Wrapper for indexing function that accepts UTF-8 encoded string instead. The offsets generated will be Unicode character
	// offsets, not byte offsets in text_source.
	void index(const std::string& text_source, const std::string& language, bool b_trace=false);

	// Adds User Dictionary annotations for customizing purposes
	void addUdctAnnotation(size_t start, size_t stop, const char* UdctLabel) {
		m_map_udct_annotations.insert(std::make_pair(start, iknow::core::IkIndexInput::IknowAnnotation(start, stop, UdctLabel)));
	}

	iknowdata::Text_Source m_index; // this is where all iKnow indexed information is stored after calling the "index" method.
	std::vector<std::string> m_traces; // optional collection of linguistic trace info, generated if b_trace equals true

private:
	iknow::core::IkIndexInput::mapInputAnnotations_t m_map_udct_annotations;

};



