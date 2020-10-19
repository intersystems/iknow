#pragma once
#include <string>
#include <list>
#include <algorithm>
#include <vector>

namespace iknow {
	namespace csvdata {
		class CSV_DataGenerator; // forward to avoid circular include

		class iKnow_KB_Lexrep
		{
		public:
			// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
			static bool ImportFromCSV(std::string lexrep_csv, CSV_DataGenerator& kb);

			iKnow_KB_Lexrep(std::vector<std::string>& row_lexrep) {
				Meta = row_lexrep[2 - 1];
				Token = row_lexrep[3 - 1];
				std::string& the_labels = Labels; // reference for lambda expression
				for_each(row_lexrep.begin() + 5 - 1, row_lexrep.end(), [&the_labels](std::string& label) { the_labels += label + ";"; });

				std::string::reverse_iterator rit = the_labels.rbegin();
				while (rit != the_labels.rend() && *rit == ';') ++rit;
				the_labels.erase(rit.base(), the_labels.end()); // remove ending ';'
			}
			iKnow_KB_Lexrep(const std::string& token, const std::string& label) :	// for UDCT use, a token with a single label
				Token(token),
				Labels(label + ";")
			{}

			std::string Token; // Property Token As %String(MAXLEN = 2048, XMLPROJECTION = "ATTRIBUTE")[Required];

			std::string Meta; // Property Meta As %String(MAXLEN = 128, XMLPROJECTION = "ATTRIBUTE");

			std::string Labels; // Property Labels As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE") [ Required ];

			// Property Knowledgebase As Knowledgebase[Required];

			// Index KnowledgebaseIndex On Knowledgebase;

			iKnow_KB_Lexrep() : isRegex(false) {}
			~iKnow_KB_Lexrep() {}

		//private:
			bool isRegex;
		};

		// static std::list<iknow::base::String> ParseAlternatives(iknow::base::String& input);
		template<typename T>
		static std::list<T> ParseAlternatives(T& input)
		{
			size_t len = input.length();
			int depth = 1; //the leading "("
			T token; // = "";
			std::list<T> tokenList; // Set tokenList = ""
			bool escaped = false; // Set escaped = 0
			int i = 2 - 1;
			for (; i < (int)len; ++i) {
				typename T::value_type char_ = input[i]; // Set char = $E(input, i)

				//Process regexp characters
				if (!escaped) {
					if (char_ == '(') {
						depth = depth + 1;
					}
					else {
						if (char_ == ')') {
							depth = depth - 1;
						}
						else {
							if (char_ == '|' && depth == 1) {
								tokenList.push_back(token);
								token.clear(); // = "";
								continue;
							}
							else {
								if (char_ == '\\') {
									escaped = true;
									continue;
								}
							}
						}
					}
				}
				//Add character to current token
				escaped = false;
				if (depth != 0) {
					token = token + char_;
				}
				else {
					tokenList.push_back(token);
					token.clear(); // = "";
					break;
				}
			}
			// if (depth != 0) Throw ##class(%Exception.SystemException).%New("Mismatched Parentheses")
			input = T(input.begin() + i + 1, input.end());
			return tokenList;
		}

	}
}