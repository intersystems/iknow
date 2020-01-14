#include "aliExports.h"
#include "IkTypes.h"
#include "LanguageBase.h"
#include "IkTrace.h"
#include <map>

/**
*	text:
*		The text to be identified.
*	returns:
*		The length of the language string, invoke getString to get the string itself.
*		Since the language string is according to iso 639-3, the length should be 3;
*/
namespace iknow {
  namespace ali {
    ALI_API iknow::base::String identify(const iknow::base::String& text, double& certainty);

    ALI_API iknow::base::String identify(const iknow::base::String::const_iterator start, iknow::base::String::const_iterator end, double& certainty);

    //Set the trace to get debugging output into the trace
    ALI_API void SetTrace(iknow::base::IkTrace& trace);

    //Clear the trace to turn it off
    ALI_API void ClearTrace();

    typedef std::map<iknow::base::String, LanguageBase*> LanguagebaseMap;


    ALI_API LanguagebaseMap& Languagebases();
  }
}
