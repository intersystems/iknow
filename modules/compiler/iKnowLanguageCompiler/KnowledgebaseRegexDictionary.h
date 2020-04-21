#pragma once
#ifdef WIN32
#pragma warning (disable: 4251)
#endif

#include "IkTypes.h"

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator;
	}
}
namespace iknow {
	namespace AHO {

		class KnowledgebaseRegexDictionary
		{
		public:
			KnowledgebaseRegexDictionary();
			~KnowledgebaseRegexDictionary();

			iknow::csvdata::CSV_DataGenerator *Knowledgebase; // Property Knowledgebase As %iKnow.KB.Knowledgebase;

			iknow::base::String Lookup(std::string name); // Method Lookup(name As %String) As %String
			iknow::base::String Lookup(iknow::base::String); 
		};

	}
}

