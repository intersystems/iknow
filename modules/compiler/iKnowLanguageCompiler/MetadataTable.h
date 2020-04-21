#pragma once

#include <vector>
#include <fstream>

namespace iknow {
	namespace AHO {

		class MetadataTable
		{
		public:
			MetadataTable();
			~MetadataTable();

			std::vector<int> Values; // Property Values As list Of %Integer;

			void ToC(std::string dir) 
			{
				// Do ##class(Util).OutputToFile(dir _ "/Metadata.inl")
				// Open file:("WN":/TRANSLATE=1:/IOT="UTF8")
				std::ofstream ofs(dir + "/Metadata.inl", std::ofstream::out); // std::ofstream::trunc?
				//Write metadata
				ofs << "DataValue Metadata[] = {" << std::endl; // Write "DataValue Metadata[] = {", !
				for (int i = 1; i <= Values.size(); ++i) {
					ofs << Values.at(i - 1) << "," << std::endl;
				}
				ofs << "};" << std::endl; // Write "};", !
				ofs.close();  // Close $IO
			}

			void AddValue(int value)
			{
				Values.push_back(value);
			}

		};

	}
}

/* Copyright (c) 2020 by InterSystems Corporation.
Cambridge, Massachusetts, U.S.A.  All rights reserved.
Confidential property of InterSystems Corporation. */
/*
/// A class to construct and output metadata tables in C code
Class %iKnow.Compiler.MetadataTable Extends %RegisteredObject[Hidden]
{

	Property Values As list Of %Integer;

	Method ToC(dir As %String = "")
	{
		Do ##class(Util).OutputToFile(dir _ "/Metadata.inl")
			//Write metadata
			Write "DataValue Metadata[] = {", !
			For i = 1 :1 : ..Values.Count() {
			Write ..Values.GetAt(i), ",", !
		}
		Write "};", !
			Close $IO
	}

	Method AddValue(value As %Integer)
	{
		Do ..Values.Insert(value)
	}

}
*/

