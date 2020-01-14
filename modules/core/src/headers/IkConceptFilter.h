// ConceptFilter.h: interface for the CConceptFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONCEPTFILTER_H__9CEEDF2F_0E1D_4F90_B759_D30540513473__INCLUDED_)
#define AFX_CONCEPTFILTER_H__9CEEDF2F_0E1D_4F90_B759_D30540513473__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
// Disable stl warnings
#pragma warning (push)
#pragma warning (disable: 4786)
#endif

#include "IkExportCore.h"
#include "IkObject.h"
#include "SafeString.h"
#include "IkTypes.h"

namespace iknow
{
  namespace core
  {
    //! The conceptfilter class
    /*!
     *	This class is a filter for conceptstrings
     *	Exceptions: IkException 
     */
    class CORE_API IkIndexFilter : public iknow::base::IkObject 
    {
    public:
		typedef enum {
          _concept, _relation, _nonrelevant
		} Flt_Type ;

      /*!
       *	Constructor
       *	\param strPattern: the original pattern to be find
       *	\param strReplacementValue: the replacement pattern
       *	\param nPosition: specify the position of the original pattern (0=everywhere, 1=begin, 2=end, 3=begin&end)
       */
      IkIndexFilter(iknow::base::String const & strPattern, iknow::base::String const & strReplacementValue, int const & nPosition);
      virtual ~IkIndexFilter();/*!< Destructor. */
      virtual std::string GetType(void) const { return "IkIndexFilter"; }
	  Flt_Type getType(void) const { return m_Type; }

      /*!
       *	Filter a conceptstring
       *	\param strConcept: the conceptstring to be filtered
       *	\exception IkException: UNKNOWN ERROR
       */
      void Filter(iknow::base::String& strConcept) const;

      bool Equals(iknow::base::IkObject const * const & obj) const;

    protected:
      /*!
       *	Filter a conceptstring at the beginning of the string
       *	\param strConcept: the conceptstring to be filtered
       *	\exception IkException: UNKNOWN ERROR
       */
      void FilterBegin(iknow::base::String& strConcept) const;

      /*!
       *	Filter a conceptstring at the end of a string
       *	\param strConcept: the conceptstring to be filtered
       *	\exception IkException: UNKNOWN ERROR
       */
      void FilterEnd(iknow::base::String& strConcept) const;

      /*!
       *	Filter a conceptstring anywhere in the string
       *	\param strConcept: the conceptstring to be filtered
       *	\exception IkException: UNKNOWN ERROR
       */
      void FilterEverywhere(iknow::base::String& strConcept) const;

    protected:
      iknow::base::String /*const*/ m_strPattern;
      iknow::base::String const m_strReplacementValue;
      int const m_nPosition;
      Flt_Type /*const*/ m_Type;
    private:
      void operator=(const IkIndexFilter&);
    };
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_CONCEPTFILTER_H__9CEEDF2F_0E1D_4F90_B759_D30540513473__INCLUDED_)

