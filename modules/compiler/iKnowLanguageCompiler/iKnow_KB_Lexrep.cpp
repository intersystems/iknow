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
}
Do kb.AddToHash(token)
Do kb.AddToHash(labels)
Do kb.AddToHash(meta)
}
kill:$$$IKISDEVBUILD ^lexreps(kb.Name) // temporary storage for double lexrep checking
// Set:$$$IKISDEVBUILD hasUpper = ""
}
*/
bool IsNumeric(iknow::base::Char token) { //	Method IsNumeric(token As %Char) As %Boolean
	return ((int)token >= 0x0030) && ((int)token <= 0x0039);  // Quit:(($Ascii(token)'<$ZHEX("0030")) & ($Ascii(token)'>$ZHEX("0039"))) 1 // '0' to '9'
}
bool IsKatakana(iknow::base::Char token) { // IsKatakana(token As %Char) As %Boolean
	return ((int)token >= 0x30A0) && ((int)token <= 0x30FF); // Quit:(($Ascii(token)'<$ZHEX("30A0")) & ($Ascii(token)'>$ZHEX("30FF"))) 1
}

inline void add_if_not_empty(iknow::base::String& token, vector<string>& lst) {
	if (token.empty())
		return;
	lst.push_back(iknow::base::IkStringEncoding::BaseToUTF8(token));
	token.clear();
}
vector<string> CollectSegments(string& token, const string& labels, CSV_DataGenerator& kb)
{
	if (labels.find('-') == string::npos) // token is *not* segmented.
		return vector<string>();

	vector<string> lstToken, token_segments;
	string separator(" "); // classic token separator

	if (kb.GetName() == "ja") { // Japanese style
		separator.clear(); // no separator
		iknow::base::String StringKana, StringNumeric;

		iknow::base::String wToken = iknow::base::IkStringEncoding::UTF8ToBase(token);
		for (int cntToken = 1; cntToken <= wToken.length(); cntToken++) {
			iknow::base::Char symbol = wToken[cntToken - 1];
			if (IsNumeric(symbol)) {
				add_if_not_empty(StringKana, lstToken);
				StringNumeric += symbol;
				continue;
			}
			if (IsKatakana(symbol)) {
				add_if_not_empty(StringNumeric, lstToken);
				StringKana += symbol;
				continue;
			}
			add_if_not_empty(StringKana, lstToken);
			add_if_not_empty(StringNumeric, lstToken);
			iknow::base::Char StrSymbol[2] = { symbol, '\0' };
			iknow::base::String Symbol(StrSymbol);
			add_if_not_empty(Symbol, lstToken);
		}
		add_if_not_empty(StringKana, lstToken);
		add_if_not_empty(StringNumeric, lstToken);
	}
	else { // single tokens are space separated, except for ideographical languages
		lstToken = kb.split_row(token, ' ');
	}

	// collect label segments
	vector<string> label_segments, list_labels = kb.split_row(labels, ';');
	string label_segment;
	for (auto it = list_labels.begin(); it != list_labels.end(); it++) {
		if (*it == "-") { // segment splitter
			label_segments.push_back(label_segment);
			label_segment.clear();
			continue;
		} 
		label_segment += (*it + ";");
	}
	label_segments.push_back(label_segment);

	int idx_single_token_segments = 0;
	for (int i = 0; i < (int)label_segments.size(); ++i) {
		string& label_segment = label_segments[i];
		token_segments.push_back(lstToken[idx_single_token_segments++]);
		size_t join_index = label_segment.find(";Join;");
		while (join_index != string::npos) {
			token_segments.back() += (separator + lstToken[idx_single_token_segments++]);
			join_index = label_segment.find(";Join;", join_index + 5);
		}
		// cout << "seg=\"" << token_segments.back() << "\"" << endl;
	}
	if (label_segments.size() != token_segments.size())
		throw ExceptionFrom<iKnow_KB_Lexrep>("While reading lexreps : Mismatch between token and label segments:\"" + token + "\":\"" + labels + "\"");

	return token_segments;
}

bool iKnow_KB_Lexrep::ImportFromCSV(string lexreps_csv, CSV_DataGenerator& kb)
{
	kb.kb_lexreps.clear();
	kb.lexrep_index.clear();
	kb.lexrep_segments_index.clear();

	cout << "Reading lexrep data..." << endl;

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

			// collect token segments (splitted by '-' label)
			vector<string> token_segments = CollectSegments(token, labels, kb);
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
				int idx_lexrep = (int)kb.kb_lexreps.size(); // index in lexrep vector for fast retral
				kb.lexrep_index[lexrep.Token] = idx_lexrep;
				if (token_segments.size()) { // collect the token segments
					int idx_segment = 0; // segment index 
					for_each(token_segments.begin(), token_segments.end(), [idx_lexrep, &kb, &idx_segment](string& token_segment) { kb.lexrep_segments_index[token_segment].push_back(idx_lexrep); kb.lexrep_segments_index[token_segment].push_back(idx_segment++); });
				}
				kb.kb_lexreps.push_back(lexrep); // Set lexrep.Knowledgebase = kb
			}
			if (!(kb.kb_lexreps.size() % 2048)) cout << char(9) << kb.kb_lexreps.size();
		}
		ifs.close();
		return true;
	}
	cerr << "Error opening file: " << lexreps_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	return false;
}