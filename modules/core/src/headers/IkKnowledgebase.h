#ifndef IKNOW_CORE_IKKNOWLEDGEBASE_H_
#define IKNOW_CORE_IKKNOWLEDGEBASE_H_

#include "SafeString.h"

#include "IkExportCore.h"
#include "IkLexrep.h"
#include "IkLabel.h"
#include "IkFilter.h"
#include "IkTypes.h"
#include "IkRule.h"
#include "IkMetadataCache.h"

namespace iknow
{
	
  namespace core
  {

    //For semantic properties
    typedef unsigned short PropertyId;
    const PropertyId kNoSuchProperty = static_cast<PropertyId>(-1);

    //For property strings
    struct AttributeString {
      AttributeString(size_t size, const iknow::base::Char* data) : size(size), data(data) {}
      size_t size;
      const iknow::base::Char* data;
    };

    //Special labels
    enum SpecialLabel {
      ConceptLabel,
      RelationLabel,
      UnknownLabel,
      PunctuationLabel,
      SentenceBeginLabel,
      SentenceEndLabel,
      JoinLabel,
	  JoinReverse,	// New, reverse join of the lexreps 
      CapitalInitialLabel,
      CapitalMixedLabel,
      CapitalAllLabel,
      SubjectLabel,
      ObjectLabel,
      PrimaryRelationLabel,
      NonSemanticLabel,
	  NumericLabel, // Japanese only (at the moment), marks numeric substrings
	  AlphaBeticLabel, // Japanese only, marks alphabetic symbols
	  SpaceLabel, // Japanese only, marks space symbols
	  KatakanaLabel, // Japanese only, marks katakana symbols
      CertaintyLabel, // Certainty label that influences the certainty level
      EndLabels, //Special one-past-the-end marker for iterator-like semantics
      BeginLabels = ConceptLabel //Ditto: usual begin marker
    };

    class CORE_API IkKnowledgebase
    {
    public:
      IkKnowledgebase();/*!< Constructor. */
      virtual ~IkKnowledgebase();/*!< Destructor. */

    public:

      //Replaces the old user dictionary "doSubstitute"
      virtual void FilterInput(iknow::base::String& /* input */) const {
	/* default does nothing */
      };

      virtual FastLabelSet::Index GetLabelIndex(SpecialLabel label) const = 0;
      
      //Currently needed only for tracing. Avoid constructing IkLabels in performance-critical
      //code.
      virtual IkLabel GetAtIndex(FastLabelSet::Index index) const = 0;

      virtual IkLabel::Type GetLabelTypeAtIndex(FastLabelSet::Index index) const = 0;

      //Rules are sorted by phase then precedence
      virtual size_t RuleCount() const = 0;

      virtual IkRule GetRule(size_t rule_id) const = 0;
      
      //Label attribute interface: Labels can have associated attributes, which are
      //strings given ID values
      //Default implementation does not support attributes
      virtual AttributeId AttributeIdForName(const iknow::base::String& /* name */) const { return 0; }
      virtual AttributeString AttributeNameForId(AttributeId /* id */) const { return AttributeString(0,0); }
      
      virtual size_t GetAttributeCount(FastLabelSet::Index /* index */) const { return 0; }
      virtual AttributeId GetAttributeType(FastLabelSet::Index /* index */, size_t /* position */) const { return 0; }

      virtual const AttributeId* GetAttributeParamsBegin(FastLabelSet::Index /* index */, size_t /* position */) const { return 0; }

      virtual const AttributeId* GetAttributeParamsEnd(FastLabelSet::Index /* index */, size_t /* position */) const { return 0; }

      LabelAttributeIterator LabelAttributesBegin(FastLabelSet::Index index) const {
	return LabelAttributeIterator(LabelAttribute(index, 0));
      }

      LabelAttributeIterator LabelAttributesEnd(FastLabelSet::Index index) const {
	return LabelAttributeIterator(LabelAttribute(index, GetAttributeCount(index)));
      }

      //Returns the phases to which the label applies
      virtual const Phase* GetPhasesBegin(FastLabelSet::Index /* index */) const { return 0; }
      virtual const Phase* GetPhasesEnd(FastLabelSet::Index /* index */) const { return 0; }

      virtual PropertyId PropertyIdForName(const iknow::base::String& /* name */) const { return 0; }
      virtual PropertyId PropertyIdForName(const AttributeString& /* attr */) const { return 0; }

      virtual void FilterPreprocess(iknow::base::String& str) const = 0;
      //TODO: TRW, Consolidate these
      virtual void FilterConcept(iknow::base::String& str, bool first = 0, bool last = 0) const = 0;
      virtual void FilterRelation(iknow::base::String& str, bool first = 0, bool last = 0) const = 0;
      virtual void FilterNonRelevant(iknow::base::String& str, bool first = 0, bool last = 0) const = 0;
	  virtual void FilterPathRelevant(iknow::base::String& str, bool first = 0, bool last = 0) const = 0;
      
      //returns the next lexrep by matching "primitive" lexreps agains the dictionary, adjusting current
      //iterator as it goes
      virtual IkLexrep NextLexrep(Lexreps::iterator& current, Lexreps::iterator end) const = 0;
      //NextLexrep will occasionally discover more than one lexrep for each match. The knowing that
      //current=end after NextLexrep is not enough to conclude that there are no further lexreps to match.
      virtual bool MoreLexrepsBuffered() const { return false; }
	  virtual void ResetLexrepsBuffer() const {} // need to reset the buffer for reentrancy

      //For labeling tokens via the user dictionary
	  virtual bool LabelSingleToken(IkLexrep&) const { return false; }
	  virtual bool LabelSingleToken(IkLexrep&, const iknow::base::String&) const { return false;  }

      virtual short       IsNonSentenceSeparator(const iknow::base::String &strLabel ) const = 0; /*!< checks if the char is a non sentence separator */
      virtual short       IsSentenceSeparator   (const iknow::base::String &strLabel ) const = 0; /*!< checks if the string is a sentence separator */
      
      bool FastIsSentenceSeparator(iknow::base::Char c) const { //Most language models will be happy with this. Avoid the virtual call for now.
	    switch (c) {
	    case '.':
	    case ';':
	    case '!':
	    case '?':
	        return true;
	    default:
	        return false;
	    }
      } 
      virtual short       IsSentenceSeparator   (const iknow::base::Char &cCharacter      ) const = 0; /*!< checks if the char is linked to a sentenceseparator  */
      virtual short       IsWordSeparator       (const iknow::base::Char &cCharacter      ) const = 0; /*!< checks if the char is a wordseparator  */
      virtual short       IsConceptSeparator    (const iknow::base::Char &cCharacter      ) const = 0; /*!< checks if the char is a conceptseparator*/

      virtual const iknow::base::String GetMetadataString(const std::string& /* name */) const { return iknow::base::String(); }

      const IkMetadataCache* GetMetadataCache() const { if (!cache_) cache_ = new IkMetadataCache(*this); return cache_; }

      template<MetadataValue V>
	    typename MetadataTraits<V>::value_type GetMetadata() const {
	    const IkMetadataCache* cache = GetMetadataCache();
	    return cache->GetValue<V>();
      }

      //Returns a hash value for identifying this knowledgebase
      //Note that on a 32-bit system we'd need around 70,000 different knowledgebases
      //to get a 1:2 chance of a collision.
      virtual size_t GetHash() const { return 0; }
      virtual size_t longest_lexrep_length() const = 0; // returns the longest lexrep to lookup.

    private:
      mutable IkMetadataCache* cache_;
    };

  }
}

#endif

