// IkSummarizer.cpp: implementation of the IkSummarizer class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
// Disable stl warnings
#pragma warning (push)
#pragma warning (disable: 4786)
#endif

#include "IkObject.h"
#include "IkSummarizer.h"
#include "IkMergedLexrep.h"
#include "IkSentence.h"
#include "IkLexrep.h"
#include "IkMergedLexrep.h"
#include "IkStringAlg.h"
#include "IkPath.h"
#include <vector>
#include <algorithm>
#include "SafeString.h"
#include "utlExceptionFrom.h"
#include <sstream>
#include <map>

using namespace iknow::core;
using namespace iknow::base;
using namespace std;

IkSummarizer* IkSummarizer::inst = NULL;

struct SummaryArgsBuilder {
  std::vector<String>& argsWeights_;

  SummaryArgsBuilder(std::vector<String>& argsWeights) : argsWeights_(argsWeights) {}
  void operator()(const iknow::base::String& strToken) { argsWeights_.push_back(strToken); }
private:
   void operator=(const SummaryArgsBuilder&);
};

void IkSummarizer::setSummaryWeight(const String& weights)
{
  static const Char percent[] = {'%','\0'};
  static const String SPercent(percent);
  static const Char text[] = {'t','e','x','t','\0'};
  static const String SText(text);
  static const Char max[] = {'m','a','x','\0'};
  static const String SMax(max);
  static const Char strue[] = {'t','r','u','e','\0'};
  static const String STrue(strue);
  static const Char sfalse[] = {'f','a','l','s','e','\0'};
  static const String SFalse(sfalse);
  static const Char fo[] = { 'f','o','\0'};
  static const String SFo(fo);
  static const Char n[] = {'n','\0'};
  static const String SN(n);

    IkSummarizer* summarizer = IkSummarizer::GetInstance();
    summarizer->resetSummaryWeight();

    std::vector<String> argsWeights;
    SummaryArgsBuilder SummaryArgsBuilder(argsWeights);    
    IkStringAlg::Tokenize(weights, '|', SummaryArgsBuilder);

	if (argsWeights.size()%3 != 0) // should be modulo3
		throw MessageExceptionFrom<IkSummarizer>("IKInvalidSummaryWeightsString");

	for (int i=0; i < (int) argsWeights.size(); i++) {
		String scope = argsWeights[i]; IkStringAlg::ToLower(scope); i++; // std::string scope = ((IkString*) wordList->Get(i))->GetValue(); IkStringAlg::ToLower(scope); i++;
		String key   = argsWeights[i]; IkStringAlg::ToLower(key  ); i++; // std::string key   = ((IkString*) wordList->Get(i))->GetValue(); IkStringAlg::ToLower(key  ); i++;
		String value = argsWeights[i]; IkStringAlg::ToLower(value); // std::string value = ((IkString*) wordList->Get(i))->GetValue(); IkStringAlg::ToLower(value);
		IkStringAlg::RemoveCharacters(value,SPercent); // IkStringAlg::RemoveCharacters(value,"%");

        bool const exact_match = (scope[0]=='w');
		if (scope[0]=='w' || scope[0]=='t') // rule for a whole word (group) or for a character sequence
		{          
          if      (value.find(SFo) != String::npos && value.find( STrue) != String::npos) { summarizer->addImportanceRule( FORCED_TRUE_SENTENCE, key, exact_match); }
          else if (value.find(SFo) == String::npos && value.find( STrue) != String::npos) { summarizer->addImportanceRule(        TRUE_SENTENCE, key, exact_match); }
          else if (value.find(SN) != String::npos                                       ) { summarizer->addImportanceRule(     NEUTRAL_SENTENCE, key, exact_match); }
          else if (value.find(SFo) == String::npos && value.find(SFalse) != String::npos) { summarizer->addImportanceRule(       FALSE_SENTENCE, key, exact_match); }
          else if (value.find(SFo) != String::npos && value.find(SFalse) != String::npos) { summarizer->addImportanceRule(FORCED_FALSE_SENTENCE, key, exact_match); }
          else if (key  .find(iknow::base::SpaceString())  == String::npos) // key does not contain a space
            summarizer->addScaleFactor(key, atoi(iknow::base::IkStringEncoding::BaseToUTF8(value).c_str())); // value is a weight
		}
		else if (scope[0]=='s') // rule for a sentence
		{
		  int const order = atoi(iknow::base::IkStringEncoding::BaseToUTF8(key).c_str());

		  if      (value.find(SFo) != String::npos && value.find( STrue) != String::npos) { summarizer->addImportanceRule( FORCED_TRUE_SENTENCE, order); }
		  else if (value.find(SFo) == String::npos && value.find( STrue) != String::npos) { summarizer->addImportanceRule(        TRUE_SENTENCE, order); }
		  else if (value.find(SN) != String::npos                                       ) { summarizer->addImportanceRule(     NEUTRAL_SENTENCE, order); }
		  else if (value.find(SFo) == String::npos && value.find(SFalse) != String::npos) { summarizer->addImportanceRule(       FALSE_SENTENCE, order); }
		  else if (value.find(SFo) != String::npos && value.find(SFalse) != String::npos) { summarizer->addImportanceRule(FORCED_FALSE_SENTENCE, order); }
		  else // value is a weight
			summarizer->addScaleFactor(order, atof(iknow::base::IkStringEncoding::BaseToUTF8(value).c_str())); // TODO: find something simpler to convert to double
		}
		else
	      throw MessageExceptionFrom<IkSummarizer>("IKNoValidScopeForRule");
	}
    std::sort(m_importanceRules.begin(),m_importanceRules.end(), IkSummaryImportanceRule::CompareImportanceRules); // sort the "Importance" rules (true/false) on priority
}

struct WordCountIncrementer {
  IkIndexOutput::WordCounts& word_counts_;
  WordCountIncrementer(IkIndexOutput::WordCounts& word_counts) : word_counts_(word_counts) {}
  void operator()(const iknow::base::Char* begin, const iknow::base::Char* end) {
    word_counts_[WordPtr(begin, end)]++;
  }
private:
  void operator=(const WordCountIncrementer&);
};

void IkSummarizer::ObtainWordCounts(Sentences::const_iterator begin, Sentences::const_iterator end, IkIndexOutput::WordCounts& wordCounts) const
{
  for (Sentences::const_iterator i = begin; i != end; ++i) {
    const IkSentence *currentSentence = &(*i);
    
    WordCountIncrementer incrementer(wordCounts);
    for (MergedLexreps::const_iterator j=currentSentence->GetLexrepsBegin(); j != currentSentence->GetLexrepsEnd(); ++j) {
      if (j->GetLexrepType() == IkLabel::Concept)
      {
	    for (IkMergedLexrep::const_iterator k = j->LexrepsBegin(); k != j->LexrepsEnd(); ++k) {
	      IkStringAlg::Tokenize(k->GetNormalizedValue().data(), k->GetNormalizedValue().data() + k->GetNormalizedValue().size(), static_cast<iknow::base::Char>(' '), incrementer);
	    }
      }
    }
  }
}

void IkSummarizer::AdjustWordCounts(IkIndexOutput::WordCounts& wordCounts) const
{
  if (m_scaleFactorsW.size() < wordCounts.size()) {  // it is a choise between complexity O(n*log(m)) and O(m*log(n))
    for (ScaleFactorsWords::const_iterator cit = m_scaleFactorsW.begin(); cit != m_scaleFactorsW.end(); ++cit) // loop over m_scaleFactorsW
	{
	  IkIndexOutput::WordCounts::iterator wordCountsIt = wordCounts.find(WordPtr(cit->first));
	  if (wordCountsIt != wordCounts.end())
	    wordCountsIt->second *= cit->second;
	}
  } else {
    for (IkIndexOutput::WordCounts::iterator cit = wordCounts.begin(); cit != wordCounts.end(); ++cit) // loop over wordCounts
	{
	  ScaleFactorsWords::const_iterator scaleFactorsIt = m_scaleFactorsW.find((*cit).first);
	  if (scaleFactorsIt != m_scaleFactorsW.end())
	    cit->second *= scaleFactorsIt->second;
	}
  }
}

struct LexrepRelevanceIncreaser {
  IkMergedLexrep& lexrep_;
  const IkIndexOutput::WordCounts& word_counts_; 
  LexrepRelevanceIncreaser(IkMergedLexrep& lexrep, const IkIndexOutput::WordCounts& word_counts) : lexrep_(lexrep), word_counts_(word_counts) {}
  void operator()(const iknow::base::Char* begin, const iknow::base::Char* end) {
    IkIndexOutput::WordCounts::const_iterator i = word_counts_.find(WordPtr(begin, end));
    if (i == word_counts_.end()) throw ExceptionFrom<LexrepRelevanceIncreaser>("Unknown word found during summarization.");
    lexrep_.increaseSummaryRelevance(i->second);
  }
private:
  void operator=(const LexrepRelevanceIncreaser&);
};

void IkSummarizer::IncreaseSummaryRelevance(IkMergedLexrep& concept, const IkIndexOutput::WordCounts& wordCounts) const
{
  LexrepRelevanceIncreaser increaser(concept, wordCounts);
  IkStringAlg::Tokenize(concept.GetNormalizedValue().data(), concept.GetNormalizedValue().data() + concept.GetNormalizedValue().size(), static_cast<iknow::base::Char>(' '), increaser);
}

void IkSummarizer::IncreaseSummaryRelevance(IkPath& path, IkSentence& sentence, const IkIndexOutput::WordCounts& wordCounts) const
{
  MergedLexreps::iterator begin = sentence.GetLexrepsBegin();
  for (path::Offsets::const_iterator i = path.OffsetsBegin(); i != path.OffsetsEnd(); ++i) {
    MergedLexreps::iterator lexrep = begin + *i;
    if (lexrep->GetLexrepType() == IkLabel::Concept) {
      IncreaseSummaryRelevance(*lexrep, wordCounts);
    }
  }
}

void IkSummarizer::IncreaseSummaryRelevance(Sentences::iterator begin, Sentences::iterator end, const IkIndexOutput::WordCounts& wordCounts) const
{
  for (Sentences::iterator i = begin; i != end; ++i) {
    IkSentence *currentSentence = &(*i);
    for (IkSentence::Paths::iterator j = currentSentence->GetPathsBegin(); j != currentSentence->GetPathsEnd(); ++j) {
      IncreaseSummaryRelevance(*j, *currentSentence, wordCounts);
    }
  }
}

void IkSummarizer::ApplyImportanceRules(Sentences::reverse_iterator rbegin, Sentences::reverse_iterator rend) const
{
  int reverseOrder = 0;
  for (Sentences::reverse_iterator i = rbegin; i != rend; ++i) // loop over the sentences in reverse order
  {
      reverseOrder--;
      IkSentence *currentSentence = &(*i);
      for (ImportanceRules::const_iterator it = m_importanceRules.begin(); it != m_importanceRules.end(); ++it)
      {
        IkSummaryImportanceRule* rule = (*it);
        if (rule->tryToExecute(*currentSentence, reverseOrder)) // Only one true/false rule fires per sentence
          break;
      }	
  }
}

void IkSummarizer::AdjustSummaryRelevanceOfSentences(IkIndexOutput& output) const
{
  output.GetSummaryRelevance(); // does a precomputation internally
  int reverseOrder = 0;
  for (Sentences::reverse_iterator i = output.SentencesRBegin(); i != output.SentencesREnd() ; ++i) // loop over the sentences in reverse order
    {
      --reverseOrder;
      IkSentence* currentSentence = &(*i);
      ScaleFactorsSentences::const_iterator scaleFactorsIt = m_scaleFactorsS.find(currentSentence->GetSentenceOrder());
      if (scaleFactorsIt != m_scaleFactorsS.end())
	currentSentence->scaleSummaryRelevance(scaleFactorsIt->second);

      scaleFactorsIt = m_scaleFactorsS.find(reverseOrder);
      if (scaleFactorsIt != m_scaleFactorsS.end())
	currentSentence->scaleSummaryRelevance(scaleFactorsIt->second);
    }
}

void IkSummarizer::CalculateSummaryRelevance(IkIndexOutput& output, IkIndexDebug* debug) const
{
  IkIndexOutput::WordCounts& wordCounts = output.m_wordCounts;
  ObtainWordCounts (output.SentencesBegin(), output.SentencesEnd(), wordCounts);  // gather counts of all words

  AdjustWordCounts(wordCounts);   // take the scaleFactors of individual words into account
  ApplyImportanceRules(output.SentencesRBegin(), output.SentencesREnd());  // mark sentences (forced) true/false
  IncreaseSummaryRelevance(output.SentencesBegin(), output.SentencesEnd(), wordCounts);  // calculate the summaryRelevance of the entities, based on the wordCounts
  AdjustSummaryRelevanceOfSentences(output);  // adjust the summaryRelevance of whole sentences according to the scaleFactors given to sentences

  for (Sentences::iterator i = output.SentencesBegin(); i != output.SentencesEnd(); ++i) {
    IkSentence* sentence = &(*i);
    switch (sentence->GetSentenceImportance()) {
      case FORCED_TRUE_SENTENCE:
      case TRUE_SENTENCE:
        sentence->trueSummary(); // negate summary value to mark it's importance
        break;
      case FORCED_FALSE_SENTENCE:
      case FALSE_SENTENCE:
        sentence->dropSummary(); // drop sentence from summary, set summaryvalue=0
        break;
      default: // leave it
        break;
    } 
  }
  if (debug) {
    for (Sentences::const_iterator i = output.SentencesBegin(); i != output.SentencesEnd(); ++i) {
      const IkSentence* sentence = &(*i);
      debug->SentenceSummarizationComplete(*sentence);
    }
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif
