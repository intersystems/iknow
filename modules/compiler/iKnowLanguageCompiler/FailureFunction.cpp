#include "FailureFunction.h"
#include "LexrepOutputFunction.h"
#include <algorithm>
#include <fstream>

using namespace std;
using namespace iknow::AHO;
using namespace iknow::base;

FailureFunction::FailureFunction() {}
FailureFunction::~FailureFunction() {}

FailureFunction* FailureFunction::Create(GotoFunction* gotoFunc, OutputFunction* outputFunc_par, bool is_ideographic) // ClassMethod Create(gotoFunc As GotoFunction, outputFunc As OutputFunction) As FailureFunction
{
	LexrepOutputFunction* outputFunc = static_cast<LexrepOutputFunction*>(outputFunc_par); // cast to LexrepOutputFunction*

	FailureFunction* failFunc = new FailureFunction; // Set failFunc = ..%New()
	failFunc->MaxState = outputFunc->MaxState; // Set failFunc.MaxState = outputFunc.MaxState

	/*
	Set depth = ""
	Set state = ""
	Set input = ""
	Set nextState = ""
	Do gotoFunc.DepthOrder(.depth, .state, .input, .nextState)
	*/
	// now sort on depth,state,input,nextstate
	std::sort(gotoFunc->StatesGlobal_DepthMap.begin(), gotoFunc->StatesGlobal_DepthMap.end(), DepthMap());
	for (GotoFunction::DepthMap_Type::iterator it = gotoFunc->StatesGlobal_DepthMap.begin(); it != gotoFunc->StatesGlobal_DepthMap.end(); ++it) { // While(depth || state || input || nextState) {
		int depth = it->depth;
		int state = it->state;
		String input = it->input;
		if (is_ideographic && ((int)input[0] >= 0x0030 && (int)input[0] <= 0x0039)) // Numericals are handled atomically, no merging !
			continue;
		int nextState = it->nextstate;

		int failState = 0; // Set failState = 0
		if (depth != 0) { // If depth '= 0 {
			int NoFailureFunction = 0;
			if (outputFunc->GetCount(nextState)) { // output is defined
				Output_Value_Struct Value = outputFunc->Get(nextState, 0);
				/*
				If $Listvalid(Value) { // Value is $List
					If $ListLength(Value) = 6 Set NoFailureFunction = $list(Value, 6) // Ideographical, if wordcount is 1, suppress failure function
				}
				*/
				NoFailureFunction = Value.NoFailureFunction; // should be always false for non Japanese, and true for Japanese single words
			}
			if (NoFailureFunction == 0) { // If NoFailureFunction = 0{ // no failure function for single words or numericals in Japanese
				int curFail = failFunc->Get(state); // Set curFail = failFunc.Get(state)
				failState = gotoFunc->Get(curFail, input);
				while (failState == -1) { // While (failState = "") {
					curFail = failFunc->Get(curFail); // Set curFail = failFunc.Get(curFail)
					failState = gotoFunc->Get(curFail, input); // Set failState = gotoFunc.Get(curFail, input)
				}
			}
		}
		failFunc->Set(nextState, failState); // Do failFunc.Set(nextState, failState)
		outputFunc->Merge(nextState, failState); // Do outputFunc.Merge(nextState, failState)

		// Do gotoFunc.DepthOrder(.depth, .state, .input, .nextState)
	}
	return failFunc;
}

void FailureFunction::ToC(std::string dir)
{
	// Do ##class(Util).OutputToFile(dir _ "/FailureTable.inl")
	// Open file:("WN":/TRANSLATE=1:/IOT="UTF8")
	std::ofstream ofs(dir + "/FailureTable.inl", std::ofstream::out); // std::ofstream::trunc?
	int TotalFailures = 0; // Set TotalFailures = 0
	std::string buf = ""; // Set buf = ""
	for (int i = 0; i <= MaxState; ++i) { // For i = 0 :1 : ..MaxState{
		int failState = Get(i); // Set failState = ..Get(i)
		buf = buf + std::to_string(failState) + ","; // Set buf = buf _ failState _ ","
		if (failState) TotalFailures = TotalFailures + 1; // If failState Set TotalFailures = TotalFailures + 1
		if (!(i % 256)) ofs << buf << std::endl, buf.clear(); // If '(i#256) Write buf,! Set buf = ""
	}
	ofs << buf; // Write buf
	ofs << "0" << std::endl; // Write "0", !
	ofs.close();
}
