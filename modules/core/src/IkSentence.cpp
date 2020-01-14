// IkSentence.cpp: implementation of the IkSentence class.
//
//////////////////////////////////////////////////////////////////////

#include "IkSentence.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace iknow::base;
using namespace iknow::core;

// sum of all summary relevances of lexreps
double IkSentence::computeSummaryRelevanceCore() const
{
  double summaryRelevance = 1; // every sentence should have some importance.
  for (MergedLexreps::const_iterator i = GetLexrepsBegin(); i != GetLexrepsEnd(); ++i) {
    summaryRelevance += i->GetSummaryRelevance();
  }
  return (summaryRelevance > 0)?summaryRelevance:1;
}

// TODO; should only be computed once ?
// JDN: the assert & first GetText were missing compared to the original sources, might be GCC problems ???
// Or did they resulted in a crash (empty sentence) ?
// If the latter, this should be solved in a different way.
String IkSentence::GetText() const
{
  String text;
  text.reserve(LexrepsSize() * 8);
  for (MergedLexreps::const_iterator i = GetLexrepsBegin(); i != GetLexrepsEnd(); ++i) {
    text += SpaceString() + i->GetText();
  }
  return text;
}

// TODO; should only be computed once ?
String IkSentence::GetNormalizedText() const
{
  String text;
  text.reserve(LexrepsSize() * 8);
  for (MergedLexreps::const_iterator i = GetLexrepsBegin(); i != GetLexrepsEnd(); ++i) {
    text += SpaceString() + i->GetNormalizedText();
  }
  return text;
}
