#pragma once

#include "unicode/regex.h"

#include "IkExportCore.h"
#include "IkTypes.h"
#include "IkKnowledgebase.h"

namespace iknow
{
	namespace core
	{
		class CORE_API RegExServices
		{
		public:
			RegExServices();
			~RegExServices();
			bool SplitValueUnit(const iknow::base::String& valunit, iknow::base::String& value, iknow::base::String& unit);
			int Parser2(const iknow::base::String& valunit, iknow::base::String& value, iknow::base::String& unit, iknow::base::String& value2, iknow::base::String& unit2);
			void swich_kb(const iknow::core::IkKnowledgebase* kb);

		private:
			void clean_regex(void);

			const iknow::core::IkKnowledgebase* kb_; // keep a reference to the current kb
			const icu::RegexPattern* pattern_; // compiled regex pattern pointer
			icu::RegexMatcher* matcher_; // regex matcher object pointer
			icu::RegexMatcher* matcher2_; // refined value/unit matcher
		};
	}
}

