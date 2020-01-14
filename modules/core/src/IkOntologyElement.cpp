// IkOntologyElement.cpp: implementation of the IkOntologyElement class.
//
//////////////////////////////////////////////////////////////////////

#include "IkOntologyElement.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace iknow::core;


IkOntologyElement::IkOntologyElement(const iknow::base::String& key, const iknow::base::String& concept, const iknow::base::String& link) : m_strkeyLexrep(key), m_strConcept(concept), m_strOntologyLink(link)
{
	m_bExactMatch = false;
	m_bExactScatteredMatch = false;
	m_bPartialMatch = false;
}

IkOntologyElement::~IkOntologyElement()
{

}

