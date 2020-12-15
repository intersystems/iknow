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

			iknow::base::String language_code_; // the language code
			const icu::RegexPattern* pattern_; // compiled regex pattern pointer
			icu::RegexMatcher* matcher_; // regex matcher object pointer
			icu::RegexMatcher* matcher2_; // refined value/unit matcher
		};
	}
}

