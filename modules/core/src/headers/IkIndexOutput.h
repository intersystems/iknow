// IndexOutput.h: interface for the CIndexOutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INDEXOUTPUT_H__271239BD_39FF_497C_9DDC_70960B9823D8__INCLUDED_)
#define AFX_INDEXOUTPUT_H__271239BD_39FF_497C_9DDC_70960B9823D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <functional>
#include <vector>
#include <utility>
#include <cstring>

#include "IkExportCore.h"
#include "IkObject.h"
#include "IkSentence.h"
#include "SafeString.h"
#include "StringPool.h"
#include "PoolAllocator.h"
#include "IkConceptProximity.h"
#include "Utility.h"

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

typedef size_t EntityId;
typedef size_t LiteralId;
typedef size_t StemId;
typedef size_t OccurrenceId;
typedef size_t CrcId;

typedef size_t PathOffset;
struct DirectOutputPathAttribute {
	static const PathOffset kUnknown = static_cast<OccurrenceId>(-1);
	iknow::core::PropertyId type;
	PathOffset begin;
	PathOffset end;
	PathOffset continuation;
};

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
	typedef std::map<K, V, std::less<K>, PoolAllocator<std::pair<const K, V> > > type;
#else //SOLARIS
	typedef std::map<K, V> type;
#endif
};

//Map from type to attribute
typedef poolvec<DirectOutputPathAttribute>::type DirectOutputPathAttributes;
typedef poolmap<iknow::core::PropertyId, DirectOutputPathAttributes>::type DirectOutputPathAttributeMap;
typedef poolvec<const iknow::core::IkMergedLexrep*>::type DirectOutputPathOffsets;
struct DirectOutputPath {
	DirectOutputPathOffsets offsets;
	DirectOutputPathAttributeMap attributes;
};

typedef poolvec<DirectOutputPath>::type DirectOutputPaths;

namespace iknow 
{
  namespace core
  {
    typedef std::vector<IkSentence, iknow::base::PoolAllocator<IkSentence> > Sentences;
	typedef std::vector<DirectOutputPaths, iknow::base::PoolAllocator<DirectOutputPaths> > Paths;

    class WordPtr {
    public:
      WordPtr(const iknow::base::Char* begin, const iknow::base::Char* end) : begin_(begin), end_(end) {}
      explicit WordPtr(const iknow::base::String& s) : begin_(s.data()), end_(s.data() + s.size()) {}
      bool operator==(const WordPtr& other) const {
	return size() == other.size() && !std::memcmp(begin_, other.begin(), size() * sizeof(iknow::base::Char));
      }
      bool operator<(const WordPtr& other) const {
	if (other.size() > size()) {
	  int cmp = std::memcmp(begin_, other.begin(), size() * sizeof(iknow::base::Char));
	  if (!cmp) return 1; //equal up to where they differ in size, other is greater (longer)
	  return cmp < 0;
	}
	else {
	  int cmp = std::memcmp(begin_, other.begin(), other.size() * sizeof(iknow::base::Char));
	  if (!cmp) return 0; //equal up to where they differ in size, i'm greater (longer)
	  return cmp < 0;
	}
      }
      operator iknow::base::String() const { return iknow::base::String(begin(), end()); }

      size_t size() const { return end_ - begin_; }
      const iknow::base::Char* begin() const { return begin_; }
      const iknow::base::Char* end() const { return end_; }
    private:
      const iknow::base::Char* begin_;
      const iknow::base::Char* end_;
    };

    struct hash_wordptr {
      std::size_t operator()(const WordPtr& w) const {
	return hash_range(w.begin(), w.end());
      }
    };

    /*!
     *	This output object contains a list of sentences
     *	Exceptions: IkException 
     */
	class CORE_API IkIndexOutput : public IkDocumentPart
	{
	public:
		IkIndexOutput() : m_IsJP(false), m_right_most_significant(true) { sentences_.reserve(8); }  /*!< Constructor. */

		typedef size_t EntityId;

		typedef size_t Dominance;
		typedef size_t Frequency;
		typedef std::pair<EntityId, Dominance> EntityDominance;
		// typedef std::vector<std::pair<PropertyId, std::pair<const IkMergedLexrep*, const IkMergedLexrep*> > > vecAttributePaths;
		typedef std::vector<std::pair<PropertyId, std::vector<const IkMergedLexrep*> > > vecAttributePaths;

		typedef std::unordered_map<WordPtr,size_t,hash_wordptr> WordCounts;

		typedef std::map<EntityId, Dominance> EntityDominanceMap;

		void Add(const IkSentence& sentence) { sentences_.push_back(sentence); }
		IkSentence& LastSentence() { return sentences_.back(); }
		void RemoveLastSentence() { sentences_.pop_back(); }
		Sentences::const_iterator SentencesBegin() const { return sentences_.begin(); }
		Sentences::const_iterator SentencesEnd() const { return sentences_.end(); }
		Sentences::const_reverse_iterator SentencesRBegin() const { return sentences_.rbegin(); }
		Sentences::const_reverse_iterator SentencesREnd() const { return sentences_.rend(); }
		Sentences::iterator SentencesBegin() { return sentences_.begin(); }
		Sentences::iterator SentencesEnd() { return sentences_.end(); }
		Sentences::reverse_iterator SentencesRBegin() { return sentences_.rbegin(); }
		Sentences::reverse_iterator SentencesREnd() { return sentences_.rend(); }
		vecAttributePaths::iterator AttributePathsBegin() { return vecAttributePaths_.begin(); }
		vecAttributePaths::iterator AttributePathsEnd() { return vecAttributePaths_.end(); }

		WordCounts const & GetWordCounts() const { return m_wordCounts; }

		virtual iknow::base::String GetText() const; // inherited from IkDocumentPart
		virtual iknow::base::String GetNormalizedText() const; // inherited from IkDocumentPart
		void SetJP(bool IsJP) { m_IsJP = IsJP; };
		bool IsJP(void) const { return m_IsJP; };
		void SetRMS(bool right_most_significant) { m_right_most_significant = right_most_significant; };
		bool IsRMS(void) const { return m_right_most_significant; };
		void SetTextPointer(iknow::base::Char const * const pTextData) { m_pTextData = pTextData; };
		iknow::base::Char const *GetTextPointer(void) const { return m_pTextData; };
		void CalculateDominanceAndProximity(); // do something usefull...

		void GetProximityPairVector(IkConceptProximity::ProximityPairVector_t& proximity_pair_vector) {
			proximity_pair_vector = m_concept_proximity.GetSortedProximityPairs(); // no stemming
		}

		EntityId GetConceptID(const IkMergedLexrep* lexrep) {
			if (lexrep->IsConcept()) return m_mapLexrepEntID[lexrep];
			else return static_cast<EntityId>(-1); // not a concept
		}
		EntityId GetEntityID(const IkMergedLexrep* lexrep) {
			return m_mapLexrepEntID[lexrep];
		}
		double GetEntityDominance(const IkMergedLexrep* lexrep) {
			EntityId ent_id = m_mapLexrepEntID[lexrep]; // get the entity id
			if (lexrep->IsConcept()) return static_cast<double>(m_concept_dominance[ent_id]);
			if (lexrep->IsRelation()) return static_cast<double>(m_relation_dominance[ent_id]);
			return static_cast<double>(0.0);
		}
		
    protected:
      virtual double computeSummaryRelevanceCore() const; // inherited from IkDocumentPart
      Sentences sentences_;
	  Paths paths_;
	  vecAttributePaths vecAttributePaths_;

    private:
      WordCounts m_wordCounts; // summarizer is responsable for maintaining the wordCounts // TODO: is this ok?
      friend class IkSummarizer;
	  bool m_IsJP; // indicator for Japanese
      bool m_right_most_significant; // modifiers put on the left (Dutch & English), if false, modifiers put on the right (French, Spanish, Portuguese)
  	  const iknow::base::Char *m_pTextData; // store pointer to text data 

	  IkConceptProximity m_concept_proximity;
	  EntityDominanceMap m_concept_dominance, m_relation_dominance;
	  std::map<const IkMergedLexrep*, EntityId> m_mapLexrepEntID;
	  
    };
  }
}
#ifdef WIN32
#pragma warning(pop)
#endif

#endif // !defined(AFX_INDEXOUTPUT_H__271239BD_39FF_497C_9DDC_70960B9823D8__INCLUDED_)
