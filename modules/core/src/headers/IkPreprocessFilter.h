// IkPreprocessFilter.h: interface for the IkPreprocessFilter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKPREPROCESSFILTER_H__6542150A_EC39_43ED_8CB3_3060D5C86C15__INCLUDED_)
#define AFX_IKPREPROCESSFILTER_H__6542150A_EC39_43ED_8CB3_3060D5C86C15__INCLUDED_

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
    //! The PreprocessFilter class.
    /*! This class represents the preprocess filter.
     *  This filter preprocesses the text and normalizes it for input into the engine.
     */
    class CORE_API IkPreprocessFilter : public iknow::base::IkObject 
    {
    public:
      /*!
       * Constructor
       * \param strOriginal: The string to be found in the text.
       * \param strSubstitution: The string the original should be substituted with.
       */
      IkPreprocessFilter(iknow::base::String strOriginal, iknow::base::String strSubstitution);
      /*!
       * Destructor
       */
      virtual ~IkPreprocessFilter();
      virtual std::string GetType(void) const { return "IkPreprocessFilter"; }

      /*!
       * Filters the text.
       * \param strInput: The text that should be filtered. It will contain the filtered stringwhen the function is done.
       * \param bFullMatch: The search string must be a full match
       */
      void Filter(iknow::base::String& strInput) const ;

    protected:
      iknow::base::String m_strOriginal;
      iknow::base::String m_strSubstitution;
      bool m_isGlobal;
    };
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_IKPREPROCESSFILTER_H__6542150A_EC39_43ED_8CB3_3060D5C86C15__INCLUDED_)

