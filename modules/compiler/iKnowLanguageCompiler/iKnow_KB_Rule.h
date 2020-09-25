#pragma once
#include <string>
#include <map>
#include <vector>
#include <set>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		typedef std::map<std::string, int> newLabelsIndex_type; // mapping new labels to index in newLabels_type
		typedef std::vector<std::pair<std::string, std::string> > newLabels_type; // new labelName and phase store
		typedef std::set<std::string> SPhases_type; // collector for SBegin/SEnd phases

		class iKnow_KB_Rule
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static bool ImportFromCSV(std::string rule_csv, CSV_DataGenerator& kb);

			std::string TransformRulePattern(std::string& csv_rule_input, std::string& csv_phase, CSV_DataGenerator& kb, newLabels_type &newLabels, newLabelsIndex_type &newLabelsIndex, SPhases_type& SBegin, SPhases_type& SEnd);

			std::string csv_id; // identification number in csv-file
			/*
			Property InputPattern As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE") [ Required ];
			Property OutputPattern As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE") [ Required ];
			Property Precedence As %Integer(XMLPROJECTION = "ATTRIBUTE") [ Required ];
			Property Phase As %String(XMLPROJECTION = "ATTRIBUTE");
			Index KnowledgebaseIndex On (Knowledgebase, Precedence) [ Unique ];
			*/
			std::string InputPattern;
			std::string OutputPattern;
			int Precedence;
			std::string Phase;

			iKnow_KB_Rule() {}
			~iKnow_KB_Rule() {}
		};
	}
}