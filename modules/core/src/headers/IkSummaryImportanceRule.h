// IkSummaryImportanceRule.h: interface for the IkSummaryImportanceRule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKSUMMARYIMPORTANCERULE_H__0CED3940_6BCF_4974_BCBD_690FF2074C8B__INCLUDED_)
#define AFX_IKSUMMARYIMPORTANCERULE_H__0CED3940_6BCF_4974_BCBD_690FF2074C8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IkExportCore.h"
#include "IkSentence.h"
#include "IkTypes.h"
#include "SafeString.h"

namespace iknow
{
  namespace core 
  {
    class CORE_API IkSummaryImportanceRule
    {
    public:
      IkSummaryImportanceRule(SENTENCE_IMPORTANCE importance) : m_importance(importance) { }
      virtual ~IkSummaryImportanceRule() {}
      bool tryToExecute(IkSentence& sentence, int reverseOrder) {
	    bool match = isMatch(sentence, reverseOrder);
	    if (match) 
          setSentenceImportance(sentence);
	    return match;
      }
      virtual int  getPriority() const  = 0;

      static bool CompareImportanceRules       (IkSummaryImportanceRule* ir1, IkSummaryImportanceRule* ir2);
      static bool CompareImportanceRulesForced (IkSummaryImportanceRule* ir1, IkSummaryImportanceRule* ir2);
    protected:
      virtual bool isMatch(IkSentence& sentence, int reverseOrder)  = 0;

    private:				
      void setSentenceImportance(IkSentence& sentence) { sentence.sentence_importance_ = m_importance; }
      SENTENCE_IMPORTANCE  m_importance;
    };

    class CORE_API IkSummaryImportanceRuleText : public IkSummaryImportanceRule
    {
    public:
      IkSummaryImportanceRuleText(SENTENCE_IMPORTANCE importance, const iknow::base::String& text, bool wholeWord);
      virtual int  getPriority() const  { return m_wholeWord?2:1; } // the value itself is not important, but the value compared to the other values

    protected:
      virtual bool  isMatch(IkSentence  & sentence, int reverseOrder) ;

    private:
      iknow::base::String  m_text;
      bool    m_wholeWord;
    };

    class CORE_API IkSummaryImportanceRuleSentenceOrder: public IkSummaryImportanceRule
    {
    public:
      IkSummaryImportanceRuleSentenceOrder(SENTENCE_IMPORTANCE importance, int sentenceOrder);
      virtual int  getPriority() const  { return 3; } // the value itself is not important, but the value compared to the other values

    protected:
      virtual bool  isMatch(IkSentence& sentence, int reverseOrder) ;

    private:
      int  m_sentenceOrder;
    };
  }
}

#endif // !defined(AFX_IKSUMMARYIMPORTANCERULE_H__0CED3940_6BCF_4974_BCBD_690FF2074C8B__INCLUDED_)
