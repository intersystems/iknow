// Core.cpp: implementation of the IkCore class.
//
//////////////////////////////////////////////////////////////////////

#include "IkCore.h"

//exceptions
#include "IkIndexProcess.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace iknow::core;

IkCore::IkCore()
{ }

IkCore::~IkCore()
{
  delete m_pIndexProcess;
}

void IkCore::FinishConstruction()
{
  m_pIndexProcess = new IkIndexProcess(m_languageKbMap);
}

void IkCore::Index(IkIndexInput* pInput, IkIndexOutput* pOutput, IkIndexDebug* pDebug, bool mergeRelations, bool binaryMode, bool delimitedSentences, size_t max_concept_cluster_length, IkKnowledgebase* pUdct)
{
  m_pIndexProcess->Start(pInput, pOutput, pDebug, mergeRelations, binaryMode, delimitedSentences, max_concept_cluster_length, pUdct);
}


