#pragma once
#include <string>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		class iKnow_KB_PreprocessFilter
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static void ImportFromCSV(std::string prepro_csv, CSV_DataGenerator& kb);

			std::string InputToken; // Property InputToken As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];

			std::string OutputToken; //  Property OutputToken As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE");

			int Precedence; // Property Precedence As %Integer(XMLPROJECTION = "ATTRIBUTE")[Required];

			// Property Knowledgebase As Knowledgebase[Required];

			// Index KnowledgebaseIndex On(Knowledgebase, Precedence);

			iKnow_KB_PreprocessFilter();
			~iKnow_KB_PreprocessFilter();
		};
	}
}