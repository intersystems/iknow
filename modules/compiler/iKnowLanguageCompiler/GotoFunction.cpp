#include "GotoFunction.h"
#include "LexrepStateOutputFunc.h"
#include "IkStringEncoding.h"
#include "Util.h"
#include "utlExceptionFrom.h"

#include <fstream>
#include <map>
#include <iostream>
#include <math.h>


using namespace iknow::AHO;
using namespace std;
using iknow::base::Char;
using iknow::base::String;

GotoFunction::GotoFunction() : RegexEnabled(false), NextState(1)
{
}


GotoFunction::~GotoFunction()
{
}

#if 0
/// Produces a list of alternatives of the form (a|b|cd)
// list<String> GotoFunction::ParseAlternatives(String& input)
template<typename T>
list<T> GotoFunction::ParseAlternatives(T& input)
{
	size_t len = input.length();
	int depth = 1; //the leading "("
	String token; // = "";
	list<T> tokenList; // Set tokenList = ""
	bool escaped = false; // Set escaped = 0
	int i = 2-1;
	for (; i < (int)len; ++i) {
		T::value_type char_ = input[i]; // Set char = $E(input, i)

		//Process regexp characters
		if (!escaped) {
			if (char_ == '(') {
				depth = depth + 1;
			}
			else {
				if (char_ == ')') {
					depth = depth - 1;
				}
				else {
					if (char_ == '|' && depth == 1) {
						tokenList.push_back(token);
						token.clear(); // = "";
						continue;
					}
					else {
						if (char_ == '\\') {
							escaped = true;
							continue;
						}
					}
				}
			}
		}
		//Add character to current token
		escaped = false;
		if (depth != 0) {
			token = token + char_;
		}
		else {
			tokenList.push_back(token);
			token.clear(); // = "";
			break;
		}
	}
	// if (depth != 0) Throw ##class(%Exception.SystemException).%New("Mismatched Parentheses")
	input = T(input.begin() + i + 1, input.end());
	return tokenList;
}
#endif

// Method ConsumeInput(state As %Integer, input As %String, outputObj As %RegisteredObject, curToken As %String = "", escaping As %Boolean = 0)
void GotoFunction::ConsumeInput(int state, String input, StateOutputFunction *outputObj, String curToken, bool escaping)
{
ConsumeInputStart:

	if (input.length() == 0) {
		vector<vector<Output_Value_Struct>>& StatesGlobal_Output = outputObj->pOutputFunc->StatesGlobal_Output;
		if (state >= StatesGlobal_Output.size()) { // need to resize
			StatesGlobal_Output.resize(StatesGlobal_Output.size() + 1024);
		}
		vector<Output_Value_Struct> &state_map = StatesGlobal_Output[state];
		/*
		std::ofstream ofs = std::ofstream("C:/tmp/output_state1.csv", std::ofstream::app); 
		if (ofs.is_open()) {
			ofs << state << ";" << iknow::base::IkStringEncoding::BaseToUTF8(curToken) << endl;
		}
		ofs.close();
		*/
		Output_Value_Struct IKCValue = outputObj->GetValue(curToken);
		vector<Output_Value_Struct>::iterator IKCKey = state_map.begin(); // Set IKCKey = $ORDER(outputObj.OutputFunc.StatesGlobal("Output", state, ""), 1, IKCEValue)
		while (IKCKey != state_map.end()) { // 	While IKCKey '= "" {	
			std::cout << "double:" << iknow::base::IkStringEncoding::BaseToUTF8(curToken) << endl; // W !, "double["_curToken_"]"
			if (IKCValue == *IKCKey) return; // double, but with equal output, not need to write
			std::cout << "conflicting double:" << iknow::base::IkStringEncoding::BaseToUTF8(curToken) << endl; // W !, "a conflicting double["_IKCEValue_"]" : different output.
			IKCKey++; // Set IKCKey = $ORDER(outputObj.OutputFunc.StatesGlobal("Output", state, IKCKey), 1, IKCEValue)
		}

		// If IKCValue '= $G(IKCEValue) {						
		// size_t ISCKey = state_map.size(); // Set IKCKey = $G(outputObj.OutputFunc.StatesGlobal("Output", state), 0)
		state_map.push_back(IKCValue); // Set outputObj.OutputFunc.StatesGlobal("Output", state, IKCKey) = IKCValue
		if (outputObj->pOutputFunc->MaxState < state) { outputObj->pOutputFunc->MaxState = state; }; //	If outputObj.OutputFunc.MaxState < state Set outputObj.OutputFunc.MaxState = state
		// 	Set outputObj.OutputFunc.StatesGlobal("Output", state) = IKCKey + 1
		return;
		//}
	}
	String remainingInput = String(input.begin() + 1, input.end()); // Set remainingInput = $E(input, 2, *)
	String char_ = String(input.begin(), input.begin()+1); // Set char = $E(input, 1)
	if (char_[0] == '\\' && RegexEnabled) { // If(char = "\") && ..RegexEnabled 
		ConsumeInput(state, remainingInput, outputObj, curToken, true); // Do ..ConsumeInput(state, remainingInput, outputObj, curToken, 1)
		return; // Quit
	}
	if (char_[0] == '(' && RegexEnabled && !escaping) {
		String rest = input;
		list<String> alternatives = ParseAlternatives(rest);
		// size_t len = alternatives.size();
		for (list<String>::iterator it = alternatives.begin(); it != alternatives.end(); ++it) {
			String case_ = *it;
			ConsumeInput(state, case_ + rest, outputObj, curToken);
		}
		return; // Quit
	}
	if (char_[0] == '{' && RegexEnabled && !escaping) {
		size_t posClosing = remainingInput.find('}');
		if (posClosing == String::npos) posClosing = remainingInput.length()-1; // if closing '}' not found...
		String regexName(remainingInput.begin(), remainingInput.begin() + posClosing); // Set regexName = $PIECE(remainingInput, "}", 1)
		//A pseudo-char for the regex
		char_ = Char('{') + regexName + Char('}'); // Set char = "{"_regexName_"}"
		remainingInput = String(remainingInput.begin() + posClosing + 1, remainingInput.end()); // Set remainingInput = $PIECE(input, "}", 2, *)
	}
	
	int nextState = StatesGlobal_GoTo[state][char_]; // Set nextState = $G(..StatesGlobal("GoTo", state, char))
	if (!nextState) { // If 'nextState {	
		nextState = NextState++; // NextState = nextState + 1;

		StatesGlobal_GoTo[state][char_] = nextState; // Set ..StatesGlobal("GoTo", state, char) = nextState
		int IKCdepth = StatesGlobal_Depth[state] + 1; // Set IKCdepth = $G(..StatesGlobal("Depth", state)) + 1
		StatesGlobal_Depth[nextState] = IKCdepth; // Set ..StatesGlobal("Depth", nextState) = IKCdepth
		StatesGlobal_DepthMap.push_back(DepthMap(IKCdepth - 1, state, char_, nextState)); // Set ..StatesGlobal("DepthMap", IKCdepth - 1, state, char, nextState) = ""
	}

	state = nextState, input = remainingInput, curToken = curToken + char_; // Set state = nextState, input = remainingInput, curToken = curToken _ char
	goto ConsumeInputStart; // Goto ConsumeInputStart
}

void GotoFunction::Addword(iknow::base::String word, StateOutputFunction *stateOutputFuncton)
{
	ConsumeInput(0, word, stateOutputFuncton); // Do ..ConsumeInput(0, word, stateOutputFunction)
}

void GotoFunction::ToC(std::string dir)
{
	int maxState = NextState; // Set maxState = ..NextState
	vector<bool> AryZeroStateMap;	// for each state : if true, zero goto states (=leaf)
	AryZeroStateMap.resize(maxState+1, false);  // Kill AryZeroStateMap
	vector<string> AryOneStateMap; // for each state, if one goto state, the char_
	AryOneStateMap.resize(maxState+1); // Kill AryOneStateMap
	vector<size_t> AryNextStateMap; // for each "one goto state", the next state
	AryNextStateMap.resize(maxState+1, 0); // Kill AryNextStateMap
	vector<Char> ArySymbolMap; // Kill ArySymbolMap
	ArySymbolMap.reserve(maxState + 1);
	vector<int> AryTransitionMap; // Kill AryTransitionMap
	AryTransitionMap.reserve(maxState + 1);

	vector<string> AryRegexOneStateMap; //	Kill AryRegexOneStateMap
	AryRegexOneStateMap.resize(maxState + 1);
	vector<int> AryRegexNextStateMap; //	Kill AryRegexNextStateMap
	AryRegexNextStateMap.resize(maxState + 1, 0);
	map<int, int> AryRegexNameMap; //	Kill AryRegexNameMap
	map<int, int> AryRegexTransitionMap; //	Kill AryRegexTransitionMap

	int TotalTransitions = 0; // Set TotalTransitions = 0
	int SymbolMapIndex = 0; //Set SymbolMapIndex = 0
	int RegexMapIndex = 0; // Set RegexMapIndex = 0
	map<String,int> AryRegexId; //	Kill AryRegexId
	//	Kill AryIdRegex
	vector<String> AryIdRegex; //	Set AryIdRegex = 0
	for (int i = 0; i <= maxState; i++) { // For i = 0:1 : maxState{
		//Build a char-code sorted map, as the CharToState
		//values must be sorted.
		map<Char, int> AryCharStateMap; // Kill AryCharStateMap
		// Build a regex map
		map<String, int> AryRegexStateMap; //	Kill AryRegexStateMap
		size_t charCount = 0;	//	Set charCount = 0
		size_t regexCount = 0; //	Set regexCount = 0

		StatesGlobalGoTo_Type::iterator it_state = StatesGlobal_GoTo.find(i); // Set char = $ORDER(..StatesGlobal("GoTo", i, ""),1,nextState)
		if (it_state != StatesGlobal_GoTo.end() && !it_state->second.empty()) { // While char '= "" {
			std::map<String, int>::iterator charnext_it = it_state->second.begin();
			while (charnext_it != it_state->second.end()) {
				String char_ = charnext_it->first;
				int nextState = charnext_it->second;

				TotalTransitions = TotalTransitions + 1; // Set TotalTransitions = TotalTransitions + 1
				//Ordinary characters
				if (char_.length() == 1) { //	If $L(char) = 1 {
					charCount = charCount + 1; // Set charCount = charCount + 1
					AryCharStateMap[char_[0]] = nextState; //	Set AryCharStateMap($A(char)) = nextState
				} else {
					//Regular expression
					regexCount = regexCount + 1; // Set regexCount = regexCount + 1
					//Note the transition
					AryRegexStateMap[char_] = nextState; //  Set AryRegexStateMap(char) = nextState
					//Add an entry to the regex id map if not present
					if (AryRegexId.count(char_) == 0) { // If $G(AryRegexId(char)) = "" {
						int id = (int) AryIdRegex.size(); // Set id = AryIdRegex
						AryRegexId.insert(make_pair(char_,id)); // Set AryRegexId(char) = id
						AryIdRegex.push_back(char_); // Set AryIdRegex(id) = char
						// Set AryIdRegex = id + 1
					}
				}
				charnext_it++; //	Set char = $ORDER(..StatesGlobal("GoTo", i, char), 1, nextState)
			}
		}
		if (charCount == 0) { // end_state (ZeroState : no char to go)
			AryZeroStateMap[i] = true; // Set AryZeroStateMap(i) = 1
			AryOneStateMap[i] = to_string(0); // Set AryOneStateMap(i) = 0
			AryNextStateMap[i] = 0; // Set AryNextStateMap(i) = 0
		}
		else {
			AryZeroStateMap[i] = false; // Set AryZeroStateMap(i) = 0
			if (charCount == 1) { // If(charCount) = 1 { // OneState (one char to go)
				AryOneStateMap[i] = to_string((int)AryCharStateMap.begin()->first); // Set AryOneStateMap(i) = $O(AryCharStateMap(""))
				AryNextStateMap[i] = AryCharStateMap.begin()->second; // Set AryNextStateMap(i) = AryCharStateMap(AryOneStateMap(i))
			}
			else { // Else{
				AryOneStateMap[i] = "kNullSymbol"; // Set AryOneStateMap(i) = "kNullSymbol"
				size_t charCount_20 = charCount << 20; // *pow(2, 20);
				AryNextStateMap[i] = (SymbolMapIndex | charCount_20); // Set AryNextStateMap(i) = ..MakeNextState(SymbolMapIndex, charCount)

				map<Char, int>::iterator it_char = AryCharStateMap.begin(); // Set char = $ORDER(AryCharStateMap(""))
				while (it_char != AryCharStateMap.end()) { // While char '= "" {
					int state = it_char->second; // Set state = AryCharStateMap(char)
					ArySymbolMap.push_back(it_char->first); // Set ArySymbolMap(SymbolMapIndex) = char
					AryTransitionMap.push_back(state); // Set AryTransitionMap(SymbolMapIndex) = state
					SymbolMapIndex = SymbolMapIndex + 1; // Set SymbolMapIndex = SymbolMapIndex + 1
					it_char++; // Set char = $ORDER(AryCharStateMap(char))
				}
			}
		}
		//Just like the characters for the regexes.
		if (regexCount == 0) { // If regexCount = 0{
			AryRegexOneStateMap[i] = "kNullOffset"; // Set AryRegexOneStateMap(i) = "kNullOffset"
			AryRegexNextStateMap[i] = 0; // Set AryRegexNextStateMap(i) = 0
		}
		else { // ElseIf
			if (regexCount == 1) { // ElseIf regexCount = 1 {
				String regex = AryRegexStateMap.begin()->first; // Set regex = $O(AryRegexStateMap(""))
				int regexId = AryRegexId.find(regex)->second; // Set regexId = AryRegexId(regex)
				AryRegexOneStateMap[i] = to_string(regexId + 1); // Set AryRegexOneStateMap(i) = regexId + 1 //So 0 is special
				AryRegexNextStateMap[i] = AryRegexStateMap[regex]; // Set AryRegexNextStateMap(i) = AryRegexStateMap(regex)
			}
			else {
				AryRegexOneStateMap[i] = "kNullOffset"; // Set AryRegexOneStateMap(i) = "kNullOffset"
				AryRegexNextStateMap[i] = (int) (RegexMapIndex | regexCount << 20); // Set AryRegexNextStateMap(i) = ..MakeNextState(RegexMapIndex, regexCount)
				map<String, int>::iterator it_regex = AryRegexStateMap.begin(); // Set regex = $ORDER(AryRegexStateMap(""))
				while (it_regex != AryRegexStateMap.end()) { // While regex '= "" {
					int state = it_regex->second; // Set state = AryRegexStateMap(regex)
					AryRegexNameMap[RegexMapIndex] = AryRegexId[it_regex->first];		// Set AryRegexNameMap(RegexMapIndex) = AryRegexId(regex)
					AryRegexTransitionMap[RegexMapIndex] = state; // Set AryRegexTransitionMap(RegexMapIndex) = state
					RegexMapIndex = RegexMapIndex + 1; // Set RegexMapIndex = RegexMapIndex + 1
					it_regex++; //	Set regex = $ORDER(AryRegexStateMap(regex))
				}
			}
		}
	}
	//Skip writing the Zero state map, no efficiency gain at this time
	{
		OutputToFile ofs(dir, "OneStateMap.inl");  // Do ##class(Util).OutputToFile(dir _ "/OneStateMap.inl")
		if (ofs.IsOpen()) {
			string buf; //	Set buf = ""
			for (int i = 0; i <= maxState; i++) { //	for i = 0 :1 : maxState{
				buf = buf + AryOneStateMap[i] + ",";	// Set buf = buf _ AryOneStateMap(i) _ ","
				if (!(i % 256)) ofs.o() << buf << endl, buf.clear(); // If '(i#256) Write buf,! Set buf = ""
			}
			ofs.o() << buf; //	Write buf
			ofs.o() << "Symbol()" << endl; //	Write "Symbol()", ! //At least one.
		}
	}
	{
		OutputToFile ofs(dir, "NextStateMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/NextStateMap.inl")
		if (ofs.IsOpen()) {
			string buf; //	Set buf = ""
			for (int i = 0; i <= maxState; i++) { //	For i = 0 :1 : maxState{
				//The behavior of large unsigned literals that wouldn't fit inside
				//signed integers is tricky, and some compilers warn about it, so we
				//mark them explicitly unsigned
				buf = buf + to_string(AryNextStateMap[i]) + "U" + ","; // Set buf = buf _ AryNextStateMap(i)_"U"_","
				if (!(i % 256)) ofs.o() << buf << endl, buf.clear(); // If '(i#256) Write buf,! Set buf = ""
			}
			ofs.o() << buf; // Write buf
			ofs.o() << "SmallState()" << endl; // Write "SmallState()", ! //At least one
		}
	}
	{
		OutputToFile ofs(dir, "SymbolMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/SymbolMap.inl")
		if (ofs.IsOpen()) {
			string buf; //	Set buf = ""
			for (int i = 0; i <= SymbolMapIndex - 1; i++) { //	for i = 0 :1 : SymbolMapIndex - 1 {
				buf = buf + to_string((int)ArySymbolMap[i]) + ","; // Set buf = buf _ ArySymbolMap(i)_","
				if (!(i % 256)) ofs.o() << buf << endl, buf.clear();	//	If '(i#256) Write buf,! Set buf = ""
			}
			ofs.o() << buf; // Write buf
			ofs.o() << "Symbol()" << endl; // Write "Symbol()", ! //At least one
		}
	}
	{
		OutputToFile ofs(dir, "StateMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/StateMap.inl")
		if (ofs.IsOpen()) {
			string buf; //	Set buf = ""
			for (int i = 0; i <= SymbolMapIndex - 1; i++) { //	for i = 0 :1 : SymbolMapIndex - 1 {
				buf = buf + to_string(AryTransitionMap[i]) + ","; // Set buf = buf _ AryTransitionMap(i)_","
				if (!(i % 256)) ofs.o() << buf << endl, buf.clear(); //		If '(i#256) Write buf,! Set buf = ""
			}
			ofs.o() << buf; // Write buf
			ofs.o() << "SmallState()" << endl; // Write "SmallState()", ! //At least one
		}
	}
	//Regexes
	if (RegexEnabled) { // If ..RegexEnabled{
		{
			OutputToFile ofs(dir, "RegexObjs.inl"); // Do ##class(Util).OutputToFile(dir _ "/RegexObjs.inl")
			if (ofs.IsOpen()) {
				// map<Char,int> AryRegexId; //	Kill AryRegexId
				for (int i = 0; i <= ((int)AryIdRegex.size() - 1); ++i ) { // For i = 0 :1 : AryIdRegex - 1 {
					String regex = AryIdRegex[i]; // Set regex = AryIdRegex(i)
					String regexName = String(regex.begin() + 1, regex.end() - 1);  //	Set regexName = $E(regex, 2, *-1) //remove {}
					String pattern = RegexDictionary->Lookup(regexName); //	Set pattern = ..RegexDictionary.Lookup(regexName)

					if (pattern.empty()) throw ExceptionFrom<GotoFunction>("Unknown regex specified."); // 	If pattern = "" Throw ##class(%Exception.SystemException).%New("Unknown regex specified.")
					ofs.o() << "static const Char Regex" << i << "Str[] = {"; // Write "static const Char Regex"_i_"Str[] = {"
					for (int j = 1; j <= pattern.length(); j++) {	// For j = 1 :1 : $L(pattern) {
						ofs.o() << static_cast<int>(pattern[j - 1]) << ", "; // W $A(pattern, j)_", "
					}
					ofs.o() << "}; // \"" + iknow::base::IkStringEncoding::BaseToUTF8(pattern) + "\"" << endl; // Write "}; // """_ pattern _ """", !
				}
				ofs.o() << endl; // Write !
			}
		}
		{
			OutputToFile ofs(dir, "RegexMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/RegexMap.inl")
			if (ofs.IsOpen()) {
				// vector<Char> AryIdRegex;
				for (int i = 0; i <= ((int)AryIdRegex.size() - 1); ++i) { // For i = 0 :1 : AryIdRegex - 1 {
					String regex = AryIdRegex[i]; // Set regex = AryIdRegex(i)
					String regexName = String(regex.begin() + 1, regex.end() - 1);	// Set regexName = $E(regex, 2, *-1) //remove {}
					String pattern = RegexDictionary->Lookup(regexName);	// Set pattern = ..RegexDictionary.Lookup(regexName)
					ofs.o() << "Regex(String(Regex" << i << "Str, " << pattern.length() << ")), "; // Write "Regex(String(Regex"_i_"Str, "_$L(pattern)_")), "
					if (!((i + 1) % 4)) ofs.o() << endl; //  	If '(i+1#4) Write !
				}
				ofs.o() << "Regex()" << std::endl; // Write "Regex()", ! //Avoid 0-length array
			}
		}
		{
			OutputToFile ofs(dir, "RegexOneStateMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/RegexOneStateMap.inl")
			if (ofs.IsOpen()) {
				if (AryIdRegex.size() > 0) { // If $G(AryIdRegex) > 0 {
					for (int i = 0; i <= maxState; i++) { // for i = 0:1 : maxState{
						ofs.o() << AryRegexOneStateMap[i] << ", "; // Write AryRegexOneStateMap(i)_", "
						if (!((i + 1) % 4)) ofs.o() << endl; // If '(i+1#4) Write !
					}
				}
				else { // Else{
					ofs.o() << "0"; // Write "0"
				}
				ofs.o() << endl; // Write !
			}
		}
		{
			OutputToFile ofs(dir, "RegexNextStateMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/RegexNextStateMap.inl")
			if (ofs.IsOpen()) {
				if (AryIdRegex.size() > 0) { // If $G(AryIdRegex) > 0 { 
					for (int i = 0; i <= maxState; i++) { // for i = 0:1 : maxState{
						//The behavior of large unsigned literals that wouldn't fit inside
						//signed integers is tricky, and some compilers warn about it, so we
						//mark them explicitly unsigned
						ofs.o() << AryRegexNextStateMap[i] << "U, "; // Write AryRegexNextStateMap(i), "U, "
						if (!((i + 1) % 4)) ofs.o() << endl; // If '(i+1#4) Write !
					}
				}
				else { //Else{
					ofs.o() << "0"; // Write "0"
				}
				ofs.o() << endl; // Write !
			}
		}
		{
			OutputToFile ofs(dir, "RegexOffsetMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/RegexOffsetMap.inl")
			if (ofs.IsOpen()) {
				for (int i = 0; i <= RegexMapIndex - 1; i++) { // for i = 0 :1 : RegexMapIndex - 1 {
					ofs.o() << AryRegexNameMap[i] << ", "; // Write AryRegexNameMap(i), ", "
					if (!((i + 1) % 8)) ofs.o() << endl; //	If '(i+1#8) Write !
				}
				ofs.o() << "0" << endl; // Write "0", ! //avoid 0-sized array
			}
		}
		{
			OutputToFile ofs(dir, "RegexStateMap.inl"); // Do ##class(Util).OutputToFile(dir _ "/RegexStateMap.inl")
			if (ofs.IsOpen()) {
				for (int i = 0; i <= RegexMapIndex - 1; i++) { // for i = 0 :1 : RegexMapIndex - 1 {
					ofs.o() << AryRegexTransitionMap[i] << ", "; // Write AryRegexTransitionMap(i), ", "
					if (!((i + 1) % 8)) ofs.o() << endl; // If '(i+1#8) Write !
				}
				ofs.o() << "0" << endl; // Write "0", ! //avoid 0-sized array		
			}
		}
	}
	// Close $IO
	std::cout << "Total transitions (" << dir << "): " << TotalTransitions << endl; // Write "Total transitions ("_dir_"): " _ TotalTransitions, !
	std::cout << "Total states (" << dir << "): " << NextState << endl; // Write "Total states ("_dir_"): " _ ..NextState, !


}
