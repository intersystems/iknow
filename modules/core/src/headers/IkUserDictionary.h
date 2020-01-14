// IkUserDictionary.h: interface for the IkUserDictionary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IKUSERDICTIONARY_H__5CA03BB3_F9D6_4625_9D41_34AD2F28CEEF__INCLUDED_)
#define AFX_IKUSERDICTIONARY_H__5CA03BB3_F9D6_4625_9D41_34AD2F28CEEF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IkExportCore.h"
#include "IkObject.h"
#include "IkKnowledgebase.h"

namespace iknow
{
	namespace core
	{
		class CORE_API IkUserDictionary : public iknow::base::IkObject
		{
		public:
			IkUserDictionary();
			virtual ~IkUserDictionary();
		
		private:
			virtual void Initialize(IkKnowledgebase* kb) = 0;
		};
	}
}

#endif // !defined(AFX_IKUSERDICTIONARY_H__5CA03BB3_F9D6_4625_9D41_34AD2F28CEEF__INCLUDED_)
