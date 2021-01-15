// IndexProcess.h: interface for the CIndexProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INDEXPROCESS_H__5CCA0607_73DA_45FF_8E64_C410DD72DD49__INCLUDED_)
#define AFX_INDEXPROCESS_H__5CCA0607_73DA_45FF_8E64_C410DD72DD49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
// Disable stl warnings
#pragma warning (push)
#pragma warning (disable: 4786)
#endif

#define MAX_SENTENCE_SIZE 1024
#include "IkExportCore.h"
#include "IkObject.h"
#include "IkIndexInput.h"
#include "IkIndexOutput.h"
#include "IkKnowledgebase.h"
#include "IkMetadataCache.h"
#include "IkOntology.h"
#include "IkPath.h"
#include "SafeString.h"
#include "IkIndexDebug.h"
#include "IkLexrep.h"
#include "IkMergedLexrep.h"
#include <vector>

namespace iknow
{
  namespace core
  {
		
    //! The indexing process
    /*!
     *	The Y form YMCA, indexes the text in the input-object and returns an output-object.
     */
    class CORE_API IkIndexProcess : public iknow::base::IkObject
    {
    public:
      typedef IkSentence::Paths Paths;
				
      /*!
       *	Constructor
       *	\param languageKbMap: a list of languages with the needed knowledgebases.
       */
      IkIndexProcess(std::map<iknow::base::String, IkKnowledgebase const *> const & languageKbMap);

      virtual ~IkIndexProcess(); /*!< Destructor. */

      /*!
       *	Start the process
       *	\param pInput: pointer to a input-object (must contain the text)en
       *	\param pOutput: pointer to the output-object
       *	\param nMaxConceptSize: the maximum conceptSize
       *	\exception IkException: UNKNOWN ERROR
       */
      void Start(IkIndexInput* pInput, IkIndexOutput* pOutput, IkIndexDebug<TraceListType>* pDebug, bool bMergeRelations, bool bBinaryMode, bool delimitedSentences, size_t max_concept_cluster_length=0, IkKnowledgebase* pUdct=NULL);

      iknow::base::String NormalizeText(const iknow::base::String& input, const IkKnowledgebase* kb, const IkKnowledgebase* ud, bool bLowerCase=true, bool bStripPunct=true);
      
    private:
		/*!
		*	Preprocess string token into initial lexreps
		*	\param labels : If the text is annotated, these are the labels to apply (UIMA user dictionary mechanism)
		*/
		void Preprocess(const iknow::base::Char* val_begin, const iknow::base::Char* val_end, Lexreps& lexrep_vector, std::string const * labels = NULL);

      /*!
       *	Find the next sentence 
       *	\param pInput: pointer to a input-object (must contain the text)
       *	\param nPosition: current position
       *	\exception IkException: UNKNOWN ERROR
       */
      bool FindNextSentence(IkIndexInput* pInput, Lexreps& lexrep_vector, int& nPosition, size_t cntWordLimit, bool delimitedSentences, iknow::base::String& kb_name, double& certainty, IkKnowledgebase *pUdct=NULL, double certaintyThresholdForChangingLanguage = 0.35, int nPositionEndOfPreviousIteration = 0); // TODO: why 0.35 ???
	  bool FindNextSentenceJP(IkIndexInput* pInput, Lexreps& lexrep_vector, int& nPosition, size_t cntWordLimit);

      // Find known lexreps by searching the knowledgebase, use internal if pkb==NULL
      void FindKnownLexreps(bool first_run, Lexreps& in_lexrep_vector, Lexreps& out_lexrep_vector, iknow::core::IkKnowledgebase *pkb=NULL);
      IkLexrep NextLexrep(Lexreps::iterator& current, Lexreps::iterator end, iknow::core::IkKnowledgebase *pkb=NULL);

	  void MergeKatakana(Lexreps& lexreps); // For Japanese, Katakana symbols are merged before rules processing...

      /* Add "special" labels for literal attributes like capitalization.
      */
      void AddAttributeLabels(Lexreps& lexreps);
      void AddAttributeLabels(IkLexrep& lexrep);

      void AddUserLabels(Lexreps& lexreps, const IkKnowledgebase& ud);
      void AddUserLabels(IkLexrep& lexrep, const IkKnowledgebase& ud);

      void DetectCapitalization(IkLexrep& lexrep);

      /*!
       *	Solve ambiguous lexreps ant try to classify them
       *	\param lstLexreps: list of lexreps
       *	\exception IkException: UNKNOWN ERROR
       */
      void SolveAmbiguous(Lexreps& lexreps);

      /*!
       *	Merge the concepts and convert the lexreps to the appropriate type
       *	\param lstLexreps: list of lexreps
       *	\exception IkException: UNKNOWN ERROR
       */
      void MergeAndConvert(Lexreps& in_lexrep_vector, MergedLexreps& out_lexrep_vector, bool mergeRelations);

      /*!
       *	Merge the concepts 
       *	\param lstNewLexreps: list of lexreps
       *	\param nConceptCounter: number of items to merge
       *	\exception IkException: UNKNOWN ERROR
       */
      void MergeConcepts(Lexreps& in_lexrep_vector, MergedLexreps& out_lexrep_vector);

      void MergeNextConcepts(Lexreps::iterator& begin, const Lexreps::iterator& end, MergedLexreps& out_lexrep_vector);

      void MergeConceptLexreps(const Lexreps::iterator& begin, const Lexreps::iterator& end, MergedLexreps& out_lexrep_vector);
      /*!
      ** Apply entity filter: will remove empty concept lexreps (index & literal),
      ** will apply the KB concept filter rules,
      ** and count all valid entities (concepts & relations).
      ** \param lstLexreps: list of lexreps.
	  ** \return count of entities (valid concepts & relations) in lexreps.
      */
      void EntityFilter(MergedLexreps& lexrep_vector);
	  void MergeRelation(const Lexreps::iterator& begin, const Lexreps::iterator& end, size_t& cnt_nonrels_in_relation, MergedLexreps& out_lexrep_vector);
      void MergeRelationNonrelevant(IkLexrep& lexrep, MergedLexreps& out_lexrep_vector);
      void MergeRelations(Lexreps& in_lexrep_vector, MergedLexreps& out_lexrep_vector);

      /*!
       *	Find CRC/CCR-patterns 
       *	\param lstLexreps: list of lexreps
       *	\return ArrayList of CRC-patterns
       *	\exception IkException: UNKNOWN ERROR
       */
      void DetectCRCPatterns(const MergedLexreps& in_lexrep_vector, path::CRCs& out_crc_vector, const ChainPattern pattern);
      void BuildPathsFromCRCs(const MergedLexreps& in_lexrep_vector, const path::CRCs& crc_vector, Paths& out_path_vector);
      void BuildPathsFromAttributes(const MergedLexreps& in_lexrep_vector, Paths& out_path_vector);
	  void BuildPathsFromPathRelevants(const MergedLexreps& in_lexrep_vector, Paths& out_path_vector);
      void BuildEntityVector(const MergedLexreps& in_lexrep_vector, IkSentence::EntityVector& out_entity_vector);
      void AddPath(path::CRCs::const_iterator start, path::CRCs::const_iterator end, Paths& out_path_vector);

    private:
      void SetCurrentKnowledgebase(const IkKnowledgebase* kb) {
        m_pKnowledgebase = kb;
        if (!m_external_max_ccl) // not externally set.
          m_max_concept_cluster_length = m_pKnowledgebase->GetMetadata<kConceptsToMergeMax>();
		m_max_nonrels_in_relation = m_pKnowledgebase->GetMetadata<kNonRelevantsToMergeMax>();
      }
      IkKnowledgebase const * m_pKnowledgebase;
      bool m_external_max_ccl; // if true, the max concept cluster parameter is externally set, if false, use the KB setting. We need to know the difference when switching KB's if ALI is used. 
      size_t m_max_concept_cluster_length; // choice between external (via COS) parameter, or preferably set in the KB. Since the parameter does not change while indexing, it should be set at the highest level.
	  size_t m_max_nonrels_in_relation; // maximum number of nonrelevants in a relation merge
	  IkLexrep m_begin_lr, m_end_lr; // head & tail lexreps (SBegin and SEnd), always added to surround the sentence

      typedef std::map<iknow::base::String, IkKnowledgebase const *> KbMap;
      const KbMap& m_languageKbMap;
      IkIndexDebug<TraceListType>* m_pDebug;
      //No assignment operator.
      void operator=(const IkIndexProcess& other);
    };
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_INDEXPROCESS_H__5CCA0607_73DA_45FF_8E64_C410DD72DD49__INCLUDED_)

