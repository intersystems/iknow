#pragma once
#include "IkTypes.h"

#include <string>
#include <map>
#include <vector>
#include <set>

namespace iknow {
	namespace AHO {
		typedef std::vector<int> labels_Type;
		struct Output_Value_Struct {
			Output_Value_Struct() :
				wordCount(0)
			{}
			Output_Value_Struct(int wordCount_par, std::vector<int> Labels_par, iknow::base::String value_par, std::string LabelString_par, std::string MetaString_par) :
				wordCount(wordCount_par), Labels(Labels_par), value(value_par), LabelString(LabelString_par), MetaString(MetaString_par), NoFailureFunction(false)
			{}
			Output_Value_Struct(int wordCount_par, std::vector<int> Labels_par, iknow::base::String value_par, std::string LabelString_par, std::string MetaString_par, bool NoFailureFunction_par) :
				wordCount(wordCount_par), Labels(Labels_par), value(value_par), LabelString(LabelString_par), MetaString(MetaString_par), NoFailureFunction(NoFailureFunction_par)
			{}

			bool operator==(const Output_Value_Struct &rhs) {
				if (this->wordCount != rhs.wordCount) return false;
				if (this->Labels != rhs.Labels) return false;
				if (this->value != rhs.value) return false;
				if (this->MetaString != rhs.MetaString) return false;

				return true;
			}
			int wordCount;
			labels_Type Labels;
			iknow::base::String value;
			std::string LabelString;
			std::string MetaString;
			bool NoFailureFunction;
		};

		typedef std::vector<Output_Value_Struct>		StateGlobal_Output_Type;
		typedef std::vector<StateGlobal_Output_Type>	StatesGlobal_Output_Type;

		class OutputFunction  // Class %iKnow.Compiler.OutputFunction Extends %RegisteredObject[Hidden, System = 4]
		{

		public:
			OutputFunction();
			~OutputFunction();
			
			StatesGlobal_Output_Type StatesGlobal_Output; // Property StatesGlobal As %String[MultiDimensional];

			int MaxState; // Property MaxState As %Integer;

			void Merge(int toState, int fromState) // Method Merge(toState As %Integer, fromState As %Integer)
			{
				int count = GetCount(fromState); // Set count = ..GetCount(fromState)
				for (int i = 0; i <= count - 1; ++i) { // For i = 0:1 : count - 1 {
					Add(toState, Get(fromState, i)); // Do ..Add(toState, ..Get(fromState, i))
				}
			}

			bool HasValue(int state, Output_Value_Struct value)
			{
				bool hasValue = false;
				for (StateGlobal_Output_Type::iterator it_output = StatesGlobal_Output[state].begin(); it_output != StatesGlobal_Output[state].end(); it_output++) {
					if (value == *it_output) { hasValue = true; break; } // if (Get(state, i) == value) { hasValue = true; break; }
				}
				return hasValue;
			}

			void Add(int state, Output_Value_Struct value)
			{
				if (HasValue(state, value)) return;
				if (MaxState < state) MaxState = state; // If ..MaxState < state Set ..MaxState = state
				StatesGlobal_Output[state].push_back(value);
			}

			int GetCount(int state) // Method GetCount(state As %Integer) As %Integer[CodeMode = expression, Final]
			{
				std::vector<Output_Value_Struct>& state_output = StatesGlobal_Output[state];
				return (int) state_output.size(); // number of outputs for this state
			}
			Output_Value_Struct Get(int state, int index, std::string default_output = "") // Method Get(state As %Integer, index As %Integer, default As %String = "")[CodeMode = expression, Final]
			{
				std::vector<Output_Value_Struct>& state_output = StatesGlobal_Output[state]; // $G(..StatesGlobal("Output", state, index), default)
				if (state_output.empty()) return Output_Value_Struct();
				return state_output[index];
			}

			virtual void ToC(std::string file) // Method ToC(file As %String = "")
			{
			}

		};

	}
}

/*
Class %iKnow.Compiler.OutputFunction Extends %RegisteredObject[Hidden, System = 4]
{
		Method SetAt(state As %Integer, count As %Integer, value As %String) [Private]
		{
			If ..MaxState < state Set ..MaxState = state
				Set ..StatesGlobal("Output", state, count) = value
		}
		Method SetCount(state As %Integer, count As %Integer)
		{
				Set ..StatesGlobal("Output", state) = count
		}
		Method GetCount(state As %Integer) As %Integer[CodeMode = expression, Final]
		{
				+$G(..StatesGlobal("Output", state))
		}
		Method Get(state As %Integer, index As %Integer, default As %String = "") [CodeMode = expression, Final]
		{
					$G(..StatesGlobal("Output", state, index), default)
		}
}

*/
