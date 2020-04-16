#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>
#include <vector>

using namespace iknow::csvdata;
using namespace std;

void iKnow_KB_Metadata::ImportFromCSV(std::string metadata_csv, CSV_DataGenerator& kb)
{
	ifstream ifs = ifstream(metadata_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);

		int count = 0; // Set count = 0
		for (string line; getline(ifs, line);) // while ('stream.AtEnd) { // Set line = $ZCONVERT(stream.ReadLine(), "O", "UTF8")
		{
			++count; // Set count = count + 1
			if (line.find("/*") != string::npos) continue; // Continue:$Find(line, "/*")'=0 // comment line
			if ((std::count(line.begin(), line.end(), ';') + 1) < 2) continue; // Continue : ($L(line, ";") < 2)
			vector<string> row_metadata = kb.split_row(line);
			iKnow_KB_Metadata metadata; // Set metadata = ..%New()
			metadata.Name = row_metadata[1 - 1]; //	Set metadata.Name = $PIECE(line, ";", 1)
			metadata.Val = row_metadata[2 - 1]; //	Set metadata.Val = $PIECE(line, ";", 2)
			//	Set metadata.Knowledgebase = kb
			//	Do kb.AddToHash(metadata.Name)
			//	Do kb.AddToHash(metadata.Val)
			kb.kb_metadata.push_back(metadata); //	Set sc = metadata.%Save()
			//	$$$IKModelCheck(sc, stream.Filename, count, line)
		}
	}
	else {
		cerr << "Error opening file: " << metadata_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	}

	ifs.close();

}

iKnow_KB_Metadata::iKnow_KB_Metadata()
{
}


iKnow_KB_Metadata::~iKnow_KB_Metadata()
{
}
