#pragma once
#include <string>
#include <map>
#include <vector>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		class iKnow_KB_Label
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static bool ImportFromCSV(std::string label_csv, CSV_DataGenerator& kb);

			std::string Name; // Name As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];

			std::string Type; // Type As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];

			std::string Attributes; // Attributes As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE");

			// Property Knowledgebase As Knowledgebase[Required];

			std::string PhaseList; // Property PhaseList As %String;

			// Index KnowledgebaseIndex On Knowledgebase;

			// Index NameIndex On(Knowledgebase, Name)[Unique];

		private:
			static void LoadSpecialLabels(CSV_DataGenerator& kb);
			static iKnow_KB_Label LabelFromString(std::vector<std::string>& row_label, std::string &isDefault);
		};
	}
}