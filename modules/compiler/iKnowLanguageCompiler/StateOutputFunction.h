#pragma once
#include <string>
#include <vector>
#include "OutputFunction.h"
#include "IkTypes.h"

namespace iknow {
	namespace AHO {

		class StateOutputFunction
		{
		public:
			StateOutputFunction();
			~StateOutputFunction();

			OutputFunction *pOutputFunc; // Property OutputFunc As OutputFunction;
			Output_Value_Struct Data; // // Property Data As %Binary;

			virtual Output_Value_Struct GetValue(iknow::base::String value) { // Method GetValue(token As %String) As %String
				return Data; // Quit ..Data
			}

			/*
			/// The GotoFunction callbacks: Derive any data needed in the value part of the output function from the token.
			Method AddOutputAt(state As %Integer, token As %String)
			{
			Do ..OutputFunc.Add(state, ..GetValue(token))
			}

			*/
		};

	}
}

