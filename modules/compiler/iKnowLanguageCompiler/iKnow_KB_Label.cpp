#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>

using namespace iknow::csvdata;
using namespace std;

void iKnow_KB_Label::LoadSpecialLabels(CSV_DataGenerator& kb)
{
	string isDefault = "";
	for (vector<string>::const_iterator it = special_labels.begin(); it != special_labels.end(); ++it) {
		vector<string> row_label = kb.split_row(*it);
		kb.kb_labels.push_back(iKnow_KB_Label::LabelFromString(row_label,isDefault)); // create and add label object
	}
}

bool iKnow_KB_Label::ImportFromCSV(string labels_csv, CSV_DataGenerator& kb)
{
	LoadSpecialLabels(kb); // Do ..LoadSpecialLabels(kb)
	int count = 0;
	ifstream ifs = ifstream(labels_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);
		
		for (string line; getline(ifs, line);) // While ('stream.AtEnd) { // Set line = $ZCONVERT(stream.ReadLine(), "O", "UTF8")
		{
			count = count + 1;
			if (line.find("/*") != string::npos) continue; // Continue:$Find(line, "/*")'=0 // comment line
			if ((std::count(line.begin(), line.end(), ';') + 1) < 6) continue; // Continue : ($L(line, ";") < 6)
			vector<string> row_label = kb.split_row(line);
			string isDefault = ""; // ByRef isDefault = ""
			iKnow_KB_Label label = LabelFromString(row_label, isDefault);
			kb.kb_labels.push_back(label); // Set sc = ..AddLabelToKB(label, kb)
			if (isDefault[0]=='1') kb.kb_concept_label = &kb.kb_labels.back(); // If isDefault Set kb.ConceptLabel = label
		}
		ifs.close();
		return true;
	}
	cerr << "Error opening file: " << labels_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	return false;
}

iKnow_KB_Label iKnow_KB_Label::LabelFromString(vector<string>& row_label, string &isDefault) // ClassMethod LabelFromString(line As %String, ByRef isDefault = "") As Label
{
	iKnow_KB_Label label(row_label[3 - 1], row_label[4 - 1]);
	isDefault = row_label[6 - 1]; // Set isDefault = $PIECE(line, ";", 6)

	if (row_label.size()>7) label.Attributes = row_label[8 - 1]; // Set label.Attributes = $PIECE(line, ";", 8)
	label.PhaseList = row_label[2 - 1]; // Set label.PhaseList = $PIECE(line, ";", 2)
	return label;
}