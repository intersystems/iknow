// IkOntology.h: interface for the IkOntology class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKONTOLOGY_H__2D89BE7C_85C4_4EFE_A502_DCABFE2D51D9__INCLUDED_)
#define AFX_IKONTOLOGY_H__2D89BE7C_85C4_4EFE_A502_DCABFE2D51D9__INCLUDED_

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
#include "IkOntologyElement.h"
#include "SafeString.h"
#include "IkTypes.h"


namespace iknow
{
	namespace core
	{	
		//! The ontology base class.
		/*! This class defines the interface to the ontology file.
		 */
		class CORE_API IkOntology : public iknow::base::IkObject  
		{
			public:
				/*!
				 * Constructor
				 */
				IkOntology();
				/*!
				 * Destructor
				 */
				virtual ~IkOntology();
				
		};
	}
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_IKONTOLOGY_H__2D89BE7C_85C4_4EFE_A502_DCABFE2D51D9__INCLUDED_)

