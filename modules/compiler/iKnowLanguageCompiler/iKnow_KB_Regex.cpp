#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>

using namespace iknow::csvdata;
using namespace std;

bool iKnow_KB_Regex::ImportFromCSV(string regex_csv, CSV_DataGenerator& kb)
{
	ifstream ifs = ifstream(regex_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);

		int count = 0;
		set<string> regex_set; // keep set of regexes, to prevent double use
		for (string line; getline(ifs, line);)
		{
			++count;
			// cout << line << std::endl;
			vector<string> row_regex = kb.split_row(line);
			if (row_regex.size() < 2) // not valid, or comment line
				continue;
			if (regex_set.count(row_regex[0]))
				throw ExceptionFrom<iKnow_KB_Regex>("Double use of regex name in : " + line);
			else
				regex_set.insert(row_regex[0]);

			kb.kb_regex.push_back(iKnow_KB_Regex(row_regex)); // create and add label object
			/*
			Continue:$Find(line, "/*")'=0 // comment line
			Continue : ($L(line, ";") < 2) // Not a valid line
			Set regex = ..%New()
			Set name = $PIECE(line, ";", 1)
			Set regex.Name = name
			If $$$IKISDEVBUILD{ // Check for duplicate names
			Throw:$Data(^regexname(kb.Name, name)) ##class(%Exception.General).%New(kb.Name_":Duplicate Regex "_name_" in regex file")
			Set ^regexname(kb.Name, name) = 1 // store name for duplicate check
			}
			Set regex.Pattern = $PIECE(line, ";", 2)
			*/
		}
		ifs.close();
		return true;
	}
	cerr << "Error opening file: " << regex_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	return false;
}