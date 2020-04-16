#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>

using namespace iknow::csvdata;
using namespace std;

// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
void iKnow_KB_Filter::ImportFromCSV(string filter_csv, CSV_DataGenerator& kb)
{
	ifstream ifs = ifstream(filter_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);

		int count = 0; // Set count = 0
		for (string line; getline(ifs, line);) // While ('stream.AtEnd) { Set line = $ZCONVERT(stream.ReadLine(), "O", "UTF8")
		{
			++count; // Set count = count + 1
			if (line.find("/*") != string::npos) continue; // Continue:$Find(line, "/*")'=0 // comment line
			if ((std::count(line.begin(), line.end(), ';') + 1) < 5) continue; // Continue:($L(line,";") < 5)
			vector<string> row_filter = kb.split_row(line);
			iKnow_KB_Filter filter; // Set filter = ..%New()
			string inputToken = row_filter[3 - 1]; // Set inputToken = $PIECE(line, ";", 3)
			char fType = inputToken[0]; // Set fType = $Extract(inputToken)
			switch (fType) { // Set filter.InputToken = $Case(fType, "C":$Extract(inputToken, 2, *), "R" : $Extract(inputToken, 2, *), "N" : $Extract(inputToken, 2, *), "P" : $Extract(inputToken, 2, *), : inputToken)
			case 'C': filter.InputToken = string(inputToken.begin() + 1, inputToken.end());
				break;
			case 'R': filter.InputToken = string(inputToken.begin() + 1, inputToken.end());
				break;
			case 'N': filter.InputToken = string(inputToken.begin() + 1, inputToken.end());
				break;
			case 'P': filter.InputToken = string(inputToken.begin() + 1, inputToken.end());
				break;
			default: filter.InputToken = inputToken;
			}
			// For the new PathRelevant, we use a hack : if both Concept and Relation are flagged true, it is pathrelevant, if both are false, it is nonrelevant.
			filter.IsConceptFilter = ((fType == 'C' || fType == 'P') ? 1 : ((fType == 'R' || fType == 'N') ? 0 : 1)); // Set filter.IsConceptFilter = $Case(fType, "C":1, "R" : 0, "N" : 0, "P" : 1, : 1)
			filter.IsRelationFilter = ((fType == 'R' || fType == 'P') ? 1 : 0); // Set filter.IsRelationFilter = $Case(fType, "C":0, "R" : 1, "N" : 0, "P" : 1, : 0)

			filter.OutputToken = row_filter[4 - 1]; //	Set filter.OutputToken = $PIECE(line, ";", 4)
			string where = row_filter[5 - 1]; // Set where = $Piece(line, ";", 5)
			filter.ApplyOnlyAtBeginning = (where == "1" || where == "3"); // Set filter.ApplyOnlyAtBeginning = (where = 1) || (where = 3)
			filter.ApplyOnlyAtEnd = (where == "2" || where == "3"); // Set filter.ApplyOnlyAtEnd = (where = 2) || (where = 3)
			//	Set filter.Knowledgebase = kb
			// Do kb.AddToHash(filter.InputToken)
			//	Do kb.AddToHash(filter.OutputToken)
			//	Do kb.AddToHash(filter.ApplyOnlyAtBeginning)
			//	Do kb.AddToHash(filter.ApplyOnlyAtEnd)
			//	Do kb.AddToHash(filter.IsConceptFilter)
			//	Do kb.AddToHash(filter.IsRelationFilter)
			kb.kb_filter.push_back(filter); //	Set sc = filter.%Save()
			//	$$$IKModelCheck(sc, stream.Filename, count, line)
		}
	}
	else {
		cerr << "Error opening file: " << filter_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	}
	ifs.close();
}


iKnow_KB_Filter::iKnow_KB_Filter()
{
}


iKnow_KB_Filter::~iKnow_KB_Filter()
{
}
