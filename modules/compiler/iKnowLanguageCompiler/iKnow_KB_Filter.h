#pragma once
#include <string>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		class iKnow_KB_Filter
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static bool ImportFromCSV(std::string filter_csv, CSV_DataGenerator& kb);

			std::string InputToken; // InputToken As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];

			std::string OutputToken; //  OutputToken As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE");

			bool ApplyOnlyAtBeginning; //  Property ApplyOnlyAtBeginning As %Boolean(XMLPROJECTION = "ATTRIBUTE")[Required];

			bool ApplyOnlyAtEnd; // Property ApplyOnlyAtEnd As %Boolean(XMLPROJECTION = "ATTRIBUTE")[Required];

			bool IsConceptFilter; // Property IsConceptFilter As %Boolean(XMLPROJECTION = "ATTRIBUTE")[Required];

			bool IsRelationFilter; //  Property IsRelationFilter As %Boolean(XMLPROJECTION = "ATTRIBUTE")[Required];

			// Property Knowledgebase As Knowledgebase[Required];

			// Index KnowledgebaseIndex On Knowledgebase;

			iKnow_KB_Filter() {}
			~iKnow_KB_Filter() {}
		};
	}
}
