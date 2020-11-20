#pragma once
#include "StateOutputFunction.h"
#include "IkTypes.h"

#include <string>
#include <algorithm>
#include <vector>

namespace iknow {
	namespace AHO {

		class LexrepStateOutputFunc :
			public StateOutputFunction
		{
		public:
			LexrepStateOutputFunc();
			~LexrepStateOutputFunc();

			int MaxWordCount; // Property MaxWordCount As %Integer[InitialExpression = 0];
			std::string LabelString; // Property LabelString As %String;
			std::vector<int> Labels; // Property Labels As %Binary;
			std::string MetaString; // Property MetaString As %String;

			Output_Value_Struct GetValue(iknow::base::String value) { // Method GetValue(value As %String) As %String
				int wordCount = 1 + (int) std::count(value.begin(), value.end(), ' ') -2; // Set wordCount = $L(value, " ") - 2
				if (wordCount > MaxWordCount) // If(wordCount > ..MaxWordCount) Set ..MaxWordCount = wordCount
					MaxWordCount = wordCount;
				return Output_Value_Struct(wordCount, Labels, value, LabelString, MetaString); // Quit $LB(wordCount, ..Labels, value, ..LabelString, ..MetaString)
			}

		};
		class LexrepIdeographicStateOutputFunction : // Class %iKnow.Compiler.LexrepIdeographicStateOutputFunction Extends %iKnow.Compiler.LexrepStateOutputFunc
			public LexrepStateOutputFunc
		{

			// 
			// Hiragana (u3040-u309F)                    http://unicode.org/charts/PDF/U3040.pdf
			// Katakana (u30A0-u30FF)				      http://unicode.org/charts/PDF/U30A0.pdf
			// CJK Unified Ideographs (u4E00-u9FCF)      http://www.unicode.org/charts/PDF/U4E00.pdf
			// CJK Symbols and Punctuation (u3000-u303F) http://unicode.org/charts/PDF/U3000.pdf
			// Halfwidth & Fullwidth forms (uFF00-uFFEF) http://unicode.org/charts/PDF/UFF00.pdf
			// 
			bool IsKatakanaWord(iknow::base::String word) // Method IsKatakanaWord(word As %String) As %Boolean
			{
				for (int cnt=1; cnt<=word.length(); cnt++) { // For cnt = 1:1 : $Length(word) {
					if (!IsKatakana(word[cnt - 1])) return false; //  If '..IsKatakana($Extract(word,cnt)) { Return 0 }
				}
				return true; // Return 1
			}
			bool IsKatakana(iknow::base::Char token) { // IsKatakana(token As %Char) As %Boolean
				return ((int)token >= 0x30A0) && ((int)token <= 0x30FF); // Quit:(($Ascii(token)'<$ZHEX("30A0")) & ($Ascii(token)'>$ZHEX("30FF"))) 1
			}
			bool IsJapanese(iknow::base::Char token) //	Method IsJapanese(token As %Char) As %Boolean
			{
				if ((int)token >= 0x3040 && (int)token <= 0x309F) return true; // Hiragana // Quit:(($Ascii(token)'<$ZHEX("3040")) & ($Ascii(token)'>$ZHEX("309F"))) 1 // Hiragana
				if ((int)token >= 0x309F && (int)token <= 0x30FF) return true; // Quit : (($Ascii(token)'<$ZHEX("309F")) & ($Ascii(token)'>$ZHEX("30FF"))) 1
				if ((int)token >= 0x4E00 && (int)token <= 0x9FCF) return true; // Quit : (($Ascii(token)'<$ZHEX("4E00")) & ($Ascii(token)'>$ZHEX("9FCF"))) 1
				if ((int)token >= 0x3000 && (int)token <= 0x303F) return true; // Quit : (($Ascii(token)'<$ZHEX("3000")) & ($Ascii(token)'>$ZHEX("303F"))) 1 // CJK Symbols and Punctuation (u3000-u303F)
				if ((int)token >= 0xFF00 && (int)token <= 0xFFEF) return true; // Quit : (($Ascii(token)'<$ZHEX("FF00")) & ($Ascii(token)'>$ZHEX("FFEF"))) 1 // Half width & Full width forms
				return false; // Quit 0
			}
			bool IsNumeric(iknow::base::Char token) { //	Method IsNumeric(token As %Char) As %Boolean
				return ((int)token >= 0x0030) && ((int)token <= 0x0039);  // Quit:(($Ascii(token)'<$ZHEX("0030")) & ($Ascii(token)'>$ZHEX("0039"))) 1 // '0' to '9'
			}
			bool IsNumericWord(iknow::base::String word) //	Method IsNumericWord(word As %String) As %Boolean
			{
				for (int cnt=1; cnt<=word.length(); ++cnt) { // For cnt = 1:1 : $Length(word) {
					if (!IsNumeric(word[cnt])) return false; // Return:'..IsNumeric($E(word,cnt)) 0 // not numerical
				}
				return true; // Return 1
			}
			int CountWithNumeric(iknow::base::String word) // Method CountWithNumeric(word As %String) As %Integer
			{
				bool bInNumeric = false; // Set bInNumeric = 0
				int cntNumeric = 0; // Set cntNumeric = 0
				int cntNonNumeric = 0; // Set cntNonNumeric = 0
				for (int cnt=1; cnt<=word.length(); ++cnt) { // For cnt = 1:1 : $Length(word) {
					if (IsNumeric(word[cnt - 1])) { // If ..IsNumeric($extract(word, cnt)) {
						if (!bInNumeric) cntNumeric = cntNumeric + 1; // If bInNumeric = 0 Set cntNumeric = cntNumeric + 1
						bInNumeric = true; //	Set bInNumeric = 1
					} else { // } Else{
						bInNumeric = false; // Set bInNumeric = 0
						cntNonNumeric = cntNonNumeric + 1; // Set cntNonNumeric = cntNonNumeric + 1
					}
				}
				return (cntNumeric + cntNonNumeric); // Return(cntNumeric + cntNonNumeric)
			}
			Output_Value_Struct GetValue(iknow::base::String value) { // Method GetValue(value As %String) As %String
				int wordCount = CountWithNumeric(value); // Set wordCount = ..CountWithNumeric(value)
				if (wordCount > MaxWordCount) // If(wordCount > ..MaxWordCount) Set ..MaxWordCount = wordCount
					MaxWordCount = wordCount;
				bool NoFailureFunction = (wordCount == 1); // Set NoFailureFunction = (wordCount = 1)
				if (value.find('}') != iknow::base::String::npos) wordCount = 1; // Set:$E(value, *) = "}" wordCount = 1 // regular expression
				return Output_Value_Struct(wordCount, Labels, value, LabelString, MetaString, NoFailureFunction); // Quit $LB(wordCount, ..Labels, value, ..LabelString, ..MetaString, NoFailureFunction)
			}

#if 0
					Method GetValue(value As %String) As %String
						{
							Set wordCount = ..CountWithNumeric(value)
							If(wordCount > ..MaxWordCount) Set ..MaxWordCount = wordCount
							Set NoFailureFunction = (wordCount = 1)
						Set:$E(value, *) = "}" wordCount = 1 // regular expression
							Quit $LB(wordCount, ..Labels, value, ..LabelString, ..MetaString, NoFailureFunction)
						}
#endif
		};

	}
}

