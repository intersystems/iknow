// IkSummarizer.h: interface for the IkSummarizer class.
//
//////////////////////////////////////////////////////////////////////
#ifndef IKNOW_CORE_IKSUMMARIZER_H_
#define IKNOW_CORE_IKSUMMARIZER_H_

#include "IkExportCore.h"
#include "IkObject.h"
#include "IkIndexOutput.h"
#include "IkIndexDebug.h"
#include "IkSummaryLengthMode.h"
#include "IkSummaryImportanceRule.h"
#include "IkSentence.h"
#include "IkTypes.h"
#include "IkMergedLexrep.h"

#include "IkPath.h"
#include "SafeString.h"
#include <map>

#ifdef _WIN32
#pragma warning(disable:4251)
#endif

namespace iknow
{
  namespace core
  {
    class CORE_API IkSummarizer
    {
			
    public:
      static IkSummarizer* GetInstance() { return (!inst ? inst = new IkSummarizer() : inst); }
      void setSummaryWeight(const iknow::base::String& weights); // Influence the natural order of things...
      void CalculateSummaryRelevance(IkIndexOutput& output, IkIndexDebug* debug) const; // Calculate summary values per sentence

    private:
      static IkSummarizer* inst;
      IkSummarizer() /*: m_forceImportanceRules(false), m_sortImportanceRules(true)*/ {}
      ~IkSummarizer() {
        resetSummaryWeight();
      }
      void resetSummaryWeight() {
        m_scaleFactorsW.clear();
        m_scaleFactorsS.clear();
        std::for_each(m_importanceRules.begin(),m_importanceRules.end(), delete_object());
        m_importanceRules.clear();
      }

      IkSummarizer(const IkSummarizer&); // Prevent copy-construction
      IkSummarizer& operator=(const IkSummarizer&); // Prevent assignment

      typedef map<iknow::base::String, int> ScaleFactorsWords;
      ScaleFactorsWords m_scaleFactorsW;
      typedef map<int, double> ScaleFactorsSentences;
      ScaleFactorsSentences m_scaleFactorsS;
      typedef vector<IkSummaryImportanceRule*> ImportanceRules; 
      ImportanceRules m_importanceRules; // an array of pointers is chosen because sorting is more performant

      void addImportanceRule(SENTENCE_IMPORTANCE sentenceImportance, const iknow::base::String& text, bool wholeWord) {
        m_importanceRules.push_back(new IkSummaryImportanceRuleText(sentenceImportance, text, wholeWord));
      }
      void addImportanceRule(SENTENCE_IMPORTANCE sentenceImportance, int sentenceOrder) {
        m_importanceRules.push_back(new IkSummaryImportanceRuleSentenceOrder(sentenceImportance, sentenceOrder));
      }
      void addScaleFactor(const iknow::base::String& word, int weight ) { m_scaleFactorsW[word] = weight; }
      void addScaleFactor(int sentenceOrder, double weight ) { m_scaleFactorsS[sentenceOrder] = weight; }

      void ObtainWordCounts(Sentences::const_iterator begin, Sentences::const_iterator end, IkIndexOutput::WordCounts& wordCounts) const;
      void AdjustWordCounts(IkIndexOutput::WordCounts& wordCounts) const;
      void ApplyImportanceRules(Sentences::reverse_iterator rbegin, Sentences::reverse_iterator rend) const;
      void AdjustSummaryRelevanceOfSentences(IkIndexOutput& output) const;

      void IncreaseSummaryRelevance(Sentences::iterator begin, Sentences::iterator end, const IkIndexOutput::WordCounts& wordCounts) const;
      void IncreaseSummaryRelevance(IkPath& path, IkSentence& sentence, const IkIndexOutput::WordCounts& wordCounts) const;
      void IncreaseSummaryRelevance(IkMergedLexrep& concept, const IkIndexOutput::WordCounts& wordCounts) const;
    };
  }
}

#endif //IKNOW_CORE_IKSUMMARIZER_H_
