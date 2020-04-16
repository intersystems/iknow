#include "CSV_DataGenerator.h"

#include <fstream>
#include <iostream>
#include <list>
#include <algorithm>

using namespace iknow::csvdata;
using namespace std;

static void AddToken(string token, vector<string>& tokens)
{
	tokens.push_back(token);
}

static void ConsumeInput(string input, string curToken, vector<string>& tokens, bool escaping = false)
{
	if (input.length() == 0) {
		AddToken(curToken, tokens);
		return;
	}
	char char_ = input[0]; // $E(input, 1)
	if (char_ == '\\') {
		string rest(input.begin() + 1, input.end());
		ConsumeInput(rest, curToken, tokens, true);
		return;
	}
	if (char_ == '(' && !escaping) {
		string rest = input;
		list<string> alternatives = ParseAlternatives(rest);
		for (list<string>::iterator it = alternatives.begin(); it != alternatives.end(); ++it) {
			string case_ = *it; // Set case = $LI(alternatives, i)
			ConsumeInput(case_ + rest, curToken, tokens);
		}
		return;
	}
	ConsumeInput(string(input.begin() + 1, input.end()), curToken + char_, tokens);
}

static void EnumerateTokens(string token, vector<string>& tokens)
{
	tokens.clear();
	ConsumeInput(token, "", tokens);
}

/*
ClassMethod ImportFromStream(stream As %CharacterStream, kb As Knowledgebase)
{
kill:$$$IKISDEVBUILD ^lexreps(kb.Name) // temporary storage for double lexrep checking
Set count = 0
While ('stream.AtEnd) {
Set count = count + 1
Set line = $ZCONVERT(stream.ReadLine(), "O", "UTF8")
Continue:$Find(line,"/*")'=0 // comment line
Continue:$Length(line)<2 // empty line
If ($$$IKISDEVBUILD) { // Verify the minimum number of label parameters
throw:($Length(line,";") < 5) ##class(%Exception.General).%New("Syntax Error in line "_count_": "_$ZCONVERT(line,"I","UTF8")_"""")
}
Continue:($L(line,";") < 5) // if not in development, just pass...

Set meta = $Piece(line, ";", 2) // new !, metadata linked to lexrep
Set token = $PIECE(line, ";", 3)
Set labels = $PIECE(line, ";", 5, 99)
If ($$$IKISDEVBUILD) { // Verify the labels
Set tokenLength = $length(token, " ")
Set labelCount = $Length(labels, ";")
Set labelSegmentCount=1
Set labelRealCount=0
For itLabel=1:1:labelCount {
Set labelName=$Piece(labels, ";", itLabel)
If labelName="" Continue
If labelName="-" {
throw:labelRealCount=0 ##class(%Exception.General).%New(kb.Name_":segment label(""-"") can not start the label sequence ! (line:"_count_") "_line)
Set labelSegmentCount=labelSegmentCount+1 } // Count Segments
Else {
Set labelRealCount=labelRealCount+1
Continue:$Extract(labelName,1,4)="Lit_" // Dont check on literal labels
Continue:labelName="SBegin" // fixed label added after loading rules
Continue:labelName="SEnd" // idem
throw:'$Data(^labelname(kb.Name,labelName)) ##class(%Exception.General).%New(kb.Name_":Non Existing Label: "_$ZCONVERT(labelName,"I","UTF8")_" In line ("_count_"): "_$ZCONVERT(line,"I","UTF8"))
}
}
throw:labelRealCount=0 ##class(%Exception.General).%New(kb.Name_":Syntax Error, No Valid labels in line("_count_"):"_$ZCONVERT(line,"I","UTF8"))
}
If 'kb.RegexpEnabled {
Set lexrep = ..%New()
Set lexrep.Token = token
Set lexrep.Meta = meta
Set lexrep.Labels = labels
Set lexrep.Knowledgebase = kb
Set sc = lexrep.%Save()
$$$IKModelCheck(sc,stream.Filename,count,line)
}
Else {
Do ..EnumerateTokens(token, .tokens)
For i=1:1:tokens {
Set lexrep = ..%New()
Set lexrep.Token = tokens(i)
Set lexrep.Meta = meta
Set lexrep.Labels = labels
Set lexrep.Knowledgebase = kb
Set sc = lexrep.%Save()
$$$IKModelCheck(sc,stream.Filename,count,line)
}
}
Do kb.AddToHash(token)
Do kb.AddToHash(labels)
Do kb.AddToHash(meta)
}
kill:$$$IKISDEVBUILD ^lexreps(kb.Name) // temporary storage for double lexrep checking
// Set:$$$IKISDEVBUILD hasUpper = ""
}
*/
void iKnow_KB_Lexrep::ImportFromCSV(string lexreps_csv, CSV_DataGenerator& kb)
{
	ifstream ifs = ifstream(lexreps_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);

		int count = 0;
		for (string line; getline(ifs, line);)
		{
			++count;
			if (line.find("/*") != string::npos) continue; // Continue:$Find(line, "/*")'=0 // comment line
			if ((std::count(line.begin(), line.end(), ';') + 1) < 5) continue; // Continue:($L(line,";") < 5) // if not in development, just pass...
			vector<string> row_lexrep = kb.split_row(line);

			string meta = row_lexrep[2 - 1]; // Set meta = $Piece(line, ";", 2) // new !, metadata linked to lexrep
			string token = row_lexrep[3 - 1]; // Set token = $PIECE(line, ";", 3)
			string labels;
			for_each(row_lexrep.begin() + 5 - 1, row_lexrep.end(), [&labels](std::string& label) { labels += label + ";"; }); // Set labels = $PIECE(line, ";", 5, 99)

			/*
			std::string::reverse_iterator rit = labels.rbegin();
			while (rit != labels.rend() && *rit == ';') ++rit;
			labels.erase(rit.base(), labels.end()); // remove ending ';' characters
			*/
			vector<string> tokens;
			EnumerateTokens(token, tokens); // Do ..EnumerateTokens(token, .tokens)
			for (int i = 0; i < tokens.size(); ++i) { // For i = 1:1 : tokens{
				iKnow_KB_Lexrep lexrep; // Set lexrep = ..%New()
				lexrep.Token = tokens[i]; // Set lexrep.Token = tokens(i)
				lexrep.Meta = meta; // Set lexrep.Meta = meta
				lexrep.Labels = labels; // Set lexrep.Labels = labels
				kb.lexrep_index[lexrep.Token] = (int) kb.kb_lexreps.size(); // index on Token for fast retrieval
				kb.kb_lexreps.push_back(lexrep); // Set lexrep.Knowledgebase = kb
			}
		}
	}
	else {
		cerr << "Error opening file: " << lexreps_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	}
	ifs.close();
}

iKnow_KB_Lexrep::iKnow_KB_Lexrep()
{
}


iKnow_KB_Lexrep::~iKnow_KB_Lexrep()
{
}
