#include "CSV_DataGenerator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>

using namespace iknow::csvdata;
using namespace std;

template <typename T>
T StringToNumber(const string& Text)
{
	istringstream ss(Text);
	T result;
	return ss >> result ? result : 0;
}

void str_subsitute(string& text, const string str_find, const string str_replace)
{
	string::size_type n = 0;
	while ((n = text.find(str_find, n)) != std::string::npos)
	{
		text.replace(n, str_find.size(), str_replace);
		n += str_replace.size();
	}
}

bool iKnow_KB_Rule::ImportFromCSV(string rules_csv, CSV_DataGenerator& kb)
{
	ifstream ifs = ifstream(rules_csv, ifstream::in);
	if (ifs.is_open()) {
		kb.handle_UTF8_BOM(ifs);

		newLabelsIndex_type newLabelsIndex; // mapping new labels 
		newLabels_type newLabels; // mapping labelName and phase to new label
		SPhases_type SBeginPhases, SEndPhases; // collector for SBegin/SEnd phases
		int max_rule_csv_id = 0; // store the maximum rule id, to be able to generate a new, unique, one.
		vector<string> vec_rules_csv;
		set<int> rule_rewrite_indexes;
		for (string line; getline(ifs, line);)
		{
			vec_rules_csv.push_back(line); // store in case we need to rewrite with new id's
			if (line.find("/*") != string::npos) continue; // Continue:$Find(line, "/*")'=0 // comment line
			if ((std::count(line.begin(), line.end(), ';') + 1) < 4) continue; // Continue : ($L(line, ";") < 4)

			vector<string> row_rule = kb.split_row(line);
			iKnow_KB_Rule rule; // Set rule = ..%New()
			rule.csv_id = row_rule[1 - 1]; // csv identification of rule
			int rule_csv_id = StringToNumber<int>(rule.csv_id);
			if (rule_csv_id == 0) { // need a new id
				string& last_line = vec_rules_csv.back();

				string quot1("\xe2\x80\x9d"), quot2("\xe2\x80\x9c"); // rewrite exotic (”,“) qoutes.
				str_subsitute(last_line, quot1, "\"");
				str_subsitute(last_line, quot2, "\"");

				rule_rewrite_indexes.insert((int)vec_rules_csv.size() - 1); // keep the index

				row_rule = kb.split_row(last_line);
			}
			else {
				if (rule_csv_id > max_rule_csv_id) // store the bigger one
					max_rule_csv_id = rule_csv_id;
			}
			rule.Phase = row_rule[2 - 1]; // Set phase = $PIECE(line, ";", 2)	// Set rule.Phase = phase
			rule.InputPattern = rule.TransformRulePattern(row_rule[3 - 1], rule.Phase, kb, newLabels, newLabelsIndex, SBeginPhases, SEndPhases); // Set rule.InputPattern = ..TransformRulePattern($PIECE(line, ";", 3), phase, kb, .newLabels, .newLabelsIndex, .SBeginPhases, .SEndPhases)
			rule.OutputPattern = rule.TransformRulePattern(row_rule[4 - 1], rule.Phase, kb, newLabels, newLabelsIndex, SBeginPhases, SEndPhases); // Set rule.OutputPattern = ..TransformRulePattern($PIECE(line, ";", 4), phase, kb, .newLabels, .newLabelsIndex, .SBeginPhases, .SEndPhases)
			/*
			If($$$IKISDEVBUILD) { // read extra data fields for linguistic tracing
			Set rule.IdCSV = $PIECE(line, ";", 1)
			Set rule.Comment = $PIECE(line, ";", 5) // a little confusion, comments can be on position 5 or 6
			Set rule.Comment = rule.Comment_$PIECE(line, ";", 6)
			}
			//The precedence value used to be taken from the 5th field of the csv but
			//it's simpler and more convenient to just take rules in order, adding suitable
			//high order bits for phase. Phase and precedence are combined in a single
			//integer field to make iteration easier from the C++ interface, which likes
			//using integer keys.
			Set:rule.Phase = "$" rule.Phase = 99 // prevent last_phase rules to be selected first
			Set rule.Precedence = (rule.Phase * 1000000) + count
			Do kb.AddToHash(rule.InputPattern)
			Do kb.AddToHash(rule.OutputPattern)
			Do kb.AddToHash(rule.Precedence)
			Do kb.AddToHash(rule.Phase)
			Set rule.Knowledgebase = kb
			Set sc = rule.%Save()
			*/
			if (rule.Phase == "$") rule.Phase = "99"; // prevent last_phase rules to be selected first
			rule.Precedence = (std::stoi(rule.Phase) * 100000) + (int)vec_rules_csv.size(); // sort on phasenumbers and count = appearance in file.

			kb.kb_rules.push_back(rule);
		}
		ifs.close(); // done reading
		if (rule_rewrite_indexes.size()) { // generate new indexes and rewrite...
			std::ofstream ofs = std::ofstream(rules_csv, ofstream::out);
			if (ofs.is_open()) {
				for (int idx = 0; idx < vec_rules_csv.size(); ++idx) {
					string& line = vec_rules_csv[idx];

					if (rule_rewrite_indexes.count(idx)) { // needs rewrite
						max_rule_csv_id++; // next index
						ofs << max_rule_csv_id << line << endl;
					}
					else { // copy line
						ofs << line << endl;
					}
				}
				ofs.close();
			}
		}
		// sort kb_rules on Precedence
		std::sort(kb.kb_rules.begin(), kb.kb_rules.end(), [](iKnow_KB_Rule const& a, iKnow_KB_Rule const& b) { return a.Precedence < b.Precedence;  });

		// typedef std::vector<std::pair<std::string, std::string> > newLabels_type;
		for (newLabels_type::iterator itLabel = newLabels.begin(); itLabel != newLabels.end(); ++itLabel) // Set key = $ORDER(newLabels(""))	// While key '= "" {	// Set key = $ORDER(newLabels(key))
		{
			iKnow_KB_Label labelObj; // Set labelObj = ##class(Label).%New()
			labelObj.Name = itLabel->first; // Set labelObj.Name = $List(newLabels(key), 1)
			// Set labelObj.Knowledgebase = kb
			labelObj.Type = "typeLiteral"; // Set labelObj.Type = "typeLiteral"
			labelObj.Attributes = ""; // Set labelObj.Attributes = ""
			labelObj.PhaseList = itLabel->second + ",$"; // Set labelObj.PhaseList = $List(newLabels(key), 2) _ ",$" // add last phase
			kb.kb_labels.push_back(labelObj); // Set sc = labelObj.%Save()
			// If 'sc throw ##class(%Exception.StatusException).CreateFromStatus(sc)
		}
		// SPhases_type SBeginPhases, SEndPhases; // collector for SBegin/SEnd phases
		iKnow_KB_Label SBeginObj; // Set labelObj = ##class(Label).%New()
		SBeginObj.Name = "SBegin"; // Set labelObj.Name = "SBegin"
		// Set labelObj.Knowledgebase = kb
		SBeginObj.Type = "typeAttribute"; // Set labelObj.Type = "typeAttribute"
		SBeginObj.Attributes = ""; // Set labelObj.Attributes = ""
		// Set phaselist = ""
		for_each(SBeginPhases.begin(), SBeginPhases.end(), [&SBeginObj](string phase) { SBeginObj.PhaseList += (SBeginObj.PhaseList.size() ? "," + phase : phase); }); // For idxPhases = $listlength(SBeginPhases) :-1 : 1 Set phaselist = $list(SBeginPhases, idxPhases)_$Select($Length(phaselist) : ","_phaselist, 1 : "")
		// Set labelObj.PhaseList = phaselist
		kb.kb_labels.push_back(SBeginObj); // Set sc = labelObj.%Save()
		// If 'sc throw ##class(%Exception.StatusException).CreateFromStatus(sc)

		iKnow_KB_Label SEndObj;
		SEndObj.Name = "SEnd";
		SEndObj.Type = "typeAttribute";
		SEndObj.Attributes = "";
		for_each(SEndPhases.begin(), SEndPhases.end(), [&SEndObj](string phase) { SEndObj.PhaseList += (SEndObj.PhaseList.size() ? "," + phase : phase); });
		kb.kb_labels.push_back(SEndObj);
		return true;
	}
	cerr << "Error opening file: " << rules_csv << " Language=\"" << kb.GetName() << "\"" << endl;
	return false;
}

void AddLabelToLexrep(CSV_DataGenerator& kb, string& token, string& label)
{
	// Full lexrep addition
	auto it_lexrep = kb.lexrep_index.find(token); // &sql(select ID into : lexrepId from Lexrep where Token = : token and Knowledgebase = : kbId)
	if (it_lexrep != kb.lexrep_index.end()) { // If(SQLCODE = 0) && lexrepId{
		int lexrep_index = it_lexrep->second;
		iKnow_KB_Lexrep &lexrep = kb.kb_lexreps[lexrep_index]; // Set lexrep = ##class(Lexrep).%OpenId(lexrepId)
		string separator = (*(lexrep.Labels.end() - 1) == ';' ? "" : ";"); // Set separator = $select($extract(lexrep.Labels, *) = ";":"", 1 : ";") // label separator
		if (lexrep.Labels.find(label) == string::npos)
			lexrep.Labels = lexrep.Labels + separator + label + ";"; // // If(lexrep.Labels '[ label) Set lexrep.Labels = lexrep.Labels _ separator _ label _ ";" // check for lexrep.Labels ending '; '
	}
	else { // new
		iKnow_KB_Lexrep lexrep; // Set lexrep = ##class(Lexrep).%New()
		lexrep.Token = token; // Set lexrep.Token = token
		lexrep.Labels = label + ";"; // Set lexrep.Labels = label _ ";"
		kb.lexrep_index[lexrep.Token] = (int) kb.kb_lexreps.size(); // new index for lexrep
		kb.kb_lexreps.push_back(lexrep); // Set sc = lexrep.%Save()
	}
	// Lexrep segment addition
	auto it_lexrep_segment = kb.lexrep_segments_index.find(token);
	if (it_lexrep_segment != kb.lexrep_segments_index.end()) { // token represents a lexrep segment
		vector<int>& segment_indexes = kb.lexrep_segments_index[token];

		for (auto it_index = segment_indexes.begin(); it_index != segment_indexes.end(); ++it_index) {
			int lexrep_index = *it_index++;
			int segment_index = *it_index;

			iKnow_KB_Lexrep& lexrep = kb.kb_lexreps[lexrep_index]; // Set lexrep = ##class(Lexrep).%OpenId(lexrepId)
			vector<string> label_segments, list_labels = kb.split_row(lexrep.Labels, ';');
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

			string& token_label_segment = label_segments[segment_index];
			if (token_label_segment.find(label) == string::npos) { // add label if not present
				token_label_segment += label + ";";
			}
			string labels_update;
			for (auto it = label_segments.begin(); it < label_segments.end(); ++it) {
				labels_update += *it;
				if (it < label_segments.end() - 1)
					labels_update += "-;";
			}
			// for_each(label_segments.begin(), label_segments.end(), [&labels_update](string& label_segment) { labels_update += label_segment + "-"; });
			lexrep.Labels = labels_update; // updated with literal label
		}
	}
	// kb.lexrep_segments_index[token_segment].push_back(idx_lexrep)
}

std::string iKnow_KB_Rule::TransformRulePattern(string& pattern, string& phase, CSV_DataGenerator& kb, newLabels_type &newLabels, newLabelsIndex_type &newLabelsIndex, SPhases_type& SBeginPhases, SPhases_type& SEndPhases)
{
	// Set len = $L(pattern, "|")
	vector<string> labels = kb.split_row(pattern, '|');
	// size_t len = labels.size();
	for (vector<string>::iterator it = labels.begin(); it != labels.end(); ++it) {
		string& label = *it;

		if (label.find("SBegin") != string::npos) SBeginPhases.insert(phase); // add rule phase to SBegin label
		if (label.find("SEnd") != string::npos) SEndPhases.insert(phase); // add rule phase to SEnd label

		string newLabel;
		int lastQuote = 0;
		for (string::iterator ic = label.begin(); ic != label.end(); ++ic) { // scan the label selector
			char c = *ic;
			if (lastQuote) {
				if (c == '\"') {
					string token(label.begin() + lastQuote, ic); // Set token = $E(label, lastQuote + 1, j - 1)
					string labelName = "Lit_" + token; // labelname constructed out of token = literal label
					str_subsitute(labelName, string(":"), string("DoublePoints")); // Set labelName = "Lit_" _ $replace($replace($replace(token, ":", "DoublePoints"), "+", "plus"), "(", "lbrack") // Labels cannot contain the or ':' or '+' symbol
					str_subsitute(labelName, string("+"), string("plus"));
					str_subsitute(labelName, string("("), string("lbrack"));
					
					if (newLabelsIndex.find(labelName) != newLabelsIndex.end()) { // If $Data(newLabelsIndex(labelName)) { // Literal label exist already
						int idxLabel = newLabelsIndex[labelName]; // Set idxLabel = newLabelsIndex(labelName)
						string rulePhases = newLabels[idxLabel].second; // Set rulePhases = $List(newLabels(idxLabel), 2)
						if (rulePhases.find(phase) == string::npos) // Set:rulePhases'[phase $List(newLabels(idxLabel),2) = rulePhases_","_phase
							rulePhases += ("," + phase); 
						newLabels[idxLabel].second = rulePhases; // overwrite
					}
					else { // New label
						newLabels.push_back(make_pair(labelName, phase)); // Set newLabels($Increment(newLabels)) = $Lb(labelName, phase) // store name using a numeric key to avoid collision problems
						newLabelsIndex.insert(make_pair(labelName, static_cast<int>(newLabels.size() - 1))); // Set newLabelsIndex(labelName) = newLabels
					}

					AddLabelToLexrep(kb, token, labelName); // Do ..AddLabelToLexrep(kb, token, labelName)
					//Rewrite label pattern string
					newLabel = newLabel + labelName; // Set newLabel = newLabel _ labelName
					lastQuote = 0; // Set lastQuote = 0
				}
			}
			else {
				if (c == '\"')
					lastQuote = static_cast<int>(ic - label.begin()) + 1;
				else
					newLabel += *ic;
			}
		}
		label = newLabel; // overwrite csv-label
	}
	string transformed_pattern;
	for (vector<string>::iterator it = labels.begin(); it != labels.end(); ++it) { // reconstruct output
		if (it != labels.begin()) transformed_pattern += "|";
		transformed_pattern += *it;
	}
	return transformed_pattern;
}