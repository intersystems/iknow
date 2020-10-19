#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>

using namespace iknow::csvdata;
using namespace std;

vector<string> special_labels = { // language independent labels
	";1,$;Concept;typeConcept;;0;",
	";1,$;Join;typeOther;;0;",
	";1,$;JoinReverse;typeOther;;0;",
	";1,$;NonRelevant;typeOther;;0;",
	";1,$;Punctuation;typeEndConcept;;0;",
	";1,$;Relation;typeRelation;;0;",
	";1,$;Numeric;typeOther;;0;",
	";1,$;Unknown;typeOther;;0;",
	";1,$;CapitalAll;typeAttribute;;0;",
	";1,$;CapitalInitial;typeAttribute;;0;",
	";1,$;CapitalMixed;typeAttribute;;0;",
	";1,$;NonSemantic;typeAttribute;;0;Entity(NonSemantic)",
	";1,$;User1;typeAttribute;;0;",
	";1,$;User2;typeAttribute;;0;",
	";1,$;User3;typeAttribute;;0;",
	";1,$;AlphaBetic;typeConcept;;0;",
	";1,$;Space;typeOther;;0;",
	";1,$;Katakana;typeConcept;;0;",
	";1,$;UDNegation;typeAttribute;;0;",
	";1,$;UDPosSentiment;typeAttribute;;0;",
	";1,$;UDNegSentiment;typeAttribute;;0;",
	";1,$;UDConcept;typeConcept;;0;",
	";1,$;UDRelation;typeRelation;;0;",
	";1,$;UDNonRelevant;typeNonRelevant;;0;",
	";1,$;UDUnit;typeAttribute;;0;",
	";1,$;UDNumber;typeAttribute;;0;",
	";1,$;UDTime;typeAttribute;;0;"
};

void iKnow_KB_Label::LoadSpecialLabels(CSV_DataGenerator& kb)
{
	/*
	Set labels = ..SpecialLabelList()
	Set len = $LL(labels)
	For i = 1:1 : len{
	Set label = ..LabelFromString($LI(labels, i))
	Set label.Knowledgebase = kb
	Set sc = ..AddLabelToKB(label, kb)
	$$$IKModelCheck(sc, "Built in label", i, $LI(labels, i))
	}
	*/
	string isDefault = "";
	for (vector<string>::iterator it = special_labels.begin(); it != special_labels.end(); ++it) {
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
			// Do kb.AddToHash(isDefault)
			// Set label.Knowledgebase = kb
			kb.kb_labels.push_back(label); // Set sc = ..AddLabelToKB(label, kb)
			if (isDefault[0]=='1') kb.kb_concept_label = &kb.kb_labels.back(); // If isDefault Set kb.ConceptLabel = label
			// $$$IKModelCheck(sc,stream.Filename,count,line)
		}
		ifs.close();
		return true;
	}
	cerr << "Error opening file: " << labels_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	return false;
}

iKnow_KB_Label iKnow_KB_Label::LabelFromString(vector<string>& row_label, string &isDefault) // ClassMethod LabelFromString(line As %String, ByRef isDefault = "") As Label
{
	iKnow_KB_Label label; // Set label = ..%New()
	label.Name = row_label[3 - 1]; // Set label.Name = $PIECE(line, ";", 3)
	label.Type = row_label[4 - 1]; // Set label.Type = $PIECE(line, ";", 4)
	isDefault = row_label[6 - 1]; // Set isDefault = $PIECE(line, ";", 6)

	if (row_label.size()>7) label.Attributes = row_label[8 - 1]; // Set label.Attributes = $PIECE(line, ";", 8)
	label.PhaseList = row_label[2 - 1]; // Set label.PhaseList = $PIECE(line, ";", 2)
	return label;
}