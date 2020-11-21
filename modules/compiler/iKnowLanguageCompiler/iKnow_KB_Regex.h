#pragma once
#include <string>
#include <vector>

#include "../../aho/ModelRegex.h"

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

				// Check validity of the Pattern : Throws error if regex expression is invalid !
				iknow::model::Regex test_validity(iknow::base::IkStringEncoding::UTF8ToBase(Pattern));
				// Force a dummy match operation, optimizer might throw away previous line for no use...
				iknow::base::String dummy = iknow::base::IkStringEncoding::UTF8ToBase("dummy");
				it_dummy = test_validity.Match(dummy.begin(), dummy.end());
			}
			std::string name, Pattern;

			iKnow_KB_Regex() {}
			~iKnow_KB_Regex() {}

		private:
			iknow::base::String::iterator it_dummy;
		};
	}
}