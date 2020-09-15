#ifndef IKNOW_SHELL_SHAREDMEMORYKNOWLEDGEBASE_H_
#define IKNOW_SHELL_SHAREDMEMORYKNOWLEDGEBASE_H_
#define NO_UCHAR_TYPE //Don't want sysCommon's "String" type
//Next line ***must*** always be outcommented in production !
//#define INCLUDE_GENERATE_IMAGE_CODE //Generate rawblocs for UIMA standalone annotator.
#include "IkKnowledgebase.h"
#include "RawBlock.h"
#include "StaticHashTable.h"
#include "IkTypes.h"
#include "Export.h"
#include "OffsetPtr.h"
#include "utlExceptionFrom.h"
#include "KbRegex.h"
#include "KbLabel.h"
#include "KbLexrep.h"
#include "KbProperty.h"
#include "KbRule.h"
#include "KbAcronym.h"
#include "KbPreprocessFilter.h"
#include "KbFilter.h"
#include "KbInputFilter.h"
#include "KbAttributeMap.h"
#include "KbMetadata.h"
#ifdef ISC_IRIS
#include "utlCacheList.h"
#else
typedef std::list<std::string> CacheList;
#endif
#include <queue>
#include <sstream>

#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Metadata.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Acronym.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Regex.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Filter.h"
// #include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Label.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Lexrep.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_PreprocessFilter.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Rule.h"


namespace iknow {
  namespace shell {
      class CSV_DataGenerator; // forward declaration
      class iKnow_KB_Label
      {
      public:
          std::string Name; // Name As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
          std::string Type; // Type As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
          std::string Attributes; // Attributes As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE");
          std::string PhaseList; // Property PhaseList As %String;

      private:
          static void LoadSpecialLabels(CSV_DataGenerator& kb);
          static iKnow_KB_Label LabelFromString(std::vector<std::string>& row_label, std::string& isDefault);
      };

    class AbstractKnowledgebase {
    public:
      typedef size_t Key;

      virtual bool IsCompiled() = 0;

      virtual std::string GetName() = 0;
      virtual iknow::base::String GetHash() = 0;

      virtual size_t AcronymCount() = 0;
      virtual Key NextAcronym(Key key = 0) = 0;
      virtual CacheList GetAcronym(Key key) = 0;

      virtual size_t RegexCount() = 0;
      virtual Key NextRegex(Key key = 0) = 0;
      virtual CacheList GetRegex(Key key) = 0;

      virtual size_t LabelCount() = 0;
      virtual Key NextLabel(Key key = 0) = 0;
      virtual CacheList GetLabel(Key key) = 0;

      virtual CacheList GetSpecialLabel(iknow::core::SpecialLabel label) = 0;

      virtual size_t LexrepCount() = 0;
      virtual Key NextLexrep(Key key = 0) = 0;
      virtual CacheList GetLexrep(Key key) = 0;

      virtual size_t RuleCount() = 0;
      virtual Key NextRule(Key key = 0) = 0;
      virtual CacheList GetRule(Key key) = 0;

      virtual size_t PreprocessFilterCount() = 0;
      virtual Key NextPreprocessFilter(Key key = 0) = 0;
      virtual CacheList GetPreprocessFilter(Key key) = 0;

      virtual size_t ConceptFilterCount() = 0;
      virtual Key NextConceptFilter(Key key = 0) = 0;
      virtual CacheList GetConceptFilter(Key key) = 0;

      virtual size_t InputFilterCount() = 0;
      virtual Key NextInputFilter(Key key = 0) = 0;
      virtual CacheList GetInputFilter(Key key) = 0;

      virtual size_t PropertyCount() = 0;
      virtual Key NextProperty(Key key = 0) = 0;
      virtual CacheList GetProperty(Key key) = 0;

      virtual size_t MetadataCount() = 0;
      virtual Key NextMetadata(Key key = 0) = 0;
      virtual CacheList GetMetadata(Key key) = 0;

      //These will have default dummy implementations for KBs that don't support
      //dynamic loading.
      virtual bool IsDynamic()  { return false; }
      virtual size_t PhaseCount() { return 0; }
      virtual void ReadTable(const std::string&, size_t) { }
      virtual bool TableAtEnd() { return true; }
      virtual CacheList FetchRows(size_t) { return CacheList(); }
      virtual void CloseTable() { }

      std::vector<iknow::shell::iKnow_KB_Label> kb_labels;
    };

    struct RawKBData {
      OffsetPtr<const KbLabel> labels_begin;
      OffsetPtr<const KbLabel> labels_end;
      //Table from label name to offset from labels_begin
      typedef StaticHash::Table<iknow::base::String, size_t> LabelsTable;
      OffsetPtr<const LabelsTable> labels;
      //Array of "special" label indexes
      size_t special_labels[iknow::core::EndLabels];
      typedef StaticHash::Table<iknow::base::String, KbLexrep> LexrepsTable;
      OffsetPtr<const LexrepsTable> lexreps;
      typedef StaticHash::Table<iknow::base::String, KbProperty> PropertyTable;
      OffsetPtr<const PropertyTable> properties;
      OffsetPtr<const KbRegex> regexes_begin;
      OffsetPtr<const KbRegex> regexes_end;
      OffsetPtr<const KbRule> rules_begin;
      OffsetPtr<const KbRule> rules_end;
      typedef StaticHash::Table<iknow::base::String, KbAcronym> AcronymsTable;
      OffsetPtr<const AcronymsTable> acronyms;
      OffsetPtr<const KbPreprocessFilter> preprocess_filters_begin;
      OffsetPtr<const KbPreprocessFilter> preprocess_filters_end;
      OffsetPtr<const KbFilter> concept_filters_begin;
      OffsetPtr<const KbFilter> concept_filters_end;
      OffsetPtr<const KbInputFilter> input_filters_begin;
      OffsetPtr<const KbInputFilter> input_filters_end;
      OffsetPtr<const KbAttributeMap> attribute_map;
      typedef StaticHash::Table<std::string, KbMetadata> MetadataTable;
      OffsetPtr<const MetadataTable> metadata;
      size_t max_lexrep_size;
      size_t max_token_size; // JDN: for Asian languages (not whitespace separated) we need to know the maximum length of tokens
      bool IsAsian; // JDN: indicates Asian language.
      size_t hash;
    };

#define OFFSETPTRGUARD BasePointerFrame frame_guard(RawData())

    class SHELL_API SharedMemoryKnowledgebase : public iknow::core::IkKnowledgebase {
    public:
      //for building from an abstract representation
      SharedMemoryKnowledgebase(RawAllocator& allocator, AbstractKnowledgebase& kb, bool is_compiled = false);
      //for reading from a pointer to a table created by the above constructor
      SharedMemoryKnowledgebase(RawKBData* kb_data);
      //does the cast to a RawKBData
      SharedMemoryKnowledgebase(unsigned char* kb_data);

      unsigned char* RawData() const { return reinterpret_cast<unsigned char*>(kb_data_); }

      void FilterInput(iknow::base::String& input) const;
      
      iknow::core::FastLabelSet::Index GetLabelIndex(iknow::core::SpecialLabel label) const {
	return static_cast<iknow::core::FastLabelSet::Index>(kb_data_->special_labels[label]);
      }

      iknow::core::IkLabel GetAtIndex(iknow::core::FastLabelSet::Index index) const {
	OFFSETPTRGUARD;
	const KbLabel* label = GetLabelAt(index);
	return iknow::core::IkLabel(label->Name(), label->Type());
      }

      iknow::core::IkLabel::Type GetLabelTypeAtIndex(iknow::core::FastLabelSet::Index index) const {
	OFFSETPTRGUARD;
	return GetLabelAt(index)->Type();
      }

      size_t RuleCount() const {
	OFFSETPTRGUARD;
        return kb_data_->rules_end - kb_data_->rules_begin;
      }

      iknow::core::IkRule GetRule(size_t rule_id) const {
	OFFSETPTRGUARD;
	const KbRule& rule = kb_data_->rules_begin[rule_id];
	return iknow::core::IkRule(rule.InputPatternBegin(), rule.InputPatternEnd(), rule.OutputPatternBegin(), rule.OutputPatternEnd(), rule.GetPhase());
      }

      iknow::core::AttributeId AttributeIdForName(const iknow::base::String& name) const {
	OFFSETPTRGUARD;
	const iknow::core::AttributeId* attr = kb_data_->attribute_map->GetId(name);
	return attr ? *attr : iknow::core::kNoSuchAttribute;
      }

      iknow::core::AttributeString AttributeNameForId(iknow::core::AttributeId id) const {
	OFFSETPTRGUARD;
	const StoredAttributeValue* value = kb_data_->attribute_map->GetValue(id);
	return iknow::core::AttributeString(value->size, value->data());
      }

      size_t GetAttributeCount(iknow::core::FastLabelSet::Index index) const {
	OFFSETPTRGUARD;
	return GetLabelAt(index)->AttributeCount();
      }

      iknow::core::AttributeId GetAttributeType(iknow::core::FastLabelSet::Index index, size_t position) const {
	OFFSETPTRGUARD;
	return *GetLabelAt(index)->GetAttribute(position)->GetType();
      }
      
      const iknow::core::AttributeId* GetAttributeParamsBegin(iknow::core::FastLabelSet::Index index, size_t position) const {
	OFFSETPTRGUARD;
	return GetLabelAt(index)->GetAttribute(position)->GetParametersBegin();
      }
      
      const iknow::core::AttributeId* GetAttributeParamsEnd(iknow::core::FastLabelSet::Index index, size_t position) const {
	OFFSETPTRGUARD;
	return GetLabelAt(index)->GetAttribute(position)->GetParametersEnd();
      }

      const iknow::core::Phase* GetPhasesBegin(iknow::core::FastLabelSet::Index index) const {
	OFFSETPTRGUARD;
	return GetLabelAt(index)->GetPhasesBegin();
      }

      const iknow::core::Phase* GetPhasesEnd(iknow::core::FastLabelSet::Index index) const {
	OFFSETPTRGUARD;
	return GetLabelAt(index)->GetPhasesEnd();
      }

      iknow::core::PropertyId PropertyIdForName(const iknow::base::String& name) const {
	OFFSETPTRGUARD;
	const KbProperty* prop = kb_data_->properties->Lookup(name.data(), name.data() + name.size());
	if (prop) return prop->Id();
	return iknow::core::kNoSuchProperty;
      }

      iknow::core::PropertyId PropertyIdForName(const iknow::core::AttributeString& name) const {
	OFFSETPTRGUARD;
	const KbProperty* prop = kb_data_->properties->Lookup(name.data, name.data + name.size);
	if (prop) return prop->Id();
	return iknow::core::kNoSuchProperty;
      }

      void FilterPreprocess(iknow::base::String& str) const {
	    OFFSETPTRGUARD;
	    for (const KbPreprocessFilter* i = kb_data_->preprocess_filters_begin; i != kb_data_->preprocess_filters_end; ++i) {
	      i->Apply(str);
	    }
		iknow::base::IkStringAlg::Trim(str); // remove leading & trailing spaces, if any.
      }
      
      void FilterConcept(iknow::base::String& str, bool first, bool last) const {
	    OFFSETPTRGUARD;
	    for (const KbFilter* i = kb_data_->concept_filters_begin; i != kb_data_->concept_filters_end; ++i) {
	      i->Apply(str, iknow::shell::KbFilter::_concept, first, last); // select concept filters
	    }
      }
      void FilterRelation(iknow::base::String& str, bool first, bool last) const {
	    OFFSETPTRGUARD;
	    for (const KbFilter* i = kb_data_->concept_filters_begin; i != kb_data_->concept_filters_end; ++i) {
	      i->Apply(str, iknow::shell::KbFilter::_relation, first, last); // select relation filters
	    }
      }
      void FilterNonRelevant(iknow::base::String& str, bool first, bool last) const {
	    OFFSETPTRGUARD;
	    for (const KbFilter* i = kb_data_->concept_filters_begin; i != kb_data_->concept_filters_end; ++i) {
	      i->Apply(str, iknow::shell::KbFilter::_nonrelevant, first, last); // select nonrelevant filters
	    }
      }
	  void FilterPathRelevant(iknow::base::String& str, bool first, bool last) const {
		  OFFSETPTRGUARD;
		  for (const KbFilter* i = kb_data_->concept_filters_begin; i != kb_data_->concept_filters_end; ++i) {
			  i->Apply(str, iknow::shell::KbFilter::_pathrelevant, first, last); // select pathrelevant filters
		  }
	  }

      iknow::core::IkLexrep NextLexrep(iknow::core::Lexreps::iterator& current, iknow::core::Lexreps::iterator end) const;

      bool MoreLexrepsBuffered() const { return !buffered_lexreps_.empty(); }
	  void ResetLexrepsBuffer() const {  // clear the buffer for reentrancy safety after error throw
		  // LexrepQueue().swap(buffered_lexreps_); // does not compile on usparc (not c11)
		  while (!buffered_lexreps_.empty()) buffered_lexreps_.pop(); 
	  }

      bool LabelSingleToken(iknow::core::IkLexrep& lexrep) const;
	  bool LabelSingleToken(iknow::core::IkLexrep& lexrep, const iknow::base::String& label_name) const;

      short IsNonSentenceSeparator(const iknow::base::String &strLabel) const {
	OFFSETPTRGUARD;
	const KbAcronym* acronym = kb_data_->acronyms->Lookup(strLabel);
	if (!acronym) return false;
	return !acronym->IsSentenceEnd();
      }
      short IsSentenceSeparator(const iknow::base::String &strLabel) const {
	OFFSETPTRGUARD;
	const KbAcronym* acronym = kb_data_->acronyms->Lookup(strLabel);
	if (!acronym) return false;
	return acronym->IsSentenceEnd();
      }
      
      short IsSentenceSeparator(const iknow::base::Char &cCharacter) const {
	//OFFSETPTRGUARD;
	static const iknow::base::Char separators[] = {'.',';','!','?','\0'}; 
	static const iknow::base::String strSeparators(separators);
	return (strSeparators.find_first_of(cCharacter, 0) == iknow::base::String::npos) ? 0 : 1;
      }

      short IsWordSeparator(const iknow::base::Char &cCharacter) const {
	//OFFSETPTRGUARD;
	static const iknow::base::Char separators[] = {'\n','\t','\r','\0'};
	static const iknow::base::String strSeparators(separators);
	return (strSeparators.find_first_of(cCharacter, 0) == iknow::base::String::npos) ? 0 : 1;
      }

      short IsConceptSeparator(const iknow::base::Char &cCharacter) const {
	//OFFSETPTRGUARD;
	static const iknow::base::Char separators[] = {',','(',')','[',']','{','}',':','"','\0'};
	static const iknow::base::String strSeparators(separators);
	return (strSeparators.find_first_of(cCharacter, 0) == std::string::npos) ? 0 : 1;
      }

      const iknow::base::String GetMetadataString(const std::string& name) const {
	OFFSETPTRGUARD;
	const KbMetadata* metadata = kb_data_->metadata->Lookup(name);
	if (!metadata) return iknow::base::String();
	return metadata->Val();
      }

      size_t longest_lexrep_length() const {
        OFFSETPTRGUARD;
        return kb_data_->max_lexrep_size;
      }
      size_t longest_token_length() const {
        OFFSETPTRGUARD;
        return kb_data_->max_token_size;
      }
      size_t GetHash() const {
        OFFSETPTRGUARD;
        return kb_data_->hash;
      }

    protected:
      typedef std::queue<iknow::core::IkLexrep> LexrepQueue;
      //Sometimes we generate multiple lexreps while doing a NextLexrep().
      //Save the extras here to return on the next call. Stored in reverse order (next is back())
      mutable LexrepQueue buffered_lexreps_;
      RawKBData* kb_data_;

    private:
      const KbLabel* GetLabelAt(iknow::core::FastLabelSet::Index index) const {
        if (kb_data_->labels_begin + index >= kb_data_->labels_end) {
          std::ostringstream strBuffer;
          strBuffer << static_cast<int>(index);
          throw ExceptionFrom<SharedMemoryKnowledgebase>(std::string("SharedMemoryKnowledgebase::GetLabelAt Unknown label index:")+strBuffer.str());
        }
        return kb_data_->labels_begin + index;
      }
    };
  }
}

#endif //IKNOW_SHELL_SHAREDMEMORYKNOWLEDGEBASE_H_

