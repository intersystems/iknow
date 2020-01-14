// IkDocumentPart.h: interface for the IkDocumentPart class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKDOCUMENTPART_H__1064EBB0_5419_439B_9C24_0D4C3CF71CAB__INCLUDED_)
#define AFX_IKDOCUMENTPART_H__1064EBB0_5419_439B_9C24_0D4C3CF71CAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IkExportCore.h"
#include "SafeString.h"
#include "IkTypes.h"

namespace iknow
{
  namespace core 
  {
    class CORE_API IkDocumentPart  
    {
      friend class IkLexrep;
    public:

      IkDocumentPart();
      virtual ~IkDocumentPart();

      double GetSummaryRelevance() const
      {
	if (!m_summaryRelevanceComputed)
	  computeSummaryRelevance();
	return m_summaryRelevance;
      }
      virtual iknow::base::String GetText          () const = 0;
      virtual iknow::base::String GetNormalizedText() const = 0;

      void increaseSummaryRelevance(size_t value) { m_summaryRelevance += value; }
      void scaleSummaryRelevance(double value)    {	m_summaryRelevance *= value; } // only for sentences : percentage: 1 --> no change
      void dropSummary(void)                      { m_summaryRelevance = 0;      } // zero means : should not appear in summary (minimum is 1)
      void trueSummary(void)                      { m_summaryRelevance = (-m_summaryRelevance); } // negate to mark importance

    protected:
      virtual double computeSummaryRelevanceCore() const = 0;

    private:
      void computeSummaryRelevance() const {
        m_summaryRelevance = m_summaryRelevance?m_summaryRelevance:computeSummaryRelevanceCore(); // only compute when m_summaryRelevance is not 0.
        m_summaryRelevanceComputed = true;
      }
      mutable double  m_summaryRelevance;
      mutable bool    m_summaryRelevanceComputed;
    };
  }
}

#endif // !defined(AFX_IKDOCUMENTPART_H__1064EBB0_5419_439B_9C24_0D4C3CF71CAB__INCLUDED_)
