#pragma once
#include <string>
#include <map>
#include <vector>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include
		static const std::vector<std::string> special_labels = { // language independent labels
			";1,$;Concept;typeConcept;;0;",
			";1,$;Join;typeOther;;0;",
			";1,$;JoinReverse;typeOther;;0;",
			";1,$;NonRelevant;typeOther;;0;",
			";1,$;Punctuation;typeEndConcept;;0;",
			";1,$;Relation;typeRelation;;0;",
			";1,$;Numeric;typeOther;;0;",
			";1,$;Unknown;typeOther;;0;",
			";1,$;CapitalAll;typeAttribute;;0;",
			";1,$;CapitalInitial;typeAttribute;;0;",
			";1,$;CapitalMixed;typeAttribute;;0;",
			";1,$;NonSemantic;typeAttribute;;0;Entity(NonSemantic)",
			";1,$;User1;typeAttribute;;0;",
			";1,$;User2;typeAttribute;;0;",
			";1,$;User3;typeAttribute;;0;",
			";1,$;AlphaBetic;typeConcept;;0;",
			";1,$;Space;typeOther;;0;",
			";1,$;Katakana;typeConcept;;0;",
			";1,$;UDNegation;typeAttribute;;0;",
			";1,$;UDPosSentiment;typeAttribute;;0;",
			";1,$;UDNegSentiment;typeAttribute;;0;",
			";1,$;UDConcept;typeConcept;;0;",
			";1,$;UDRelation;typeRelation;;0;",
			";1,$;UDNonRelevant;typeNonRelevant;;0;",
			";1,$;UDUnit;typeAttribute;;0;",
			";1,$;UDNumber;typeAttribute;;0;",
			";1,$;UDTime;typeAttribute;;0;",
			";1,$;UDCertainty;typeAttribute;;0;",
			";1,$;UDGeneric1;typeAttribute;;0;",
			";1,$;UDGeneric2;typeAttribute;;0;",
			";1,$;UDGeneric3;typeAttribute;;0;"
		};
		class iKnow_KB_Label
		{
		public:
			static bool ImportFromCSV(std::string label_csv, CSV_DataGenerator& kb);

			iKnow_KB_Label(std::string name, std::string type) :
				Name(name), Type(type)
			{}
			iKnow_KB_Label(const char* name, const char* type, const char* attributes) :
				Name(name), Type(type), Attributes(attributes)
			{}
			std::string Name; // Name As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
			std::string Type; // Type As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
			std::string Attributes; // Attributes As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE");

			std::string PhaseList; // Property PhaseList As %String;

		private:
			static void LoadSpecialLabels(CSV_DataGenerator& kb);
			static iKnow_KB_Label LabelFromString(std::vector<std::string>& row_label, std::string &isDefault);
		};
	}
}