// ConceptFilter.cpp: implementation of the CConceptFilter class.
//
//////////////////////////////////////////////////////////////////////

#include "IkConceptFilter.h"
#include "IkStringAlg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace iknow::core;
using iknow::base::Char;
using iknow::base::String;

IkIndexFilter::IkIndexFilter(String const & strPattern, String const & strReplacementValue, int const & nPosition) 
: m_strPattern(strPattern), m_strReplacementValue(strReplacementValue), m_nPosition(nPosition)
{
	switch(strPattern[0]) { // type defined filter
		case Char('C'): // concept
			m_strPattern.erase(0,1); // remove filter type indicator
			m_Type=_concept;
			break;
		case Char('R'): // relation
			m_strPattern.erase(0,1); // remove filter type indicator
			m_Type=_relation;
			break;
		case Char('N'): // nonrelevant
			m_strPattern.erase(0,1); // remove filter type indicator
			m_Type=_nonrelevant;
			break;
		default: // old style concept
			m_Type=_concept;
			break;
	}
}

IkIndexFilter::~IkIndexFilter()
{
}

bool IkIndexFilter::Equals(iknow::base::IkObject const * const & obj) const
{
  if(obj->GetType() != GetType())
    return false;
  else
    {
      IkIndexFilter* filter = (IkIndexFilter*) obj;
      return (filter->m_nPosition == m_nPosition && filter->m_strPattern == m_strPattern && filter->m_strReplacementValue == m_strReplacementValue);
    }
}

void IkIndexFilter::Filter(String& strConcept) const
{
  switch(m_nPosition)
    {
    case 0: FilterEverywhere(strConcept);
      break;
    case 1: FilterBegin(strConcept);
      break;
    case 2: FilterEnd(strConcept);
      break;
    case 3: FilterBegin(strConcept);
      FilterEnd(strConcept);
      break;	
    }
  iknow::base::IkStringAlg::Trim(strConcept);
}

void IkIndexFilter::FilterBegin(String& strConcept) const
{
  size_t nPos = strConcept.find(m_strPattern, 0);
  if (nPos == 0)
  {
      strConcept.replace(nPos, m_strPattern.size(), m_strReplacementValue);	
  }
}

void IkIndexFilter::FilterEnd(String& strConcept) const
{
  size_t nPos = strConcept.rfind(m_strPattern, strConcept.size());
  if (nPos == (strConcept.size()-m_strPattern.size()) && nPos != String::npos )
  {
      strConcept.replace( nPos, m_strPattern.size(), m_strReplacementValue);
  }
}

void IkIndexFilter::FilterEverywhere(String& strConcept) const
{
  size_t nPos = strConcept.find(m_strPattern, 0); 
  while (nPos != String::npos )
  {
      strConcept.replace( nPos, m_strPattern.size(), m_strReplacementValue );
      nPos = strConcept.find(m_strPattern, 0);
  }
}
