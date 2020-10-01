#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>

#include <string>
#include <algorithm>
#include <vector>

using namespace iknow::csvdata;
using namespace std;

// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
bool iKnow_KB_Acronym::ImportFromCSV(std::string acro_csv, CSV_DataGenerator& kb)
{
	ifstream ifs = ifstream(acro_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);

		int count = 0; // Set count = 0
		for (string line; getline(ifs, line);) // while ('stream.AtEnd) { // Set line = $ZCONVERT(stream.ReadLine(), "O", "UTF8")
		{
			++count; // Set count = count + 1
			if (line.find("/*") != string::npos) continue; // Continue:$Find(line, "/*")'=0 // comment line
			if ((std::count(line.begin(), line.end(), ';') + 1) < 4) continue; // Continue : ($L(line, ";") < 4)
			vector<string> row_acronym = kb.split_row(line);
			iKnow_KB_Acronym acronym(row_acronym[3 - 1], row_acronym[4 - 1][0] != '0'); // Set acronym = ..%New()
			// acronym.Token = row_acronym[3 - 1]; // Set acronym.Token = $PIECE(line, ";", 3)
			// acronym.IsSentenceEnd = (row_acronym[4 - 1][0] != '0'); // Set acronym.IsSentenceEnd = $PIECE(line, ";", 4)
			// Set acronym.Knowledgebase = kb
			// Do kb.AddToHash(acronym.Token)
			// Do kb.AddToHash(acronym.IsSentenceEnd)
			kb.kb_acronyms.push_back(acronym); // Set sc = acronym.%Save()
			// $$$IKModelCheck(sc, stream.Filename, count, line)
		}
		ifs.close();
		return true;
	}
	cerr << "Error opening file: " << acro_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	return false;
}
