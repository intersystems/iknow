#pragma once

#include "GotoFunction.h"
#include "OutputFunction.h"

#include <string>
#include <map>

namespace iknow {
	namespace AHO {  // Class %iKnow.Compiler.FailureFunction Extends %RegisteredObject [ Hidden, System = 4 ]

		class FailureFunction
		{
		public:
			FailureFunction();
			~FailureFunction();

			typedef std::map<int, int> FailureGlobal_Type;

			FailureGlobal_Type FailureGlobal; // Property FailureGlobal As %String [ MultiDimensional ];
			int MaxState; // Property MaxState As %Integer;

			static FailureFunction* Create(GotoFunction *gotoFunc, OutputFunction *outputFunc, bool is_ideographical); // ClassMethod Create(gotoFunc As GotoFunction, outputFunc As OutputFunction) As FailureFunction

			void Set(int state, int nextState) { // Method Set(state As %Integer, nextState As %Integer)
				FailureGlobal.insert(std::make_pair(state, nextState)); // Set ..FailureGlobal(state) = nextState
			}

			int Get(int state) { // Method Get(state As %Integer) As %Integer
				FailureGlobal_Type::iterator fg_iter = FailureGlobal.find(state);
				if (fg_iter == FailureGlobal.end()) return 0; // Quit $G(..FailureGlobal(state), 0)
				else return fg_iter->second;
			}

			void ToC(std::string dir = "");
		};

	}
}
