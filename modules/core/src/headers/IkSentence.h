// IkSentence.h: interface for the IkSentence class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKSENTENCE_H__CD568084_09D8_477E_B92C_721C3AFAE379__INCLUDED_)
#define AFX_IKSENTENCE_H__CD568084_09D8_477E_B92C_721C3AFAE379__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IkExportCore.h"
#include "IkObject.h"
#include "IkLexrep.h"
#include "IkMergedLexrep.h"
#include "IkKnowledgebase.h"
#include "SafeString.h"
#include <vector>
#include "IkPath.h"
#include "PoolAllocator.h"

namespace iknow
{
  namespace core
  {
    typedef enum SentenceImportance
      {
	FORCED_TRUE_SENTENCE    =  2, // should ALWAYS  occur in summary
	TRUE_SENTENCE           =  1, // should         occur in summary, even when summaryRelevance is low , except when too much sentences are present.
	NEUTRAL_SENTENCE        =  0,
	FALSE_SENTENCE          = -1, // should NOT     occur in summary, even when summaryRelevance is high, except when needed to reach the desired summary length.
	FORCED_FALSE_SENTENCE   = -2  // should NEVER   occur in summary
      } SENTENCE_IMPORTANCE;

    //! The sentence class
    /*!
     *	This class represent objects thet are sentences
     *	Exceptions: IkException 
     */
    class CORE_API IkSentence : public iknow::base::IkObject, public IkDocumentPart
    {
    public:
      typedef std::vector<IkPath, iknow::base::PoolAllocator<IkPath> > Paths;
      typedef std::vector<size_t, iknow::base::PoolAllocator<size_t> > EntityVector;

      IkSentence(int sentence_order, const IkKnowledgebase* kb, double language_certainty, SENTENCE_IMPORTANCE sentence_importance = NEUTRAL_SENTENCE) :
        sentence_order_(sentence_order), kb_(kb), language_certainty_(language_certainty), sentence_importance_(sentence_importance) {}

      size_t LexrepsSize() const { return lexreps_.size(); }
      size_t PathsSize() const { return paths_.size(); }

      MergedLexreps::const_iterator GetLexrepsBegin() const { return lexreps_.begin(); }
      MergedLexreps::const_iterator GetLexrepsEnd() const { return lexreps_.end(); }

      Paths::const_iterator GetPathsBegin() const { return paths_.begin(); }
      Paths::const_iterator GetPathsEnd() const { return paths_.end(); }

      EntityVector::const_iterator GetEntityVectorBegin() const { return entity_vector_.begin(); }
      EntityVector::const_iterator GetEntityVectorEnd() const { return entity_vector_.end(); }

      MergedLexreps::iterator GetLexrepsBegin() { return lexreps_.begin(); }
      MergedLexreps::iterator GetLexrepsEnd() { return lexreps_.end(); }

      Paths::iterator GetPathsBegin() { return paths_.begin(); }
      Paths::iterator GetPathsEnd() { return paths_.end(); }

      EntityVector::iterator GetEntityVectorBegin() { return entity_vector_.begin(); }
      EntityVector::iterator GetEntityVectorEnd() { return entity_vector_.end(); }

      path::CRCs::iterator GetCRCsBegin() { return crcs_.begin(); }
      path::CRCs::iterator GetCRCsEnd() { return crcs_.end(); }

      //TODO: Replace calls to this with non-const iterator methods, once our IkIndexProcess methods
      //take output iterators rather than containers
      Paths& GetPaths() { return paths_; }
      EntityVector& GetEntityVector() { return entity_vector_; }

      path::CRCs& GetCRCs() { return crcs_; }

      
      MergedLexreps& GetLexreps() { return lexreps_; }

      int GetSentenceOrder() const { return sentence_order_; }

      SENTENCE_IMPORTANCE GetSentenceImportance() const { return sentence_importance_; } // inherited from IkDocumentPart
				
      virtual iknow::base::String GetText() const; // inherited from IkDocumentPart
      virtual iknow::base::String GetNormalizedText() const; // inherited from IkDocumentPart

      bool isForced() const { return isForced(sentence_importance_); }
      void force() {
        if      (sentence_importance_ == TRUE_SENTENCE ) sentence_importance_ = FORCED_TRUE_SENTENCE;
        else if (sentence_importance_ == FALSE_SENTENCE) sentence_importance_ = FORCED_FALSE_SENTENCE;
      }
      static bool isForced(SENTENCE_IMPORTANCE importance) {
        return importance == FORCED_TRUE_SENTENCE || importance == FORCED_FALSE_SENTENCE;
      }

      iknow::base::String GetLanguage () const { return kb_->GetMetadata<kLanguageCode>(); }
	  std::size_t GetP1() const { return kb_->GetMetadata<kP1>(); }
	  std::size_t GetP2() const { return kb_->GetMetadata<kP2>(); }
	  std::size_t GetP3() const { return kb_->GetMetadata<kP3>(); }
	  std::size_t GetP4() const { return kb_->GetMetadata<kP4>(); }
	  std::size_t GetPScale() const { return kb_->GetMetadata<kPScale>(); }
	  bool bModifiersOnLeft() const { return kb_->GetMetadata<kModifiersOnLeft>();  }
	  PathConstruction GetPathConstruction() const { return kb_->GetMetadata<kPathConstruction>();  }
      double GetLanguageCertainty() const { return language_certainty_; }
      const IkKnowledgebase* GetKB() const { return kb_; }


    protected:
      double computeSummaryRelevanceCore() const; // inherited from IkDocumentPart


    private:
      friend class IkIndexOutput;
      friend class IkSummaryImportanceRule;
      MergedLexreps lexreps_;
      Paths paths_;
      EntityVector entity_vector_;
      path::CRCs crcs_;
      int sentence_order_;
      const IkKnowledgebase* kb_;
      double language_certainty_;
      SENTENCE_IMPORTANCE sentence_importance_;
    };
  }
}

#endif // !defined(AFX_IKSENTENCE_H__CD568084_09D8_477E_B92C_721C3AFAE379__INCLUDED_)

