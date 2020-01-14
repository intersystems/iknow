// IkSummaryImportanceRule.cpp: implementation of the IkSummaryImportanceRule class.
//
//////////////////////////////////////////////////////////////////////

#include "IkSummaryImportanceRule.h"
#include "IkSentence.h"

using namespace iknow::core;
using iknow::base::String;
using iknow::base::SpaceString;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IkSummaryImportanceRuleText::IkSummaryImportanceRuleText(SENTENCE_IMPORTANCE importance, const String& text, bool  wholeWord)
  : IkSummaryImportanceRule(importance), m_text(wholeWord?(SpaceString() + text +SpaceString()):text), m_wholeWord(wholeWord) { }

bool  IkSummaryImportanceRuleText::isMatch(IkSentence& sentence, int) 
{
  // TODO more efficient checking ? (lots of string copies required)
  return (m_wholeWord?(SpaceString() + sentence.GetNormalizedText() + SpaceString()):sentence.GetNormalizedText()).find(m_text) != String::npos ||
    (m_wholeWord?(SpaceString() + sentence.GetText          () + SpaceString()):sentence.GetText          ()).find(m_text) != String::npos;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IkSummaryImportanceRuleSentenceOrder::IkSummaryImportanceRuleSentenceOrder(SENTENCE_IMPORTANCE importance, int sentenceOrder)
  : IkSummaryImportanceRule(importance), m_sentenceOrder(sentenceOrder) { }

bool  IkSummaryImportanceRuleSentenceOrder::isMatch(IkSentence& sentence, int reverseOrder) 
{
  return sentence.GetSentenceOrder() == m_sentenceOrder || reverseOrder == m_sentenceOrder;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// return true if the ir1 rule should be checked before the ir2 rule
// 1) FORCED         goes before NOT_FORCED
// 2) HIGHER PRIORTY goes before LOWER PRIORITY
// 3) TRUE           goes before FALSE
// 4) NEURTAL
bool  IkSummaryImportanceRule::CompareImportanceRules(IkSummaryImportanceRule* ir1, IkSummaryImportanceRule* ir2)
{
  // 1) FORCED         goes before NOT_FORCED
  if (IkSentence::isForced(ir1->m_importance) != IkSentence::isForced(ir2->m_importance))
    return IkSentence::isForced(ir1->m_importance);

  return CompareImportanceRulesForced(ir1,ir2);
}

// return true if the ir1 rule should be checked before the ir2 rule
// do not take into account the force difference between the rules
// 2) HIGHER PRIORTY goes before LOWER PRIORITY
// 3) TRUE     goes before FALSE
// 4) NEURTAL
bool  IkSummaryImportanceRule::CompareImportanceRulesForced(IkSummaryImportanceRule* ir1, IkSummaryImportanceRule* ir2)
{
  // 2) HIGHER PRIORTY goes before LOWER PRIORITY : Sentence > Word > Text
  if (ir1->getPriority() != ir2->getPriority())
    return ir1->getPriority() > ir2->getPriority();

  // 3) TRUE     goes before FALSE
  return ir2->m_importance == NEUTRAL_SENTENCE || ir1->m_importance > 0;
}
