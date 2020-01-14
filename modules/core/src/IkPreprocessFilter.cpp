// IkPreprocessFilter.cpp: implementation of the IkPreprocessFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "IkPreprocessFilter.h"
#include "IkStringAlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace std;
using namespace iknow::core;
using namespace iknow::base;

IkPreprocessFilter::IkPreprocessFilter(iknow::base::String strOriginal, iknow::base::String strSubstitution) : m_strOriginal(strOriginal), 
											       m_strSubstitution(strSubstitution)
{

  const Char *searchpattern=(const Char *)m_strOriginal.c_str();
  size_t len=m_strOriginal.size();
  if ((searchpattern[0] == '\\') && (searchpattern[len-1] == '\\')) {  // full match
    String strSearch = m_strOriginal;
    String::size_type pos=strSearch.length()-1;
    strSearch.erase(pos,1); // remove end '/'
    strSearch.erase(0,1); // remove start '/'
    m_strOriginal = strSearch;
    m_isGlobal = 1;
  }
  else {
    m_isGlobal = 0;
  }
}

IkPreprocessFilter::~IkPreprocessFilter()
{
}

void IkPreprocessFilter::Filter(iknow::base::String& strInput) const
{
  if (m_isGlobal) {
    IkStringAlg::ReplaceGlobal(strInput, m_strOriginal, m_strSubstitution, true);
  } else { // partial match
    IkStringAlg::Replace(strInput, m_strOriginal, m_strSubstitution);
  }
}


