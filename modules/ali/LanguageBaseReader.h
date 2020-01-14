#ifndef IKNOW_ALI_LANGUAGEBASEREADER_H_
#define IKNOW_ALI_LANGUAGEBASEREADER_H_

#include "LanguageBase.h"
#include "SafeString.h"
#include <map>
#include "aliExports.h"

class ALI_API LanguageBaseReader
{
public:
  LanguageBaseReader(void);
  virtual ~LanguageBaseReader(void);
  
  virtual void read(LanguageBase & languageBase) const = 0;
  
protected:
  inline static LanguageBase::SubstringMap & getSubstrings(LanguageBase & languageBase) { return languageBase.m_substrings; }
  inline static size_t                        & getTotalScore(LanguageBase & languageBase) { return languageBase.m_totalScore; }
};

#endif //IKNOW_ALI_LANGUAGEBASEREADER_H_
