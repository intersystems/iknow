#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>

using namespace iknow::csvdata;
using namespace std;

// ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
bool iKnow_KB_PreprocessFilter::ImportFromCSV(string prepro_csv, CSV_DataGenerator& kb)
{
	ifstream ifs = ifstream(prepro_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);

		int count = 0; // Set count = 0
		for (string line; getline(ifs, line);) // While ('stream.AtEnd) { Set line = $ZCONVERT(stream.ReadLine(), "O", "UTF8")
		{
			++count; // Set count = count + 1
			// cout << line << std::endl;
			if (line.find("/*") != string::npos) continue; // Continue:$Find(line, "/*")'=0 // comment line
			if ((std::count(line.begin(), line.end(), ';') + 1) < 5) continue; // Continue:($L(line,";") < 5)
			vector<string> row_prepro = kb.split_row(line);
			iKnow_KB_PreprocessFilter filter; // Set filter = ..%New()

			filter.InputToken = row_prepro[3 - 1]; // Set filter.InputToken = $PIECE(line, ";", 3)
			filter.OutputToken = row_prepro[4 - 1]; //	Set filter.OutputToken = $PIECE(line, ";", 4)
			filter.Precedence = count; //	Set filter.Precedence = count
			//	Set filter.Knowledgebase = kb
			//	Do kb.AddToHash(filter.InputToken)
			//	Do kb.AddToHash(filter.OutputToken)
			//	Do kb.AddToHash(filter.Precedence)
			kb.kb_prepro.push_back(filter); //	Set sc = filter.%Save()
			//	$$$IKModelCheck(sc, stream.Filename, count, line)
		}
		ifs.close();
		return true;
	}
	cerr << "Error opening file: " << prepro_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	return false;
}