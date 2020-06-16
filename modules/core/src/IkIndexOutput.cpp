// IndexOutput.cpp: implementation of the CIndexOutput class.
//
//////////////////////////////////////////////////////////////////////


#include "IkIndexOutput.h"

#include <sstream>
#include <numeric>

#include "IkPath.h"
#include "IkConceptProximity.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace iknow::core;
using namespace iknow::base;

using std::string;
using std::stringstream;

// sum of all summary relevances of lexreps
double IkIndexOutput::computeSummaryRelevanceCore() const
{
  double summaryRelevance = 0;
  for (Sentences::const_iterator i = SentencesBegin(); i != SentencesEnd(); ++i) {
    summaryRelevance += i->GetSummaryRelevance();
  }
  return summaryRelevance;
}

String IkIndexOutput::GetText() const
{
  String text;
  for (Sentences::const_iterator i = SentencesBegin(); i != SentencesEnd(); ++i) {
    text += i->GetText();
    text += SpaceString();
  }
  //remove last space
  if (!text.empty()) text.erase(text.size() - 1);
  return text;
}

String IkIndexOutput::GetNormalizedText() const
{
  String text;
  for (Sentences::const_iterator i = SentencesBegin(); i != SentencesEnd(); ++i) {
    text += i->GetNormalizedText();
    text += SpaceString();
  }
  //remove last space
  if (!text.empty()) text.erase(text.size() - 1);
  return text;
}

#define TYPE_SENTENCE		3
#define TYPE_CONCEPT		4
#define TYPE_RELATION		5
#define TYPE_NONRELEVANT	6
#define TYPE_CRC			8
#define TYPE_PATH			9
#define TYPE_DOCUMENT	   10
#define TYPE_LITERAL	   11

typedef size_t EntityId;
typedef size_t LiteralId;
typedef size_t StemId;
typedef size_t OccurrenceId;
typedef size_t CrcId;

//Simplify the syntax of declaring vectors that use the pool allocator
//(On Solaris don't bother...awful old STL implementation)
template<typename T>
struct poolvec {
#ifndef SOLARIS
	typedef std::vector<T, PoolAllocator<T> > type;
#else //SOLARIS
	typedef std::vector<T> type;
#endif //SOLARIS
};

template<typename K, typename V>
struct poolmap {
#ifndef SOLARIS
	typedef std::map<K, V, less<K>, PoolAllocator<std::pair<const K, V> > > type;
#else //SOLARIS
	typedef std::map<K, V> type;
#endif
};

// Dominance structures
typedef IkIndexOutput::Frequency Frequency;
typedef IkIndexOutput::Dominance Dominance;

struct EntityFrequencyInfo {
	Frequency concept;
	Frequency relation;
	Frequency concept_in_path; // new, frequency in path, needed for Dominance 2.0
	Frequency relation_in_path; // idem
	Dominance cDom; // (sum_of_single_word_frequencies/word_length) * total_unique_single_concept_words
	Dominance rDom; // entity_frequency * total_unique_relations
};

//Adds an K/V pair to a map of type M if there's no pair for
//k already in the map.
///Then, regardless, return
//the current V for K.
template<typename M, typename K, typename V>
static V AddToMap(M& m, const K& k, const V& v)
{
	typename M::iterator i = m.find(k);
	if (i != m.end()) {
		return i->second;
	}
	m.insert(typename M::value_type(k, v));
	return v;
}

typedef poolmap<WordPtr, Frequency>::type FreqMap;
//Builds the list of absolute singleWord source frequencies
struct mySingleWordCollector {
	FreqMap& singles_;
	size_t entity_multiplier_;

	mySingleWordCollector(FreqMap& mapSingles) : singles_(mapSingles) {}
	void reset(size_t multiplier) {
		entity_multiplier_ = multiplier;
	}
	void operator()(const iknow::base::Char* begin, const iknow::base::Char* end) {
		FreqMap::iterator it = singles_.find(WordPtr(begin, end)); // TODO : does this work ? matching text strings is not the same as matching text pointers
		if (it != singles_.end()) { // does exist
			it->second += entity_multiplier_; // increment frequency
		}
		else {
			singles_.insert(FreqMap::value_type(WordPtr(begin, end), entity_multiplier_)); // new 
		}
	}
private:
	void operator=(const mySingleWordCollector&);
};

typedef poolmap<WordPtr, double>::type dblFreqMap;
//Builds the list of P1 corrected concept and relation words
struct SingleWordP1Corrector {
	FreqMap& singles_;
	dblFreqMap& p1corrected_;

	SingleWordP1Corrector(FreqMap& mapSingles, dblFreqMap& mapCorrected, double dP1 = (double)1.0) : singles_(mapSingles), p1corrected_(mapCorrected), dP1_(dP1)
	{}
	void reset(bool bRightMostSignificant = true) {
		b_right_most_significant_ = bRightMostSignificant;
	}
	void operator()(const iknow::base::Char* begin, const iknow::base::Char* end) {
		Frequency frqSingleWord = singles_[WordPtr(begin, end)];
		dblFreqMap::iterator it = p1corrected_.find(WordPtr(begin, end));
		if (it != p1corrected_.end()) { // does exist
			it->second += (double)frqSingleWord; // increment frequency
		}
		else {
			p1corrected_.insert(dblFreqMap::value_type(WordPtr(begin, end), (double)frqSingleWord)); // new 
		}
	}
	void operator()(const iknow::base::Char* begin, const iknow::base::Char* end, bool bLP) {
		token_vct_.push_back(WordPtr(begin, end));
		if (bLP) {
			size_t entity_size = token_vct_.size();
			size_t position = b_right_most_significant_ ? 0 : entity_size; // start position
			for (vector<WordPtr>::iterator it = token_vct_.begin(); it != token_vct_.end(); it++) { // read left to right
				Frequency frqSingleWord = singles_[*it];
				b_right_most_significant_ ? position++ : position--; // increment position
				size_t factor = entity_size - position;
				double WeightSingleWord = (double)frqSingleWord / pow(dP1_, (double)factor);
				dblFreqMap::iterator itW = p1corrected_.find(*it);
				if (itW != p1corrected_.end()) { // does exist
					itW->second += (Frequency)WeightSingleWord; // increment frequency
				}
				else { // new
					p1corrected_.insert(dblFreqMap::value_type(*it, WeightSingleWord)); // new 
				}
			}
		}
		token_vct_.clear();
	}
	size_t Size() { return singles_.size(); }
private:
	void operator=(const SingleWordP1Corrector&);
	std::vector<WordPtr> token_vct_; // temporary token vector
	std::map<WordPtr, Frequency> tmpFreqMap_;  // temporary frequency map for local storage if position is involved
	bool b_right_most_significant_;
	double dP1_; // position factor
};

struct LocalIdeographicDominanceCalculator {
	FreqMap& singles_; // single symbols, adjusted
	FreqMap& doubles_; // double symbols...

	LocalIdeographicDominanceCalculator(FreqMap& mapSingles, FreqMap& mapDoubles) : singles_(mapSingles), doubles_(mapDoubles)
	{}
	void set_frequency(size_t freq) { frqConcept_ = freq; }
	void operator()(const iknow::base::Char* begin, const iknow::base::Char* end) {
		size_t con_length = (end - begin); // concept length
		size_t sum_singles = 0, sum_doubles = 0;
		while (begin != end) {
			sum_singles += singles_[WordPtr(begin, begin + 1)]; // cumulate single symbol frequency
			if ((end - begin) > 1) sum_doubles += doubles_[WordPtr(begin, begin + 2)]; // cumulate double symbol frequency
			++begin;
		}
		domConcept_ = frqConcept_ * (sum_singles / con_length) * sum_doubles;
	}
	size_t getDominance(void) {
		return domConcept_;
	}
private:
	void operator=(const LocalIdeographicDominanceCalculator&);
	size_t frqConcept_, domConcept_;
};

struct LocalDominanceCalculator {
	dblFreqMap& singles_; // frequency map of single words
	bool b_right_most_significant; // when position is important, the right position is most important
	vector<double> token_frq_;
	double sum_of_freqs_; // sum of individual word frequencies
	double dP1_; // local P1 factor

	LocalDominanceCalculator(dblFreqMap& mapSingles, double dP1, bool bModifiersOnLeft) : singles_(mapSingles), b_right_most_significant(bModifiersOnLeft), sum_of_freqs_(0), dP1_(dP1)
	{}
	void operator()(const iknow::base::Char* begin, const iknow::base::Char* end, bool bLP) {
		token_frq_.push_back(singles_[WordPtr(begin, end)]); // store token frequency
		if (bLP) {
			Frequency weight_token = (b_right_most_significant ? 1 : token_frq_.size());
			for (vector<double>::iterator it = token_frq_.begin(); it != token_frq_.end(); it++) { // consume token_fr from left to right
				sum_of_freqs_ += ((*it)*weight_token); // sum_of_freqs_+=(*it);
				b_right_most_significant ? weight_token++ : weight_token--;
			}
		}
	}
	void operator()(const iknow::base::Char* begin, const iknow::base::Char* end) {
		sum_of_freqs_ += singles_[WordPtr(begin, end)]; // accumulate frequency
	}
	double getDominance(void)
	{
		double local = sum_of_freqs_;
		token_frq_.clear();
		sum_of_freqs_ = (double) 0.0;
		return local;
	}
private:
	void operator=(const LocalDominanceCalculator&); // must not be copied
};


typedef size_t PathOffset;
struct DirectOutputPathAttribute {
	static const PathOffset kUnknown = static_cast<OccurrenceId>(-1);
	PropertyId type;
	PathOffset begin;
	PathOffset end;
	PathOffset continuation;
};

//Map from type to attribute
typedef poolvec<DirectOutputPathAttribute>::type DirectOutputPathAttributes;
typedef poolmap<PropertyId, DirectOutputPathAttributes>::type DirectOutputPathAttributeMap;
typedef poolvec<const IkMergedLexrep*>::type DirectOutputPathOffsets;
struct DirectOutputPath {
	DirectOutputPathOffsets offsets;
	DirectOutputPathAttributeMap attributes;
};

typedef poolvec<DirectOutputPath>::type DirectOutputPaths;

class pathLabelAttribute {
public:
	pathLabelAttribute(const IkKnowledgebase* kb, FastLabelSet::Index index, size_t pos) : kb_(kb), index_(index), pos_(pos) {}
	//AttributedIds are per-KB unique, so the pointer is as good as the value, and more useful with null
	AttributeId Type() const {
		return kb_->GetAttributeType(index_, pos_);
	}
	size_t ParamsCount() const {
		return ParamsEnd() - ParamsBegin();
	}
	const AttributeId* ParamsBegin() const {
		return kb_->GetAttributeParamsBegin(index_, pos_);
	}
	const AttributeId* ParamsEnd() const {
		return kb_->GetAttributeParamsEnd(index_, pos_);
	}
	const AttributeId* operator[](size_t param) const {
		return ParamsBegin() + param;
	}
private:
	const IkKnowledgebase* kb_;
	FastLabelSet::Index index_;
	size_t pos_;
};

class LabelAttributes {
public:
	LabelAttributes(const IkKnowledgebase* kb, FastLabelSet::Index index) : kb_(kb), index_(index) {}
	size_t Count() const {
		return kb_->GetAttributeCount(index_);
	}
	pathLabelAttribute GetAt(size_t pos) const {
		return pathLabelAttribute(kb_, index_, pos);
	}
private:
	const IkKnowledgebase* kb_;
	FastLabelSet::Index index_;
};

static PropertyId AttributeIdToPropertyId(AttributeId attr_id, const IkKnowledgebase& kb) {
	return kb.PropertyIdForName(kb.AttributeNameForId(attr_id)); //TODO: unnecessary string copy here, I bet.
}

template<typename Op>
static typename Op::first_argument_type AccumulateLexreps(const IkMergedLexrep& merged_lexrep, typename Op::first_argument_type init, Op op) {
	return std::accumulate(merged_lexrep.LexrepsBegin(), merged_lexrep.LexrepsEnd(), init, op);
}

template<typename Op>
static typename Op::first_argument_type AccumulateLabels(const IkLexrep& lexrep, typename Op::first_argument_type init, Op op) {
	const size_t label_count = lexrep.NumberOfLabels();
	for (size_t i = 0; i < label_count; ++i) {
		init = op(init, lexrep.GetLabelIndexAt(i));
	}
	return init;
}

template<typename Op>
static typename Op::first_argument_type AccumulateAttributes(const IkLexrep& lexrep, FastLabelSet::Index label, typename Op::first_argument_type init, Op op) {
	LabelAttributes attributes(lexrep.GetKnowledgebase(), label);
	const size_t attribute_count = attributes.Count();
	for (size_t i = 0; i < attribute_count; ++i) {
		init = op(init, attributes.GetAt(i));
	}
	return init;
}

static void BeginPathAttribute(DirectOutputPathAttributes& attributes, PropertyId type, PathOffset begin) {
	//TODO: When DirectOutput structs are real classes, this should be a member
	DirectOutputPathAttribute path_attribute;
	path_attribute.type = type;
	path_attribute.begin = begin;
	path_attribute.end = DirectOutputPathAttribute::kUnknown;
	path_attribute.continuation = begin;
	attributes.push_back(path_attribute);
}

static void EndPathAttribute(DirectOutputPathAttribute& path_attribute, PathOffset end) {
	//TODO: So should this.
	path_attribute.end = end;
}

static void ContinuePathAttribute(DirectOutputPathAttribute& path_attribute, PathOffset continuation) {
	path_attribute.continuation = continuation;
	path_attribute.end = DirectOutputPathAttribute::kUnknown;
}

struct PathAttributeAttributeAcc : public std::binary_function<DirectOutputPathAttributeMap*, const pathLabelAttribute&, DirectOutputPathAttributeMap*> {
	const IkKnowledgebase& kb_;
	PathOffset path_offset_;
	AttributeId path_type_;
	AttributeId begin_param_;
	AttributeId end_param_;
	PathAttributeAttributeAcc(const IkKnowledgebase& kb, PathOffset path_offset, AttributeId path_type, AttributeId begin_param, AttributeId end_param) :
		kb_(kb),
		path_offset_(path_offset),
		path_type_(path_type), begin_param_(begin_param), end_param_(end_param) {}

	DirectOutputPathAttributeMap* operator()(DirectOutputPathAttributeMap* path_attribute_map, const pathLabelAttribute& attribute) {
		if (attribute.Type() == path_type_) {
			if (attribute.ParamsCount() < 2) {
				throw Exception("Path attribute missing necessary arguments.");
			}
			// We have to be able to build attributes in either order, in case we have a begin and an end on the same path on the
			// same lexrep and see the end attribute first.
			if (*attribute[0] == begin_param_) {
				PropertyId type = AttributeIdToPropertyId(*attribute[1], kb_);
				DirectOutputPathAttributeMap::iterator path_attribute_iter = path_attribute_map->find(type);
				if (path_attribute_iter == path_attribute_map->end()) { // new for type
					DirectOutputPathAttributes attributes;
					BeginPathAttribute(attributes, type, path_offset_); // insert attribute start
					(*path_attribute_map)[type] = attributes;
				}
				else {
					DirectOutputPathAttribute& cur_attribute = path_attribute_iter->second.back();
					if (cur_attribute.end == DirectOutputPathAttribute::kUnknown || cur_attribute.end == path_offset_) { // Don't stop if we have a begin/end on the same lexrep.
						ContinuePathAttribute(cur_attribute, path_offset_);
					}
					else { // Last one is complete, add a new one.
						BeginPathAttribute(path_attribute_iter->second, type, path_offset_);
					}
				}
			}
			else if (*attribute[0] == end_param_) {
				PropertyId type = AttributeIdToPropertyId(*attribute[1], kb_);
				DirectOutputPathAttributeMap::iterator path_attribute_iter = path_attribute_map->find(type);
				if (path_attribute_iter != path_attribute_map->end()) { // Is there one in progress? Ignore otherwise.
					DirectOutputPathAttribute& cur_attribute = path_attribute_iter->second.back();
					if (cur_attribute.end == DirectOutputPathAttribute::kUnknown && // Must be unfinished and not have started at this same path position (linguist request)
						cur_attribute.begin != path_offset_ &&
						cur_attribute.continuation != path_offset_) {
						EndPathAttribute(cur_attribute, path_offset_);
					}
				}
			}
			else {
				throw Exception("Path attribute arg 0 invalid (must be Begin or End)");
			}
		}
		return path_attribute_map;
	}
private:
	void operator=(const PathAttributeAttributeAcc&);
};

struct PathAttributeLabelAcc : public std::binary_function<DirectOutputPathAttributeMap*, FastLabelSet::Index, DirectOutputPathAttributeMap*> {
	const IkLexrep& lexrep_; //TODO: Could wrap up in a struct with the index like LabelAttribute.
	PathOffset path_offset_;
	AttributeId path_type_;
	AttributeId path_begin_;
	AttributeId path_end_;
	PathAttributeLabelAcc(const IkLexrep& lexrep, PathOffset path_offset, AttributeId path_type, AttributeId path_begin, AttributeId path_end) :
		lexrep_(lexrep), path_offset_(path_offset), path_type_(path_type), path_begin_(path_begin), path_end_(path_end) {}
	DirectOutputPathAttributeMap* operator()(DirectOutputPathAttributeMap* path_attribute_map, FastLabelSet::Index label) {
		PathAttributeAttributeAcc attribute_accumulate(*lexrep_.GetKnowledgebase(), path_offset_, path_type_, path_begin_, path_end_);
		return AccumulateAttributes(lexrep_, label, path_attribute_map, attribute_accumulate);
	}
private:
	void operator=(const PathAttributeLabelAcc&);
};

struct PathAttributeLexrepAcc : public std::binary_function<DirectOutputPathAttributeMap*, const IkLexrep&, DirectOutputPathAttributeMap*> {
	PathOffset path_offset_;
	const String& path_string_;
	const String& begin_string_;
	const String& end_string_;
	PathAttributeLexrepAcc(PathOffset path_offset, const String& path_string, const String& begin_string, const String& end_string) :
		path_offset_(path_offset), path_string_(path_string), begin_string_(begin_string), end_string_(end_string) {}

	DirectOutputPathAttributeMap* operator()(DirectOutputPathAttributeMap* path_attribute_map, const IkLexrep& lexrep) {
		PathAttributeLabelAcc label_accumulate(lexrep,
			path_offset_,
			lexrep.GetKnowledgebase()->AttributeIdForName(path_string_),
			lexrep.GetKnowledgebase()->AttributeIdForName(begin_string_),
			lexrep.GetKnowledgebase()->AttributeIdForName(end_string_));
		return AccumulateLabels(lexrep, path_attribute_map, label_accumulate);
	}
private:
	void operator=(const PathAttributeLexrepAcc&);
};

static void DetectPathAttributes(const IkMergedLexrep& merged_lexrep, PathOffset path_offset, DirectOutputPathAttributeMap& path_attribute_map) {
	static const String kPathString = IkStringEncoding::UTF8ToBase("Path");
	static const String kBeginString = IkStringEncoding::UTF8ToBase("Begin");
	static const String kEndString = IkStringEncoding::UTF8ToBase("End");

	PathAttributeLexrepAcc lexrep_accumulator(path_offset, kPathString, kBeginString, kEndString);
	AccumulateLexreps(merged_lexrep, &path_attribute_map, lexrep_accumulator);
}

/*
static const String kEntityString = IkStringEncoding::UTF8ToBase("Entity");
static const String kNegationString = IkStringEncoding::UTF8ToBase("Negation");
static const String kPositiveSentimentString = IkStringEncoding::UTF8ToBase("PositiveSentiment");
static const String kNegativeSentimentString = IkStringEncoding::UTF8ToBase("NegativeSentiment");
*/

void IkIndexOutput::CalculateDominanceAndProximity()
{
	bool entity_extra_info = true; // need to set for proximity and dominance
	bool bIsIdeographic = IsJP();
	const String lexrep_separator = (bIsIdeographic ? String() : SpaceString()); // no separator char for Japanese
	size_t sentences_size = SentencesEnd() - SentencesBegin();

	// Settings for semantic dominance, are in fact sentence based, but are calculated on a source-scale, so we use the settings of the first sentence !
	double PScale = (sentences_size ? static_cast<double>(SentencesBegin()->GetPScale()) : (double)100.0);
	double dP1 = (sentences_size ? static_cast<double>(SentencesBegin()->GetP1()) / PScale : (double) 2.0);
	double dP2 = (sentences_size ? static_cast<double>(SentencesBegin()->GetP2()) / PScale : (double) 0.5);
	double dP3 = (sentences_size ? static_cast<double>(SentencesBegin()->GetP3()) / PScale : (double) 0.3); // default is 0.3, not 0.5
	double dP4 = (sentences_size ? static_cast<double>(SentencesBegin()->GetP4()) / PScale : (double) 0.0);
	bool bModifiersOnLeft = (sentences_size ? SentencesBegin()->bModifiersOnLeft() : true);
	/*
	if (debug) { // trace current parameter return iknow::base::IkStringEncoding::UTF8ToBase(s)
		debug->Parameter(IkStringEncoding::UTF8ToBase("PScale"), PScale);
		debug->Parameter(IkStringEncoding::UTF8ToBase("P1"), dP1);
		debug->Parameter(IkStringEncoding::UTF8ToBase("P2"), dP2);
		debug->Parameter(IkStringEncoding::UTF8ToBase("P3"), dP3);
		debug->Parameter(IkStringEncoding::UTF8ToBase("P4"), dP4);
		debug->Parameter(IkStringEncoding::UTF8ToBase("ModifiersOnLeft"), bModifiersOnLeft ? (double) 1.0 : (double) 0.0);
	}
	*/

	//Maps each entity string to its entity data object
	typedef IndexMap<WordPtr, EntityFrequencyInfo, hash_wordptr> EntityMap;
	EntityMap entity_map(sentences_size * 8);

	//Maps each lexrep occurrence (CRCs and sentence lexrep lists share values, so the pointer
	//is an identifier) to its occurrence offset.
	typedef std::unordered_map<IkMergedLexrep const*, OccurrenceId> LexrepOffsetMap;
	LexrepOffsetMap offset_map;
	AddToMap(offset_map, (IkMergedLexrep const*)0, static_cast<OccurrenceId>(0)); //null lexreps map to 0

	OccurrenceId occurrence_count = 0;

	m_concept_proximity.rehash(sentences_size * 32); // set ready for calculating and collecting concept proximity data

	size_t path_count = 0;
	DirectOutputPaths path_vector; // collector for path data

	for (Sentences::const_iterator i = SentencesBegin(); i != SentencesEnd(); ++i) { // scroll over sentences
		const IkSentence* sentence = &(*i);
		/*
		const IkKnowledgebase* kb = sentence->GetLexrepsBegin()->LexrepsBegin()->GetKnowledgebase(); // KB does not change in a sentence.
		const iknow::core::AttributeId entity_attribute_type = kb->AttributeIdForName(kEntityString);
		const iknow::core::AttributeId negation_attribute_type = kb->AttributeIdForName(kNegationString);
		const iknow::core::AttributeId positive_sentiment_attribute_type = kb->AttributeIdForName(kPositiveSentimentString);
		const iknow::core::AttributeId negative_sentiment_attribute_type = kb->AttributeIdForName(kNegativeSentimentString);
		*/

		for (MergedLexreps::const_iterator j = sentence->GetLexrepsBegin(); j != sentence->GetLexrepsEnd(); ++j) { // scroll over (merged) lexreps
			const IkMergedLexrep *lexrep = &(*j);
			IkLexrep::Type type = lexrep->GetLexrepType();
			EntityId entity_id = 0;
			// LiteralId literal_id = 0;
			// StemId stem_id = 0;
			const String* entity_value = &(lexrep->GetNormalizedValue(lexrep_separator));
			const String* literal_value = &(lexrep->GetValue(lexrep_separator));

			if (literal_value->empty()) { // correct for empty literal
				literal_value = entity_value;
			}
			if (entity_value->empty()) { // correct for empty entity
				type = IkLabel::Nonrelevant;
			}
			switch (type) {
			case IkLabel::Relation: //relations and concepts always have an entity, and may need a literal
			case IkLabel::Concept:
			case IkLabel::PathRelevant:
			{
				++occurrence_count;
				AddToMap(offset_map, lexrep, occurrence_count); // map lexrep to occurence_id
				entity_id = entity_map.Insert(WordPtr(*entity_value));
				// std::map<const IkMergedLexrep*, EntityId> m_mapLexrepEntID;
				m_mapLexrepEntID.insert(std::make_pair(lexrep, entity_id));
				if (type != IkLabel::PathRelevant) { // calculate absolute frequency
					EntityFrequencyInfo& frequency_info = entity_map.GetMetadata(entity_id);
					++(type == IkLabel::Relation ? frequency_info.relation : frequency_info.concept);
				}
			}
			if (*entity_value == *literal_value) break; // If the literal value is the same as the entity value, we're done

			case IkLabel::Nonrelevant: // nonrelevants always have a literal, but no entity
			case IkLabel::Unknown:
			{
			} break;

			default:
				throw ExceptionFrom<IkIndexOutput>("Unknown lexrep type encountered.");
			}
		}

		using iknow::core::path::CRC;
		using iknow::core::path::CRCs;
		using iknow::core::path::Offsets;

		//
		// calculate proximity
		//
		if (bIsIdeographic && entity_extra_info) { // no paths for Japanese, calculate proximity out of Entity Vectors...
			MergedLexreps& merged_lexrep_vector = const_cast<IkSentence*>(sentence)->GetLexreps();
			IkConceptProximity::ProxPoints_t concept_proximity;
			concept_proximity.reserve(const_cast<IkSentence*>(sentence)->GetEntityVector().size());

			for (IkSentence::EntityVector::const_iterator itEV = sentence->GetEntityVectorBegin(); itEV != sentence->GetEntityVectorEnd(); ++itEV) { // scroll over the sentence entity vectors
				IkMergedLexrep* lexrep = &(merged_lexrep_vector[*itEV]);
				OccurrenceId occurrence_id = offset_map[lexrep];
				EntityId entity_id = entity_map.Insert(WordPtr(lexrep->GetNormalizedValue(lexrep_separator)));
				concept_proximity.push_back(IkConceptProximity::ProxPoint_t(occurrence_id, entity_id));
			}
			m_concept_proximity(concept_proximity); // calculate concept proximity
		}
		else { // calculate proximity out of path info
			for (IkSentence::Paths::const_iterator j = sentence->GetPathsBegin(); j != sentence->GetPathsEnd(); ++j) {
				const IkPath* path = &(*j);
				DirectOutputPath output_path;
				output_path.offsets.reserve(path->Size());
				/*
				DirectOutputPath output_path_extra;
				if (entity_extra_info) output_path_extra.offsets.reserve(path->Size());
				*/
				IkConceptProximity::ProxPoints_t concept_proximity;
				concept_proximity.reserve(path->Size()); // will be a little smaller though...

				//A valid path must have both a relation and a concept.
				bool path_has_relation = false;
				bool path_has_concept = false;
				for (Offsets::const_iterator k = path->OffsetsBegin(); k != path->OffsetsEnd(); ++k) {
					const IkMergedLexrep* lexrep = &(*(sentence->GetLexrepsBegin() + (*k)));
					path_has_relation |= lexrep->IsRelation();
					path_has_concept |= lexrep->IsConcept();
					OccurrenceId occurrence_id = offset_map[lexrep];
					output_path.offsets.push_back(lexrep);
					DetectPathAttributes(*lexrep, output_path.offsets.size() - 1, output_path.attributes); // Detect path attributes
					if (entity_extra_info) {
						// output_path_extra.offsets.push_back(*k); // we need to preserve the sentence offsets
						EntityId entity_id = entity_map.Insert(WordPtr(lexrep->GetNormalizedValue(lexrep_separator))); // get the entity id
						EntityFrequencyInfo& ent_freq = entity_map.GetMetadata(entity_id); // entity_id is part of path, count path frequency
						if (lexrep->IsConcept()) {
							++ent_freq.concept_in_path;
							concept_proximity.push_back(IkConceptProximity::ProxPoint_t(occurrence_id, entity_id));
						}
						if (lexrep->IsRelation()) {
							++ent_freq.relation_in_path;
						}
						// if (lexrep->IsPathRelevant) TODO ?
					}
				}
				//Set the end of unfinished begin path attributes to the end of the path
				for (DirectOutputPathAttributeMap::iterator k = output_path.attributes.begin(); k != output_path.attributes.end(); ++k) {
					DirectOutputPathAttribute& last_attribute = k->second.back();
					if (last_attribute.end == DirectOutputPathAttribute::kUnknown) {
						EndPathAttribute(last_attribute, output_path.offsets.size()); // force path attribute end
					}
				}

				if ((path_has_relation && path_has_concept) || (sentence->GetPathConstruction() == iknow::core::kPathRelevant)) { // if kPathRelevant, always emit path info
					++path_count;
					path_vector.push_back(output_path);
					if (entity_extra_info) {
						// path_vector_extra.push_back(output_path_extra); // store extra dominance information
						m_concept_proximity(concept_proximity); // calculate concept proximity
					}
				}
			}
		}
	}

	// path data handling : DirectOutputPaths path_vector; 
	std::vector<const IkMergedLexrep*> path_attributes;
	path_attributes.reserve(8); // big enough to avoid reallocations
	for (DirectOutputPaths::iterator it = path_vector.begin(); it != path_vector.end(); ++it) {
		DirectOutputPath path = *it;
		for (DirectOutputPathAttributeMap::iterator it_attr_map = path.attributes.begin(); it_attr_map != path.attributes.end(); ++it_attr_map) { // iterate the attributes
			// const PropertyId& type = it_attr_map->first;
			const DirectOutputPathAttributes& attributes = it_attr_map->second;

			for (DirectOutputPathAttributes::const_iterator it_attr = attributes.begin(); it_attr != attributes.end(); ++it_attr) {
				PropertyId type = it_attr->type;
				PathOffset begin = it_attr->begin;
				PathOffset end = it_attr->end;
				if (end == begin) ++end;
				path_attributes.clear();
				for (PathOffset itPath = begin; itPath != end; ++itPath) {
					path_attributes.push_back(path.offsets[itPath]);
				}
				vecAttributePaths_.push_back(std::make_pair(type, path_attributes));
			}
		}
	}
	size_t uni_con_count = 0;  // unique concepts count
	size_t uni_rel_count = 0; // unique relations count
	// size_t semdom_con_count = 0; // sum of concept semantic dominance
	// size_t semdom_rel_count = 0; // sum of relation semantic dominance
	// size_t semdom_crc_count = 0; // sum of crc semantic dominance
	// size_t uni_con_singleword_count = 0; // number of unique concept single words
	size_t nbr_occurrences_in_text = 0; // number of entity occurences in source

	if (entity_extra_info) { // calculate entity dominance
		FreqMap singleWordsInSource, singleIdeographsInSource;
		mySingleWordCollector singleWordsInSourceBuilder(singleWordsInSource);
		mySingleWordCollector singleIdeographsInSourceBuilder(singleIdeographsInSource);
		dblFreqMap singleCWords, singleRWords;
		SingleWordP1Corrector singleCWordsBuilder(singleWordsInSource, singleCWords, dP1);
		SingleWordP1Corrector singleRWordsBuilder(singleWordsInSource, singleRWords);

		// first count the absolute frequency of the single words
		EntityId entity_id = 1;
		for (EntityMap::const_iterator_keys it = entity_map.begin_keys(); it != entity_map.end_keys(); it++) { // scroll over the entities
			const WordPtr& keyst = *it;
			EntityFrequencyInfo& frequency_info = entity_map.GetMetadata(entity_id);
			if (!bIsIdeographic) {
				Frequency frq_conrel = frequency_info.concept + frequency_info.relation;
				if (frq_conrel > static_cast<Frequency>(0)) { // can be zero because of new PathRelevant elements.
					singleWordsInSourceBuilder.reset(frequency_info.concept + frequency_info.relation); // count total frequency
					iknow::base::IkStringAlg::Tokenize(keyst.begin(), keyst.end(), static_cast<Char>(' '), singleWordsInSourceBuilder);
				}
			}
			else {
				if (frequency_info.concept > 0) { // This entity is a concept !
					singleIdeographsInSourceBuilder.reset(1); // real concept frequency does not count
					singleWordsInSourceBuilder.reset(1);
					const Char* token_begin = keyst.begin();
					const Char* token_end = keyst.end();
					while (token_begin != token_end) {
						singleIdeographsInSourceBuilder(token_begin, token_begin + 1); // collect single symbols
						if ((token_end - token_begin) > 1) singleWordsInSourceBuilder(token_begin, token_begin + 2); // collect (overlapping) double symbols
						++token_begin;
					}
				}
			}
			entity_id++;
		}
		if (bIsIdeographic) { // Correct the singleIdeographs
			for (FreqMap::iterator it = singleIdeographsInSource.begin(); it != singleIdeographsInSource.end(); it++) { // scroll over frequency map
				if (iknow::base::IkStringAlg::IsKatakana(*(it->first.begin()))) it->second = (size_t)2; // Katakana single symbol set to frequency 2
				else if (iknow::base::IkStringAlg::IsHiragana(*(it->first.begin()))) it->second = (size_t)1; // Hiragana single symbol set to frequency 1
				else if (iknow::base::IkStringAlg::IsJpnNumber(*(it->first.begin()))) it->second = (size_t)1; // Number single symbol set to frequency 1
			}
			/*
			if (debug) { // trace Single Ideographs
				for (FreqMap::const_iterator cit = singleIdeographsInSource.begin(); cit != singleIdeographsInSource.end(); ++cit) debug->SingleWordFreq(cit->first, cit->second);
			}
			*/
		}
		/*
		if (debug) { // trace Single Word Frequencies
			for (FreqMap::const_iterator cur = singleWordsInSource.begin(); cur != singleWordsInSource.end(); ++cur) debug->SingleWordFreq(cur->first, cur->second);
		}
		*/
		if (!bIsIdeographic) { // calculate positional relative 
			entity_id = 1;
			for (EntityMap::const_iterator_keys it = entity_map.begin_keys(); it != entity_map.end_keys(); it++) { // scroll over the entities
				const WordPtr& keyst = *it;
				EntityFrequencyInfo& frequency_info = entity_map.GetMetadata(entity_id);
				if (frequency_info.concept > 0) { // entity is a concept, break into single words
					nbr_occurrences_in_text += frequency_info.concept;
					uni_con_count++;
					singleCWordsBuilder.reset(bModifiersOnLeft);
					iknow::base::IkStringAlg::TokenizeWithLPFlag(keyst.begin(), keyst.end(), static_cast<Char>(' '), singleCWordsBuilder); // position is at stake
				}
				if (frequency_info.relation > 0) { // entity is a relation
					nbr_occurrences_in_text += frequency_info.relation;
					uni_rel_count++;
					singleRWordsBuilder.reset(bModifiersOnLeft);
					iknow::base::IkStringAlg::Tokenize(keyst.begin(), keyst.end(), static_cast<Char>(' '), singleRWordsBuilder); // no position relevance
				}
				entity_id++;
			}
			/*
			if (debug) { // trace Concept word & Relation word Frequencies
			for (dblFreqMap::const_iterator con_cur = singleCWords.begin(); con_cur != singleCWords.end(); ++con_cur) debug->TraceKeyDouble("ConceptWordFreq", con_cur->first, con_cur->second);
			for (dblFreqMap::const_iterator rel_cur = singleRWords.begin(); rel_cur != singleRWords.end(); ++rel_cur) debug->TraceKeyDouble("RelationWordFreq", rel_cur->first, rel_cur->second);
			}
			*/
		}

		// calculate local dominance per entity
		LocalDominanceCalculator conceptDominance(singleCWords, dP1, bModifiersOnLeft);
		LocalDominanceCalculator relationDominance(singleRWords, dP1, bModifiersOnLeft);
		LocalIdeographicDominanceCalculator ideoDominance(singleIdeographsInSource, singleWordsInSource);
		double DomConceptMax = (double)0.0, DomRelationMax = (double)0.0; // keep maximum values for normalization
		Dominance IdeoDomMax = static_cast<Dominance>(0);

		entity_id = 1;
		for (EntityMap::const_iterator_keys it = entity_map.begin_keys(); it != entity_map.end_keys(); ++it) {
			const WordPtr& keyst = *it;
			EntityFrequencyInfo& frequency_info = entity_map.GetMetadata(entity_id);
			if (frequency_info.concept > 0) { // entity is a concept
				if (bIsIdeographic) {
					ideoDominance.set_frequency(frequency_info.concept);
					ideoDominance(keyst.begin(), keyst.end());
					Dominance Dom = ideoDominance.getDominance();
					// if (debug) debug->TraceKeyDouble("LocalConceptDominance", keyst, static_cast<double>(Dom));  // trace local concept dominance
					if (Dom > IdeoDomMax) IdeoDomMax = Dom;
					frequency_info.cDom = Dom;
				}
				else {
					iknow::base::IkStringAlg::TokenizeWithLPFlag(keyst.begin(), keyst.end(), static_cast<Char>(' '), conceptDominance);
					double Dom = static_cast<double>(conceptDominance.getDominance());
					// if (debug) debug->TraceKeyDouble("LocalConceptDominance", keyst, Dom);  // trace local concept dominance
					Dom = Dom * (((double)1.0 - dP2) + dP2 * (double)frequency_info.concept); // entity frequency corrected
					// if (debug) debug->TraceKeyDouble("LCDFCorrected", keyst, Dom); // trace
					Dom = Dom * (((double)1.0 - dP3) + dP3 * (double)frequency_info.concept_in_path / (double)frequency_info.concept); // path frequency corrected
					// if (debug) debug->TraceKeyDouble("LCDPCorrected", keyst, Dom); // trace
					if (Dom > DomConceptMax) DomConceptMax = Dom;
					frequency_info.cDom = static_cast<Dominance>(Dom);
				}
			}
			if (frequency_info.relation > 0 && !bIsIdeographic) { // entity is a relation, and not ideographical
				iknow::base::IkStringAlg::Tokenize(keyst.begin(), keyst.end(), static_cast<Char>(' '), relationDominance);
				double Dom = static_cast<double>(relationDominance.getDominance());
				// if (debug) debug->TraceKeyDouble("LocalRelationDominance", keyst, Dom); // trace local relation dominance
				Dom = Dom*(((double)1.0 - dP2) + dP2*(double)frequency_info.relation); // entity frequency corrected
				// if (debug) debug->TraceKeyDouble("LRDFCorrected", keyst, Dom); // trace
				Dom = Dom*(((double)1.0 - dP3) + dP3*(double)frequency_info.relation_in_path / (double)frequency_info.relation); // path frequency corrected
				// if (debug) debug->TraceKeyDouble("LRDPCorrected", keyst, Dom); // trace
				if (Dom > DomRelationMax) DomRelationMax = Dom;
				frequency_info.rDom = static_cast<Dominance>(Dom);
			}
			entity_id++;
		}
		// Normalize with P4
		if (bIsIdeographic) DomConceptMax = static_cast<double>(IdeoDomMax);
		for (entity_id = 1; entity_id <= entity_map.Size(); ++entity_id) {
			const double OUTPUT_SCALE = (double) 1000.0;
			// StemMap::const_iterator itStem = stem_ent_map.find(entity_id); // does this entity have a stem ?
			EntityFrequencyInfo& frequency_info = entity_map.GetMetadata(entity_id);
			if (frequency_info.concept > 0) { // entity is concept
				double Dom = dP4 + (((double)1.0 - dP4)*static_cast<double>(frequency_info.cDom) / DomConceptMax); // Normalize to in between P4 <> 1
				frequency_info.cDom = static_cast<Dominance>(Dom*OUTPUT_SCALE);
				m_concept_dominance.insert(std::make_pair(entity_id, frequency_info.cDom));
			}
			if (frequency_info.relation > 0 && !bIsIdeographic) { // entity is relation
				double Dom = dP4 + (((double)1.0 - dP4)*static_cast<double>(frequency_info.rDom) / DomRelationMax); // Normalize to in between P4 <> 1
				frequency_info.rDom = static_cast<Dominance>(Dom*OUTPUT_SCALE);
				m_relation_dominance.insert(std::make_pair(entity_id, frequency_info.rDom));
				// m_relation_dominance.insert(EntityDominance(entity_id, frequency_info.cDom));
			}
		}
	}
}
