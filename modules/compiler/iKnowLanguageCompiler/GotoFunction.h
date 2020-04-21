#pragma once
#include <string>
#include <list>
#include <map>
#include <vector>

#include "IkTypes.h"
#include "StateOutputFunction.h"
#include "KnowledgebaseRegexDictionary.h"

namespace iknow {
	namespace AHO {

		// Set ..StatesGlobal("DepthMap", IKCdepth - 1, state, char, nextState) = ""
		struct DepthMap {
			DepthMap() {} // for sorting
			DepthMap(int IKCdepth_par, int state_par, iknow::base::String char_par, int nextstate_par) : depth(IKCdepth_par), state(state_par), input(char_par), nextstate(nextstate_par) {} // for filling

			int depth;
			int state;
			iknow::base::String input;
			int nextstate;
			
			bool operator()(DepthMap const &a, DepthMap const &b) const { // sort operator : first depth, then state, then input, then nextstate
				if (a.depth < b.depth) return true;
				if (a.depth == b.depth) {
					if (a.state < b.state) return true;
					if (a.state == b.state) {
						if (a.input < b.input) return true;
						if (a.input == b.input) {
							return (a.nextstate < b.nextstate);
						}
						return false;
					}
					return false;
				}
				return false;
			}

		};

		class GotoFunction
		{
		public:
			GotoFunction();
			~GotoFunction();

			// static std::list<iknow::base::String> ParseAlternatives(iknow::base::String& input);
			template<typename T>
			static std::list<T> ParseAlternatives(T& input)
			{
				size_t len = input.length();
				int depth = 1; //the leading "("
				T token; // = "";
				std::list<T> tokenList; // Set tokenList = ""
				bool escaped = false; // Set escaped = 0
				int i = 2 - 1;
				for (; i < (int)len; ++i) {
					typename T::value_type char_ = input[i]; // Set char = $E(input, i)

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


			void Addword(iknow::base::String word, StateOutputFunction *stateOutputFuncton); // Method AddWord(word As %String, stateOutputFunction As StateOutputFunction)

			/*
			Property StatesGlobal As %String[MultiDimensional];
			Property NextState As %Integer[InitialExpression = 1];
			Property RegexEnabled As %Boolean[InitialExpression = 0];
			*/
			typedef std::map<int, std::map<iknow::base::String, int>> StatesGlobalGoTo_Type;
			StatesGlobalGoTo_Type StatesGlobal_GoTo;
			typedef std::map<int, int> StatesGlobalDepth_Type;
			StatesGlobalDepth_Type StatesGlobal_Depth;
			typedef std::vector<struct DepthMap>	DepthMap_Type;
			DepthMap_Type StatesGlobal_DepthMap;
			int NextState;
			bool RegexEnabled;
			iknow::AHO::KnowledgebaseRegexDictionary *RegexDictionary; // Property RegexDictionary As RegexDictionary;

			/*
			void Set(int state, iknow::base::String input, int nextState) // Method Set(state As %Integer, input As %String, nextState As %Integer)
			{
				// Set ..StatesGlobal("GoTo", state, input) = nextState
				StatesGlobalGoTo_Type::iterator it_state = StatesGlobal_GoTo.find(state);
				if (it_state == StatesGlobal_GoTo.end()) { // state does not exist
					std::map<iknow::base::Char, int> input_next;
					input_next.insert(std::make_pair(input[0], nextState)); // first input-nextState
					StatesGlobal_GoTo.insert(std::make_pair(state, input_next));
				}
				else { // state does exist
					it_state->second.insert(std::make_pair(input[0], nextState)); // add input-nextState
				}
				//Record depth
				// Set depth = $G(..StatesGlobal("Depth", state)) + 1
				int depth = 0;
				StatesGlobalDepth_Type::iterator it_depth = StatesGlobal_Depth.find(state);
				if (it_depth != StatesGlobal_Depth.end()) depth = it_depth->second; 
				depth = depth + 1;
				// Set ..StatesGlobal("Depth", nextState) = depth
				StatesGlobal_Depth.insert(std::make_pair(nextState, depth));
				//Depth map: We need states sorted by depth to build the failure function
				// Set ..StatesGlobal("DepthMap", depth - 1, state, input, nextState) = ""
				StatesGlobal_DepthMap.push_back(DepthMap(depth - 1, state, input[0], nextState));
			}
			*/
			int Get(int state, iknow::base::String input) // Method Get(state As %Integer, input As %String) As %Integer
			{
				// Set next = $G(..StatesGlobal("GoTo", state, input))
				StatesGlobalGoTo_Type::iterator it_state = StatesGlobal_GoTo.find(state);
				if (it_state == StatesGlobal_GoTo.end()) return -1;
				std::map<iknow::base::String, int>::iterator it_input = it_state->second.find(input);
				if (it_input == it_state->second.end()) return (state == 0 ? 0 : -1); //	If(next = "") && (state = 0) Quit 0 //0-state loops back
				return it_input->second; //	Quit next
			}
			
			void ToC(std::string dir = "");

		private:
			void ConsumeInput(int state, iknow::base::String input, StateOutputFunction *outputObj, iknow::base::String curToken = iknow::base::String(), bool escaping = false);

		};

	}
}

/*
Method Set(state As %Integer, input As %String, nextState As %Integer)
{
	Set ..StatesGlobal("GoTo", state, input) = nextState
		//Record depth
		Set depth = $G(..StatesGlobal("Depth", state)) + 1
		Set ..StatesGlobal("Depth", nextState) = depth
		//Depth map: We need states sorted by depth to build the failure function
		Set ..StatesGlobal("DepthMap", depth - 1, state, input, nextState) = ""
}

Method Get(state As %Integer, input As %String) As %Integer
{
	Set next = $G(..StatesGlobal("GoTo", state, input))
	If(next = "") && (state = 0) Quit 0 //0-state loops back
	Quit next
}
*/


