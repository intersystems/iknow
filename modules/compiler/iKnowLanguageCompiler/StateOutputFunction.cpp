#include "StateOutputFunction.h"

using namespace iknow::AHO;

/*
Property OutputFunc As OutputFunction;

Property Data As %Binary;

/// The GotoFunction callbacks: Derive any data needed in the value part of the output function from the token.
Method AddOutputAt(state As %Integer, token As %String)
{
	Do ..OutputFunc.Add(state, ..GetValue(token))
}

Method GetValue(token As %String) As %String
{
	Quit ..Data
}
*/

StateOutputFunction::StateOutputFunction() : pOutputFunc(0)
{
}


StateOutputFunction::~StateOutputFunction()
{
}
