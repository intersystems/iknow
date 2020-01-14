// IkOntologyElement.h: interface for the IkOntologyElement class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKONTOLOGYELEMENT_H__4FEF8A98_D29E_446F_A1EE_BF9FE440B8DE__INCLUDED_)
#define AFX_IKONTOLOGYELEMENT_H__4FEF8A98_D29E_446F_A1EE_BF9FE440B8DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IkExportCore.h"
#include "IkObject.h"
#include "IkTypes.h"

namespace iknow
{
	namespace core
	{
		//! The ontology-element class.
		/*!
		 * This class represents 1 ontology element.
		 * An ontology element contains a key, a concept and a link.
		 */
		class CORE_API IkOntologyElement : public iknow::base::IkObject  
		{
			public:
				/*!
				 * Constructor
				 * \param key: The key of the ontology element.
				 * \param concept: The concept of the ontology element
				 * \param link: The link of the ontology element
				 */
				IkOntologyElement(const iknow::base::String& key, const iknow::base::String& concept, const iknow::base::String& link);
				/*!
				 * Destructor
				 * */
				virtual ~IkOntologyElement();

				/*!
				 * Get the key of the ontology element
				 */
				iknow::base::String GetKeyLexrep() const { return m_strkeyLexrep; }
				/*!
				 * Get the concept of the ontology element
				 */
				iknow::base::String GetConcept() const { return m_strConcept; }
				/*!
				 * Get the link of the ontology element
				 */
				iknow::base::String GetOntologyLink() const { return m_strOntologyLink; }
				/*!
				 * Set if the element is an exact match
				 * \param bMatch: true for an exact match
				 */
				void SetExactMatch(bool bMatch) { m_bExactMatch = bMatch; }
				/*!
				 * Get if the element is an exact match
				 */
				bool GetExactMatch(void) const { return m_bExactMatch; }
				/*!
				 * Set if the element is a scattered match
				 * \param bMatch: true for a scattered match
				 */
				void SetExactScatteredMatch(bool bMatch) { m_bExactScatteredMatch = bMatch; }
				/*!
				 * Get if the element is a scattered match
				 */
				bool GetExactScatteredMatch(void) const { return m_bExactScatteredMatch; }
				/*!
				 * Set if the element is a partial match
				 * \param bMatch: true for a partial match
				 */
				void SetPartialMatch(bool bMatch) { m_bPartialMatch = bMatch; }
				/*!
				 * Get if the element is a partial match
				 */
				bool GetPartialMatch(void) const { return m_bPartialMatch; }

				void setKeyLexrep(iknow::base::String str) { m_strkeyLexrep = str; }
				void SetOntologyLink(iknow::base::String str) { m_strOntologyLink = str; } 
				
				
			protected:
				iknow::base::String m_strkeyLexrep;
				iknow::base::String m_strConcept;
				iknow::base::String m_strOntologyLink;
				bool m_bExactMatch;
				bool m_bExactScatteredMatch;
				bool m_bPartialMatch;
		};
		
	}
}

#endif // !defined(AFX_IKONTOLOGYELEMENT_H__4FEF8A98_D29E_446F_A1EE_BF9FE440B8DE__INCLUDED_)
