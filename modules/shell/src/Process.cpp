// Process.cpp: implementation of the CProcess class.
//
//////////////////////////////////////////////////////////////////////
#ifdef ISC_IRIS
#include "KernelInterface.h"
#endif

#include "Process.h"
#include "IkStringAlg.h"
#include "IkSentence.h"
#include "IkLexrep.h"
#include "IkMergedLexrep.h"
#include "IkPath.h"
#include "IkLabel.h"
#include "IkOntologyElement.h" 
#include "IkSummaryImportanceRule.h"
#include "IkSummarizer.h"
#include "utlExceptionFrom.h"
#include "IkIndexDebug.h"
#include "PoolAllocator.h"

#include <set>
#include <iostream> // console log
// #include <fstream> // for trace file

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace iknow::base;
using namespace iknow::core;
using namespace iknow::shell;

CProcess::CProcess(const std::map<String, IkKnowledgebase*>& languageKbMap) : m_IsJapaneseInvolved(false), m_tracing_enabled(0), m_merge_relations(1), m_allow_long_sentences(1), m_delimited_sentences_mode(0), m_make_summary(0), m_max_concept_cluster_length(0), m_user_dictionary(NULL), m_stemmer(NULL)
{
  if (languageKbMap.empty())
    throw MessageExceptionFrom<CProcess>("IKNoKBLoaded");

  m_pCore = new iknow::core::IkCore();
  bool some_language_loaded = 0; //Did we load at least one language?
  m_IsJapaneseInvolved = false; //We start with no Japanese

  // workaround for not being able to pass a std map in vc6
  // TODO: Probably no longer needed.
  std::map<String, IkKnowledgebase*>::const_iterator cit;
  for (cit = languageKbMap.begin(); cit != languageKbMap.end() ; ++cit) {
    //Only "real" KBs have rules. User dictionaries don't (yet, at least)
    if (cit->second->RuleCount()) {
      m_pCore->AddToLanguageKbMap(cit->first,cit->second);
      some_language_loaded = 1;
	  if (cit->second->GetMetadata<kIsJapanese>()) {
		  m_IsJapaneseInvolved = true; // reserve a larger lexrep pool
	  }
    }
  }

  if (!some_language_loaded) throw ExceptionFrom<CProcess>("No knowledgebases with rules loaded.");

  m_pCore->FinishConstruction();
}

CProcess::~CProcess()
{
  if(m_pCore)
    delete m_pCore;
}

void CProcess::IndexFunc(IkIndexInput& text_input, OutputFunc func, void *userdata, bool bDomAndProx, bool tracing_enabled)
{
	size_t text_size = text_input.GetString()->size();
	const size_t string_pool_size = text_size * 2 * (m_IsJapaneseInvolved ? 2 : 1);
	const size_t string_pool_string_size = 32;
	static const size_t kMinStringPoolSize = 64;
	static StringPool* string_pool = 0;
	if (!string_pool) {
		string_pool = new StringPool(max(string_pool_size, kMinStringPoolSize), string_pool_string_size);
	}
	else {
		string_pool->Reset(string_pool_size, string_pool_string_size);
	}
	IkLexrep::SetStringPool(string_pool);
	IkLexrep::SetTextBuffer((text_input.GetString())->c_str()); // store pointer to input text data 
	//Allocate space in input size chunks
	static const size_t kMinPoolSize = 16384;
	PoolAllocator<int>::Reset(max(text_size * 32, kMinPoolSize));
	//We need these in their own scope so they're destroyed before
	//we clear the pool. IkIndexOutput uses the pool and IkIndexDebug might someday
	{
		try {
			const size_t lexrep_store_size = text_size / 8 + 16;
			iknow::core::LexrepStore lexrep_store(lexrep_store_size);
			iknow::core::IkLexrep::SetLexrepStore(lexrep_store);
			IkIndexOutput Output;
			IkIndexDebug<TraceListType> Debug;
			m_tracing_enabled = tracing_enabled;
			m_pCore->Index(&text_input, &Output, m_tracing_enabled ? &Debug : 0, m_merge_relations, m_allow_long_sentences, m_delimited_sentences_mode, m_max_concept_cluster_length, m_user_dictionary);
			if (m_make_summary) {
				IkSummarizer::GetInstance()->CalculateSummaryRelevance(Output, m_tracing_enabled ? &Debug : 0);
			}
			if (bDomAndProx) Output.CalculateDominanceAndProximity(); // we need these for UIMA dominance and proximity document annotations 
			func(&Output, m_tracing_enabled ? &Debug : 0, userdata, m_stemmer);
		}
		catch (const std::bad_alloc& ba) {
#ifdef ISC_IRIS
			iKnowKernel::SetError("IKInternalEngineFault", ba.what()); // Notify memory problems to kernel
#else
			std::cerr << "IKInternalEngineFault" << ba.what();
#endif
			// Todo : give indication of text size 
		}
	}
	PoolAllocator<int>::Clear();
}

void CProcess::IndexFunc(const String& const_input, OutputFunc func, void *userdata, bool tracing_enabled, bool merge_relations, bool allow_long_sentences, bool delimited_sentence_mode, bool make_summary, size_t max_concept_cluster_length, IkKnowledgebase* user_dictionary, Stemmer* stemmer) {
  //Windows defines a non-standard max macro, so we have to use "max" below without
  //the std qualifier.
#ifndef WIN32
  using std::max;
#endif //WIN32

  String input = const_input;
  if (user_dictionary) {
    user_dictionary->FilterInput(input);
  }
  IkIndexInput Input(&input); // empty map, no annotations
  // m_tracing_enabled = tracing_enabled;
  m_merge_relations = merge_relations;
  m_allow_long_sentences = allow_long_sentences;
  m_delimited_sentences_mode = delimited_sentence_mode;
  m_make_summary = make_summary;
  m_max_concept_cluster_length = max_concept_cluster_length;
  m_user_dictionary = user_dictionary;
  m_stemmer = stemmer;

  IndexFunc(Input, func, userdata, false, tracing_enabled); // Caché iKnow calls this function
}
