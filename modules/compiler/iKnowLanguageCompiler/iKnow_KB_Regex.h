#pragma once
#include <string>
#include <vector>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		class iKnow_KB_Regex
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static bool ImportFromCSV(std::string regex_csv, CSV_DataGenerator& kb);

			/*
			Set name = $PIECE(line, ";", 1)
			Set regex.Pattern = $PIECE(line, ";", 2)
			*/
			iKnow_KB_Regex(std::vector<std::string>& row_regex) {
				name = row_regex[1 - 1];
				Pattern = row_regex[2 - 1];
			}
			std::string name, Pattern;

			iKnow_KB_Regex() {}
			~iKnow_KB_Regex() {}
		};
	}
}