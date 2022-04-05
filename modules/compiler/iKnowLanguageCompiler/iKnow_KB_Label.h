#pragma once
#include <string>
#include <map>
#include <vector>
#include <set>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include
		static const std::set<std::string> accepted_types = { // label types
			"typeConcept",
			"typeOther",
			"typeEndConcept",
			"typeRelation",
			"typeAttribute",
			"typeNonRelevant",
			"typeAmbiguous",
			"typePathRelevant",
			"typeBeginEndConcept",
			"typeBeginConcept",
			"typeBeginRelation",
			"typeEndRelation",
			"typeBeginEndRelation",
			"typeLiteral"
		};
		static const std::set<std::string> accepted_attributes = {
			"Entity(Certainty)",
			"Entity(Generic1)",
			"Path(Begin,Generic1)",
			"Path(End,Generic1)",
			"Entity(Generic2)",
			"Path(Begin,Generic2)",
			"Path(End,Generic2)",
			"Entity(Generic3)",
			"Path(Begin,Generic3)",
			"Path(End,Generic3)",
			"Entity(Negation)",
			"Path(Begin,Negation)",
			"Path(End,Negation)",
			"Entity(DateTime)",
			"Entity(PositiveSentiment)",
			"Entity(NegativeSentiment)",
			"Path(Begin,PositiveSentiment)",
			"Path(End,PositiveSentiment)",
			"Path(Begin,NegativeSentiment)",
			"Path(End,NegativeSentiment)",
			"Entity(Measurement,Value,Unit)",
			"Entity(Frequency)",
			"Entity(Duration)",
			"Path(Begin,DateTime)",
			"Path(End,DateTime)",
			"Path(Begin,Measurement)",
			"Path(End,Measurement)",
			"Entity(Measurement)",
			"Entity(Measurement,Unit)",
			"Entity(Measurement,Value)",
			"Path(Begin,Certainty)",
			"Path(End,Certainty)"

		};
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
			";1,$;UDGeneric3;typeAttribute;;0;",
			";1,$;UDIgnoreNegation;typeAttribute;;0;",
			";1,$;UDIgnoreSentiment;typeAttribute;;0;",
			";1,$;UDIgnorePosSentiment;typeAttribute;;0;",
			";1,$;UDIgnoreNegSentiment;typeAttribute;;0;",
			";1,$;UDIgnoreNumber;typeAttribute;;0;",
			";1,$;UDIgnoreTime;typeAttribute;;0;",
			";1,$;UDIgnoreUnit;typeAttribute;;0;",
			";1,$;UDIgnoreCertainty;typeAttribute;;0;"
		};
		class iKnow_KB_Label
		{
		public:
			static bool ImportFromCSV(std::string label_csv, CSV_DataGenerator& kb);

			iKnow_KB_Label(std::string name, std::string type) :
				Name(name), Type(type)
			{
				_checktype(type.c_str());
			}
			iKnow_KB_Label(const char* name, const char* type, const char* attributes) :
				Name(name), Type(type), Attributes(attributes)
			{
				_checktype(type);
				_checkattribute(attributes);
			}
			std::string Name; // Name As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
			std::string Type; // Type As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
			std::string Attributes; // Attributes As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE");

			std::string PhaseList; // Property PhaseList As %String;

		private:
			static void LoadSpecialLabels(CSV_DataGenerator& kb);
			static iKnow_KB_Label LabelFromString(std::vector<std::string>& row_label, std::string &isDefault);
			static void _checktype(const char* type) {
				if (accepted_types.count(std::string(type)) == 0) { // type unknown
					throw ExceptionFrom<iKnow_KB_Label>("Label Type: \"" + std::string(type) + "\" Unknown !");
				}
				return;
			}
			static void _checkattribute(const char* attribute) {
				if (accepted_attributes.count(std::string(attribute)) == 0) {
					// EVSlot(0, 0, Topic, L, B) | EVSlot(0, 1, Subject, L, B) | EVSlot(0, 2, Object, L, B)
					if (std::string(attribute).find("EVSlot") != std::string::npos)
						return;
					// EVValue(0, 1, Subject, L, B)
					if (std::string(attribute).find("EVValue") != std::string::npos)
						return;
					throw ExceptionFrom<iKnow_KB_Label>("Attribute: \"" + std::string(attribute) + "\" Unknown !");
				}
				return;
			}
		};
	}
}