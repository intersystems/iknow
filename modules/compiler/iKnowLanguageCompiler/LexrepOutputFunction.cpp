#include "LexrepOutputFunction.h"
#include "Util.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <map>

using namespace iknow::AHO;
using namespace std;

LexrepOutputFunction::LexrepOutputFunction() {}
LexrepOutputFunction::~LexrepOutputFunction() {}

string LexrepOutputFunction::IndexListToOutputList(labels_Type indexList) {
	string out; // Set out = ""
	for (int i = 1; i <= indexList.size(); i++) { // For i = 1:1 : $LL(indexList) {
		out = out + to_string(indexList[i - 1]) + ","; // Set out = out _ $LI(indexList, i) _ ","
	}
	return out;
}

void LexrepOutputFunction::ToC(std::string dir)
{
	map<labels_Type, string> labels_to_labelString; // Set ^ || %IK.UniqueLabelsWordCount(labels) = labelString // Kill ^ || %IK.UniqueLabelsWordCount
	map<labels_Type, map<int, map<string, int>>> IK_UniqueLabelsWordCount;  // Set ^ || %IK.UniqueLabelsWordCount(labels, wordCount, metadata) = ""
	set<string> IK_UniqueMetadata; // Set ^ || %IK.UniqueMetadata(metadata) = ""

	// OutputToFile dbg("C:/tmp", "debug.csv");
	for (int i = 0; i <= MaxState; ++i) { // For i = 0:1 : ..MaxState{
		int count = (StatesGlobal_Output.empty() ? 0 : (int)StatesGlobal_Output[i].size()); // Set count = +$G(..StatesGlobal("Output", i)) // ..GetCount(i)
		// W:count>1 !,"output states : "_count_"]"
		// dbg.o() << i << ";" << count << endl;
		for (int j = 0; j <= count-1; ++j) { // For j = 0:1 : count - 1 {
			Output_Value_Struct match = StatesGlobal_Output[i][j]; // Set match = $G(..StatesGlobal("Output", i, j))
			int wordCount = match.wordCount; // Set wordCount = $LI(match, 1)
			vector<int> &labels = match.Labels; // Set labels = $LI(match, 2)
			string labelString = match.LabelString; // Set labelString = $LI(match, 4)
			string metadata = match.MetaString; // Set metadata = $LI(match, 5) // New, lexrep metadata
			if (metadata.empty()) metadata = " "; // Set:metadata = "" metadata = " "
			//For debugging
			labels_to_labelString[labels] = labelString; // Set ^ || %IK.UniqueLabelsWordCount(labels) = labelString
			IK_UniqueMetadata.insert(metadata); // Set ^ || %IK.UniqueMetadata(metadata) = ""
			IK_UniqueLabelsWordCount[labels][wordCount][metadata] = 0; // Set ^ || %IK.UniqueLabelsWordCount(labels, wordCount, metadata) = ""
		}
	}
	map<labels_Type, int> IK_LabelsToLabelListId; // Kill ^ || %IK.LabelsToLabelListId
	map<string, int> IK_MetaToMetaListId; // Kill ^ || %IK.MetaToMetaListId
	{
		//Output unique label lists
		OutputToFile ofs(dir, "MatchObjs.inl"); // Do ##class(Util).OutputToFile(dir _ "/MatchObjs.inl")
		if (ofs.IsOpen()) {
			map<labels_Type, string>::iterator it_labels = labels_to_labelString.begin(); // Set labels = $O(^ || %IK.UniqueLabelsWordCount(""))
			int idx = 0; // Set idx = 0
			while (it_labels != labels_to_labelString.end()) { // While labels '= "" {
				string labelString = it_labels->second; // Set labelString = ^ || %IK.UniqueLabelsWordCount(labels)
				ofs.o() << "static const LabelId Labels" << idx << "[] = {" << IndexListToOutputList(it_labels->first) << "} /* " << labelString << " */; " << std::endl; // Write "static const LabelId Labels"_idx_"[] = {" _ ..IndexListToOutputList(labels)_"} /* "_labelString_" */; ", !
				IK_LabelsToLabelListId[it_labels->first] = idx; // Set ^ || %IK.LabelsToLabelListId(labels) = idx
				idx = idx + 1; // Set idx = idx + 1
				it_labels++; // Set labels = $O(^ || %IK.UniqueLabelsWordCount(labels))
			}
			ofs.o() << std::endl; // Write !

			if (!IK_UniqueMetadata.empty()) { // If $Data(^ || %IK.UniqueMetadata) { // Only if we have lexrep metadata
				set<string>::iterator it_metadata = IK_UniqueMetadata.begin();
				int idx = 0; // Set idx = 0
				while (it_metadata != IK_UniqueMetadata.end()) { // While metadata '= "" {
					ofs.o() << "static const char* metalist" << idx << " = " << "\"" << *it_metadata << "\";" << std::endl; // Write "static const char* metalist"_idx_" = "_""""_metadata_""";", !
					IK_MetaToMetaListId[*it_metadata] = idx; // Set ^ || %IK.MetaToMetaListId(metadata) = idx
					idx = idx + 1; // Set idx = idx + 1
					it_metadata++; // Set metadata = $O(^ || %IK.UniqueMetadata(metadata))
				}
			}
		}
	}
	//List of unique matches
	map<labels_Type, map<int, map<int, int>>> IK_LabelsWordCountToMatch; // Kill ^ || %IK.LabelsWordCountToMatch
	struct MatchToLabelsWordCount_struct {
		MatchToLabelsWordCount_struct(labels_Type labels_par, int wordCount_par, int metaId_par) : labels(labels_par), wordCount(wordCount_par), metaId(metaId_par) {}
		labels_Type labels; int wordCount; int metaId;
	};
	vector<MatchToLabelsWordCount_struct> IK_MatchToLabelsWordCount; // Kill ^ || %IK.MatchToLabelsWordCount

	int idx = 0; // Set idx = 0
	map<labels_Type, map<int, map<string, int>>>::iterator it_labels = IK_UniqueLabelsWordCount.begin(); // Set labels = $O(^ || %IK.UniqueLabelsWordCount(""))
	while (it_labels != IK_UniqueLabelsWordCount.end()) { // While labels '= "" {
		labels_Type labels = it_labels->first;
		map<int, map<string, int>>::iterator it_wordCount = it_labels->second.begin(); // Set wordCount = $O(^ || %IK.UniqueLabelsWordCount(labels, ""))
		while (it_wordCount != it_labels->second.end()) { // While wordCount '= "" {
			int wordCount = it_wordCount->first;

			map<string, int>::iterator it_metaData = it_wordCount->second.begin(); // Set metaData = $O(^ || %IK.UniqueLabelsWordCount(labels, wordCount, ""))
			while (it_metaData != it_wordCount->second.end()) { // While metaData '= "" {
				string metaData = it_metaData->first;

				int metaId = IK_MetaToMetaListId[metaData]; // Set metaId = ^ || %IK.MetaToMetaListId(metaData)
				IK_LabelsWordCountToMatch[labels][wordCount][metaId] = idx; // Set ^ || %IK.LabelsWordCountToMatch(labels, wordCount, metaId) = idx
				IK_MatchToLabelsWordCount.push_back(MatchToLabelsWordCount_struct(labels, wordCount, metaId)); // Set ^ || %IK.MatchToLabelsWordCount(idx) = $LB(labels, wordCount, metaId)
				
				idx = idx + 1; // Set idx = idx + 1

				it_metaData++; // Set metaData = $O(^ || %IK.UniqueLabelsWordCount(labels, wordCount, metaData))
			}
			it_wordCount++; // Set wordCount = $O(^ || %IK.UniqueLabelsWordCount(labels, wordCount))
		}
		it_labels++; // Set labels = $O(^ || %IK.UniqueLabelsWordCount(labels))
	}

	//Unique sets of matches & metadata
	map<set<int>, int> IK_UniqueMatchSet;  // Kill ^ || %IK.UniqueMatchSet
	vector<set<int>> IK_StateToMachSet; //	Kill ^ || %IK.StateToMatchSet
	IK_StateToMachSet.reserve(MaxState); // output matches per state

	for (int i = 0; i <= MaxState; ++i) { //	For i = 0:1 : ..MaxState{
		//Use an array to sort to minimize distinct sets
		set<int> matchSet; // Kill matchSet
		int count = (StatesGlobal_Output.empty() ? 0 : (int) StatesGlobal_Output[i].size());  // Set count = +$G(..StatesGlobal("Output", i)) // ..GetCount(i)
		// W:count>1 "more matches at "_i,! 
		for (int j = 0; j <= count - 1; ++j) { // For j = 0:1 : count - 1 {
			Output_Value_Struct match = StatesGlobal_Output[i][j]; // Set match = $G(..StatesGlobal("Output", i, j))
			int wordCount = match.wordCount; // Set wordCount = $LI(match, 1)
			labels_Type labels = match.Labels; // Set labels = $LI(match, 2)
			string labelString = match.LabelString; // Set labelString = $LI(match, 4)
			string metadata = match.MetaString; // Set metadata = $LI(match, 5) // New, lexrep metadata
			if (metadata.empty()) metadata = " "; // Set:metadata = "" metadata = " "

			int matchId = IK_LabelsWordCountToMatch[labels][wordCount][IK_MetaToMetaListId[metadata]]; // Set matchId = ^ || %IK.LabelsWordCountToMatch(labels, wordCount, ^ || %IK.MetaToMetaListId(metadata))
			// W:count>1 "mId="_matchId,!
			matchSet.insert(matchId); // Set matchSet(matchId) = ""
		}
		// This basically converts a multidimensional into a list, since we use a C++ set, we can skip this...
		// Set matchList = ""
		// Set matchId = $O(matchSet(""))
		// While matchId '= "" {
		//	Set matchList = $LB(matchId) _ matchList
		//	Set matchId = $O(matchSet(matchId))
		// }
		if (!matchSet.empty()) IK_UniqueMatchSet[matchSet]++; // If $L(matchList) Set ^ || %IK.UniqueMatchSet(matchList) = "" (count unique match sets)
		IK_StateToMachSet.push_back(matchSet); // Set ^ || %IK.StateToMatchSet(i) = matchList (link them to each state)
	}
	//Output unique match sets
	map<set<int>, int> glob_IK_MatchSetToMatchSetIdx;  // Kill ^ || %IK.MatchSetToMatchSetIdx
	int TotalOutputs = 0; // Set TotalOutputs = 0
	{ 
		typedef const set<int> key_set_Type;

		OutputToFile ofs(dir, "Matches.inl"); // Do ##class(Util).OutputToFile(dir _ "/Matches.inl")
		if (ofs.IsOpen()) {
			int idx = 0; // Set idx = 0
			map<set<int>, int>::iterator matchList_it = IK_UniqueMatchSet.begin(); // Set matchList = $O(^ || %IK.UniqueMatchSet(""))
			while (matchList_it != IK_UniqueMatchSet.end()) { // While matchList '= "" {
				key_set_Type &matchList = matchList_it->first; // key is const
				int matchCount = (int) matchList.size(); // Set matchCount = $LL(matchList)
				key_set_Type::iterator matchId_it = matchList.begin();
				while (matchId_it != matchList.end()) { // For i = 1:1 : matchCount{
					TotalOutputs = TotalOutputs + 1; // Set TotalOutputs = TotalOutputs + 1
					int matchId = *matchId_it; // Set matchId = $LI(matchList, i)
					MatchToLabelsWordCount_struct data = IK_MatchToLabelsWordCount[matchId]; // Set data = ^ || %IK.MatchToLabelsWordCount(matchId)
					labels_Type labels = data.labels; // Set labels = $LI(data, 1)
					int wordCount = data.wordCount; // Set wordCount = $LI(data, 2)
					int metaId = data.metaId; // Set metaId = $LI(data, 3)
					size_t labelCount = labels.size();  // Set labelCount = $LL(labels)
					int labelIdx = IK_LabelsToLabelListId[labels]; // Set labelIdx = ^ || %IK.LabelsToLabelListId(labels)
					//TODO: Eliminate duplicates in this list.
					if (metaId == 0) { // If metaId = 0{ Write "Match(&(Labels"_labelIdx_"[0]),"_ labelCount _","_wordCount_")," }
						ofs.o() << "Match(&(Labels" << labelIdx << "[0])," << labelCount << "," << wordCount << "),";
					}
					else { // Else{ Write "Match(&(Labels"_labelIdx_"[0]),"_ labelCount _","_wordCount_", metalist"_metaId_")," }
						ofs.o() << "Match(&(Labels" << labelIdx << "[0])," << labelCount << "," << wordCount << ", metalist" << metaId << "),";
					}
					matchId_it++;
				}
				ofs.o() << char(9) << "/* " << idx << " */" << std::endl; // Write $C(9) _ "/* "_idx_" */", !
				glob_IK_MatchSetToMatchSetIdx[matchList] = idx; // Set ^ || %IK.MatchSetToMatchSetIdx(matchList) = idx
				idx = idx + matchCount; // Set idx = idx + matchCount
				matchList_it++; // Set matchList = $O(^ || %IK.UniqueMatchSet(matchList))
			}
			ofs.o() << "Match()" << std::endl; // Write "Match()", ! //At least one
		}
	}
	//Output match set table
	{
		OutputToFile ofs(dir, "MatchSet.inl");  // Do ##class(Util).OutputToFile(dir _ "/MatchSet.inl")
		if (ofs.IsOpen()) {
			string buf; //	Set buf = ""
			for (int i = 0; i <= MaxState; i++) { //	For i = 0 :1 : ..MaxState{
				// vector<set<int>> glob_IK_StateToMachSet;
				set<int> matchList = IK_StateToMachSet[i]; // Set matchList = ^ || %IK.StateToMatchSet(i)
				if (matchList.empty()) { // If matchList = "" {
					buf = buf + "0" + ","; // Set buf = buf _ 0 _ ","
				}
				else { // Else{
					buf = buf + to_string(glob_IK_MatchSetToMatchSetIdx[matchList]) + ","; // Set buf = buf _ ^ || %IK.MatchSetToMatchSetIdx(matchList) _ ","
				}
				//Old debug output
				//Set buf = buf _ $C(9)_" /* "_i_" */"
				//Set buf = buf_ ","_$C(13,10)
				if (!(i % 256)) ofs.o() << buf << std::endl, buf.clear(); // If '(i#256) Write buf,! Set buf = ""
			}
			ofs.o() << buf; // Write buf Set buf = ""
			ofs.o() << std::endl << "SmallOffset()" << std::endl; // Write !, "SmallOffset()", ! //At least one
		}
	}
	//Output match count table
	int TotalOutputStates = 0;	//	Set TotalOutputStates = 0
	{
		OutputToFile ofs(dir, "MatchCount.inl");  // Do ##class(Util).OutputToFile(dir _ "/MatchCount.inl")
		if (ofs.IsOpen()) {
			string buf; //	Set buf = ""
			for (int i = 0; i <= MaxState; ++i) { //	For i = 0 :1 : ..MaxState{
				// Not ..GetCount(i): Must be same count of functionally unique matches we used
				// when building the unique match set. Things like the label names may be different
				// even when the resolved label ids are identical.
				int count = (int) IK_StateToMachSet[i].size(); // Set count = +$LL(^ || %IK.StateToMatchSet(i))
				if (count > 0) TotalOutputStates = TotalOutputStates + 1; // If count > 0 Set TotalOutputStates = TotalOutputStates + 1
				buf = buf + to_string(count) + ","; // Set buf = buf _ count _ ","
				//For j=0:1:count - 1 {
				//	Set token = $LI($G(..StatesGlobal("Output", i, j)), 3)
				//	Set buf = buf _ "<"_token_"> "
				//}
				//Set buf = buf _ "*/,"_$C(13,10)
				if (!(i % 256)) ofs.o() << buf << std::endl, buf.clear();  // If '(i#256) Write buf,! Set buf = ""		
			}
			ofs.o() << buf; // Write buf
			ofs.o() << endl << "SmallCount()" << std::endl; // Write !, "SmallCount()", ! //At least one
		}
	}
	cout << "Total outputs (" << dir << "): " << TotalOutputs << endl; // Write "Total outputs ("_dir_"): " _ TotalOutputs, !
	cout << "Total output states (" << dir << "): " << TotalOutputStates << endl; // Write "Total output states ("_dir_"): "_ TotalOutputStates, !
}