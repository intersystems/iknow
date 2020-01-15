//The silly string return API uses wcsncpy.
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "ali.h"
#include "LanguageBase.h"
#include <map>
#include "IkStringEncoding.h"

using iknow::base::IkStringEncoding;
using iknow::base::String;
using iknow::base::IkTrace;

using namespace iknow::ali;

namespace iknow {
  namespace ali {
    LanguagebaseMap& Languagebases() {
      static LanguagebaseMap map;
      return map;
    }

    //static const size_t minSubstringLength = 4;
    //static const size_t maxSubstringLength = 4;

    String identify(const String& text, double & certainty)
    {
      return identify(text.begin(), text.end(), certainty);
    }

    String identify(const String::const_iterator start, const String::const_iterator end, double& certainty) {
      //Any languages available?
      if (Languagebases().empty()) {
	certainty = 0;
	return String();
      }
      //Just one?
      if (Languagebases().size() == 1) {
	certainty = 1;
	return Languagebases().begin()->first;
      }

      certainty = 0;

      //For empty text, just return a consistent, arbitrary languge base (the first)
      //with 0 certainty.
      if (start == end) {
	return Languagebases().begin()->first;
      }
      double maxScore = 0;
      LanguageBase* languageBase = NULL;
      String language;
      for (LanguagebaseMap::const_iterator i = Languagebases().begin(); i != Languagebases().end(); ++i) {
	//TODO: TRW, Would a method that compared the same string against *all* languages be faster?
	//So we only generate the clusters from the string once?
	double currentScore = i->second->GetSimilarity(&(*start), end - start);
	currentScore*=currentScore; // working with square of scores
	certainty += currentScore;
	if (currentScore > maxScore) {
	  maxScore = currentScore;
	  languageBase = i->second;
	  language = i->first;
	}
      }
      if (certainty)
	certainty = (maxScore / certainty + maxScore / ( maxScore + (certainty-maxScore)/(Languagebases().size()-1)))/2;

      return languageBase ? language: String();

    }
    
    static IkTrace<TraceListType>* global_trace = 0;
    
    void SetTrace(IkTrace<TraceListType>& trace) {
      global_trace = &trace;
    }

    void ClearTrace() {
      global_trace = 0;
    }

    
  }
}
