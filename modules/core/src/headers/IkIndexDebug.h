#ifndef IKNOW_CORE_IKINDEXDEBUG_H_
#define IKNOW_CORE_IKINDEXDEBUG_H_
#include "IkStringOutput.h"
#include "IkLexrep.h"
#include "IkExportCore.h"
#include "IkTrace.h"
#include "IkKnowledgebase.h"
#include "IkSentence.h"
#include "IkPath.h"


#include <map>
namespace iknow {
  namespace core {
    template<typename T>
    class CORE_API IkIndexDebug {};

    template<>
    class CORE_API IkIndexDebug<Utf8List> {
    public:
      void SwitchKnowledgebase(const iknow::base::String& old_language, const iknow::base::String& new_language, double certainty);
      void ApplyRule(size_t rule_id, Lexreps::iterator match, size_t match_length, const IkKnowledgebase& kb);
      void RuleApplied(size_t rule_id, Lexreps::iterator match, size_t match_length, const IkKnowledgebase& kb);
      void RulesComplete(const IkLexrep& lexrep, const IkKnowledgebase& kb);
      void AmbiguityResolved(const IkLexrep& lexrep, const IkKnowledgebase& kb);
      void PreprocessToken(const iknow::base::String& original, const iknow::base::String& replacement);
      void NormalizeToken(const iknow::base::String& original, const iknow::base::String& replacement);
      void LexrepCreated(const IkLexrep& lexrep, const IkKnowledgebase& kb);
      void LexrepIdentified(const IkLexrep& lexrep, const IkKnowledgebase& kb);
	  void UserDictionaryMatch(const IkLexrep& lexrep, const IkKnowledgebase& kb);
      void AttributeDetected(const std::string& attribute, const IkLexrep& lexrep, const IkKnowledgebase& kb);
      void LexrepTypeAssignment(const IkLexrep& lexrep, const IkKnowledgebase& kb);
	  void SentenceFound(const iknow::base::String& kb_name, double certainty, const iknow::base::String& language_code, const Lexreps& lexreps, const iknow::base::String& separator = iknow::base::SpaceString());
      void MergingRelations(Lexreps::const_iterator begin, Lexreps::const_iterator end, const IkKnowledgebase& kb);
      void MergingRelation(const IkLexrep& lexrep, const IkKnowledgebase& kb);
      void MergedRelation(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb);
      void MergedRelationNonrelevant(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb);
      void MergingConcepts(Lexreps::const_iterator begin, Lexreps::const_iterator end, const IkKnowledgebase& kb);
      void MergingConcept(const IkLexrep& lexrep, const IkKnowledgebase& kb);
	  void MergedKatakana(const IkLexrep& lexrep, const IkKnowledgebase& kb);
	  void LabelKatakana(const IkLexrep& lexrep, const IkKnowledgebase& kb);
      void MergedConcept(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb);
      void ConceptFiltered(const IkLexrep& lexrep, const iknow::base::String& replacement, const IkKnowledgebase& kb);
      void RelationFiltered(const IkLexrep& lexrep, const iknow::base::String& replacement, const IkKnowledgebase& kb);
	  void PathRelevantFiltered(const IkLexrep& lexrep, const iknow::base::String& replacement, const IkKnowledgebase& kb);
      void NonRelevantFiltered(const IkLexrep& lexrep, const iknow::base::String& replacement, const IkKnowledgebase& kb);
      void SentenceComplete(const IkSentence& sentence, const IkKnowledgebase& kb);
      void SentenceSummarizationComplete(const IkSentence& sentence);
      void CRC(const path::CRC& crc, const MergedLexreps& lexreps);
      void EntityVectors(const IkSentence&);
	  void InvalidEntityVector(const IkLexrep& lexrep, const IkKnowledgebase& kb);
	  void MissingEntityVector(const IkMergedLexrep& lexrep, const IkKnowledgebase& kb);
      void StemOccurence(const iknow::base::String& occurence, const iknow::base::String& stem);
	  void JoinResult(const Lexreps::iterator lexrep, const IkKnowledgebase& kb);
	  void SingleWordFreq(const iknow::base::String& word, size_t frequency);
	  void TraceKeyDouble(const char* type, const iknow::base::String& name, double value);
	  void Parameter(const iknow::base::String& name, double value);

	  void StartTimer(void);
	  void TraceTheTime(const int action);

      const iknow::base::IkTrace<Utf8List>& GetTrace() const ;

    private:
      iknow::base::IkTrace<Utf8List> trace_;

    };
  }
}

#endif //IKNOW_CORE_IKINDEXDEBUG_H_
