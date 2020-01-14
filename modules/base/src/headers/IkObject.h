// Object.h: interface for the CObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBJECT_H__B6477325_86C1_4ECE_B1C7_2E29AC32A3B0__INCLUDED_)
#define AFX_OBJECT_H__B6477325_86C1_4ECE_B1C7_2E29AC32A3B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4786)
#endif

#include "IkExportBase.h"
#include "SafeString.h"

struct delete_object
{
  template <typename T>
  void operator()(T* ptr)
  {
    delete ptr;
#ifdef _DEBUG
    ptr = NULL ; // only delete once
#endif
  }
};

namespace iknow
{
  namespace base
  {
			
    //! Base-class of all iknow-objects
    /*!
     *	All iknow-objects should be derived from this class
     *	Exceptions: IkException 
     */
    class BASE_API IkObject  
    {
    public:
      IkObject() {} /*!< Constructor. */

      virtual ~IkObject() {} /*!< Destructor. */

      virtual std::string GetType(void) const { return "IkObject"; }

    };
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_OBJECT_H__B6477325_86C1_4ECE_B1C7_2E29AC32A3B0__INCLUDED_)
