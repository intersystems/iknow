#pragma once
#include <string>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		class iKnow_KB_Metadata
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static void ImportFromCSV(std::string metadata_csv, CSV_DataGenerator& kb);

			std::string Name; // Property Name As %String(MAXLEN = 128);

			std::string Val; //  Property Val As %String(MAXLEN = 256);

			iKnow_KB_Metadata();
			~iKnow_KB_Metadata();
		};
	}
}

