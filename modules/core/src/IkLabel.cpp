// IkLabel.cpp: implementation of the IkLabel class.
//
//////////////////////////////////////////////////////////////////////

#include "IkLabel.h"
#include "IkKnowledgebase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace iknow::core;
using namespace iknow::base;

// static definition:
const IkLabel::LabelTypeMap IkLabel::label_type_map_;

IkLabel::IkLabel()
{
  m_nType = Unknown;
}

IkLabel::IkLabel(const iknow::base::String& strName, Type nType) : m_strName(strName), m_nType(nType) {}

IkLabel::~IkLabel()
{
}

