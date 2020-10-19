#pragma once

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		class iKnow_KB_Acronym
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static bool ImportFromCSV(std::string acro_csv, CSV_DataGenerator& kb);

			std::string Token; // Property Token As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];

			bool IsSentenceEnd; // Property IsSentenceEnd As %Boolean(XMLPROJECTION = "ATTRIBUTE")[Required];

			// Property Knowledgebase As Knowledgebase[Required];

			// Index KnowledgebaseIndex On Knowledgebase;

			iKnow_KB_Acronym(const std::string& token, const bool bIsSEnd) : 
				Token(token),
				IsSentenceEnd(bIsSEnd)
			{}
			~iKnow_KB_Acronym() {}
		};
	}
}
