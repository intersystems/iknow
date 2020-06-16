// StringAlg.h: interface for the CStringAlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGALG_H__09BB842C_9B36_4517_B42F_403CF465E646__INCLUDED_)
#define AFX_STRINGALG_H__09BB842C_9B36_4517_B42F_403CF465E646__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef WIN32
// Disables stl warnings
#pragma warning (push)
#pragma warning (disable: 4786)
#endif

#include "IkExportBase.h"
#include "IkObject.h"
#include "IkTypes.h"
//for debugging
#include "IkStringEncoding.h"
#include <iostream>
#include <algorithm>
#include <cmath>

//Cache String types
#define NO_UCHAR_TYPE
#ifdef ISC_IRIS
#include "sysCommon.h"
#endif
//Undefine this straggler from windows.h: StringHash also
//has a "GetObject" member
#ifdef GetObject
#undef GetObject
#endif

#include "SafeString.h"


namespace iknow
{
  namespace base
  {

    //! Class with some std::string functionality 
    /*!
     *	This class yields some common used algoritmes on strings
     *	Exceptions: IkException 
     */
    class BASE_API IkStringAlg
    {
    public:
      IkStringAlg(); /*!< Constructor. */
      virtual ~IkStringAlg(); /*!< Destructor. */
				
      /*!
       *	Cast a stl string to lower case
       *	\param strInput string that has to be converted
       *	\exception IkException: UNKNOWN ERROR
       */
      static void ToLower(String& strInput);

      enum CapitalizationClass {
	NoCapitals,
	InitialCapital,
	MixedCapitals,
	AllCapitals
      };

      static CapitalizationClass FindCapitalizationClass(const String& string);
      static bool IsAscii(const String& string) {
        for (String::const_iterator c=string.begin(); c!=string.end(); c++) {
          if (*c > 0x7F) return false; // non ascii char
        }
        return true; // no >0x7F chars detected
      }

      //TODO: Trim is now a substantial portion of preprocessing!
      //It should be much, much simpler:
      //Hard code the ' '.
      //Scan until you don't see a space.
      //Scan backwards until you don't see a space.
      //If neither forwards nor backwards scans found anything, return.
      //If forwards found something, call replace using the right range.
      //If backwards found something, just resize it off.

      //Also look at improving the quotes versions used in normalization

      /*!
       *	Trim a string (delete leading and trailing delimiters)
       *	\param strInput string that has to be converted
       *	\param strDelimiter characters that heve to be removed (leading and trailing)
       *	\exception IkException: UNKNOWN ERROR
       */
      template<typename StringT>
	static void Trim(StringT& strInput, const StringT& strDelimiter)
      {
	typename StringT::size_type end = strInput.find_last_not_of(strDelimiter);
	if(end != StringT::npos) {
	  end = end + 1;
	  typename StringT::size_type begin = strInput.find_first_not_of(strDelimiter);
	  if (end == strInput.size() && begin == 0) return;
	  if (begin != StringT::npos) {
	    //To avoid reallocation, copy out the desired substring into a static buffer,
	    //then copy it back into the string.
	    const typename StringT::value_type* char_begin = strInput.data() + begin;
	    static StringT out(64, static_cast<typename StringT::value_type>('\0'));
	    out.replace(0, out.size(), 0, static_cast<typename StringT::value_type>('\0'));
	    out.append(char_begin, end - begin);
	    strInput.replace(0, strInput.size(), end - begin, static_cast<typename StringT::value_type>('\0'));
	    std::copy(out.begin(), out.end(), strInput.begin());
	  }
	  else {
	    strInput.replace(0, strInput.size(), 0, static_cast<typename StringT::value_type>('\0'));
	  }
	}
      }

      /*!
       *	Trim a string (delete leading and trailing spaces)
       *	\param strInput string that has to be converted
       *	\exception IkException: UNKNOWN ERROR
       */

      template<typename StringT>
	static void Trim(StringT& strInput)
      {
	//Our default delimiter is an ascii space
	static const typename StringT::value_type space[] = {' ','\0'};
	static const StringT spaceString(space);
	Trim(strInput, spaceString);
      }

      /*!
       *	remove characters from a string based on delimiters
       *	\param strInput string that has to be tokenized
       *	\param strDelimiter string that specifies the delimiters 
       *	\exception IkException: UNKNOWN ERROR
       */

      template<typename StringT>
	static void RemoveCharacters(StringT& strInput, const StringT& strDelimiters)
      {
	typename StringT::size_type pos = strInput.find_first_of(strDelimiters, 0);
	
	while (StringT::npos != pos )
	  {
	    strInput.erase(pos, 1);	//remove the current element
	    pos = strInput.find_first_of(strDelimiters, pos); // find next "non-delimiter"
	  }
      }

	static void Normalize(String& strInput, bool bLowerCase=true, bool bStripPunct=true);
	static String RemoveLeadingNonDigitWords(const String& strInput);
	static String RemoveLeadingWord(const String& strInput);
	static void NormalizeDigits(String& strInput);
	static void NormalizeWidth(String& strInput);
	static void RemovePunctuationAndQuotes(String& strInput);
	static void RemoveSentenceMarkers(String& strInput);
	static void RemoveAllMarkers(String& strInput);
	static void RemoveControlChars(String& strInput);

    static bool IsQuote(Char chr);
	static bool IsText(const String& strInput);
	static bool IsText(const Char *begin, const Char *end);

      /*!
       *	IsPunctuation, check if a character symbol is a punctuation
       *	\param chr, character to check
       */
      static bool IsPunctuation(Char chr);
      static bool IsJpnSplit(const Char chr); // Is Sentence Splitter for JPN ?
      static bool IsJpnChar(const Char chr); // Is JPN char ?
      static bool IsJpnIDSP(const Char chr) { return (chr==0x3000); } // Double space to mark a new paragraph is at the very beginning of a sentence.
      static bool IsJpnDOT(const Char chr) { return (chr==0x30fb); } // DOT character used in Furigana Hiragana
	  static bool IsJpnChoon(const Char chr) { return (chr == 0x30fc); } // Cho-on symbol used in Furigana Hiragana
	  static bool IsJPnakaguro(const Char chr) { return (chr == 0x30FB || chr == 0xFF65); } // bullet point
      static bool IsKatakana(const Char chr) { return ( ((chr>=0x30A0)&&(chr<=0x30FF)) ||   // Katakana (u30A0-u30FF)	http://unicode.org/charts/PDF/U30A0.pdf
													    ((chr>=0xFF65)&&(chr<=0xFF9F))); }  // Katakana small-width (uFF65-uFF9F) http://unicode.org/charts/PDF/UFF00.pdf
      static bool IsHiragana(const Char chr) { return ((chr>=0x3040)&&(chr<=0x309F)); }  // Hiragana (u3040-u309F)  http://unicode.org/charts/PDF/U3040.pdf
      static bool IsJpnNumber(const Char chr) { return (((chr>=0x0030)&&(chr<=0x0039))||((chr>=0xFF10)&&(chr<=0xFF19))); } // Numbers, both ascii & halfwidth forms
	  static bool IsJpnPunctuationException(const Char chr) { return (chr == 0xFF0A); }
      static bool IsOpenParenthesis(const Char chr) { return ((chr==0x0028 || chr==0xFF08)); } // Open parenthesis, both ascii & halfwidth forms
      static bool IsCloseParenthesis(const Char chr) { return ((chr==0x0029 || chr==0xFF09)); } // Close parenthesis, both ascii & halfwidth forms
	  static bool IsOpenSymbolPair(const Char chr) { return ((chr == 0x0028 || chr == 0xFF08)); }
	  static bool IsCloseSymbolPair(const Char chr) { return ((chr == 0x0029 || chr == 0xFF09)); }
	  static bool IsOpenPunctuationPair(const Char);
	  static bool IsClosePunctuationPair(const Char);
	  static bool IsPunctuationException(const Char);

	  static bool IsKatakana(const String&);

      enum FuriganaClass {
        NoFurigana,
        KatakanaFurigana,
        HiraganaFurigana,
        NumberFurigana
      };
      static FuriganaClass FindFuriganaClass(const Char chr) {
        if (IsHiragana(chr)) return HiraganaFurigana;
        if (IsKatakana(chr)) return KatakanaFurigana;
        if (IsJpnNumber(chr)) return NumberFurigana;
        return NoFurigana;
      }

      /*!
       *	Convert integer to string
       *	\param nInput integer that has to be converted
       *	\exception IkException: UNKNOWN ERROR
       */
      static std::string ToString(int const & nInput);

      /*!
       *	Convert double to string
       *	\param fInput double that has to be converted
       *	\exception IkException: UNKNOWN ERROR
       */
      static std::string ToString(double const & fInput);

      static std::string ToString(unsigned long const & lnInput);

      /*!
       *	Replaces all occurences of strOld with strNew in strInput
       *	\param strInput string which contains the string to be replaced
       *	\param strOld the old string
       *	\param strNew the new string
       *	\param bGlobal If global is true, it replaces all occurences of strOld with strNew, otherwise, only one.
       *	\exception IkException: UNKNOWN ERROR
       */

      static void Replace(String& strInput, const String& strOld, const String& strNew, bool bGlobal = true) {
	size_t x1 = strInput.find(strOld); // search 
	if (x1 == String::npos) return; // no match
	
	size_t x2 = strOld.size();
	if (bGlobal)
	  {
	    size_t y2 = strNew.size();
	    while(x1 != String::npos)
	      {
		strInput.replace(x1, x2, strNew);
		x1 = strInput.find(strOld, x1+y2); // search 
	      }
	  }
	else
	  strInput.replace(x1, x2, strNew);
      }

      //Version for "raw" shared memory KB strings
      static void Replace(String& strInput, const Char* strOld, size_t strOldLen, const Char* strNew, size_t strNewLen) {
	size_t x1 = strInput.find(strOld, 0, strOldLen); // search 
	if (x1 == String::npos) return; // no match
	
	size_t x2 = strOldLen;
	size_t y2 = strNewLen;
	while(x1 != String::npos) {
	  strInput.replace(x1, x2, strNew, strNewLen);
	  x1 = strInput.find(strOld, x1+y2, strOldLen); // search 
	}
      }

      /*
      ** JDN, 2009.07.31
      ** bExact: string to replace must be exactly present in the text, surrounded by blanks (not part of another string)
      */
      static void ReplaceGlobal(String& strInput, const String& strOld, const String& strNew, bool bExact) {
	size_t x1 = strInput.find(strOld);
	size_t x2 = strOld.size();
	size_t y2 = strNew.size();
	Char cPrv=' ';
	
	while (x1 != String::npos) {
	  if (bExact) { // check previous & next char
	    if (x1>0) cPrv=strInput.at(x1-1);
	    Char cNxt=((x1+x2) < strInput.length() ? strInput.at(x1+x2) : '\0'); // get next char
	    
	    switch (cPrv) {	case ' ': case '\t': case '\n': break; default: x1=strInput.find(strOld, x1+x2); continue;/*while*/ }
	    
	    if (cNxt != '\0')
	      {
		switch (cNxt) {	case ' ': case '\t': case '\n': break; default: x1=strInput.find(strOld, x1+x2); continue;/*while*/ }
	      }
	  }
	  strInput.replace(x1, x2, strNew);
	  x1 = strInput.find(strOld, x1+y2); // search 
	}
      }

      //Version for "raw" shared memory KB strings
      static void ReplaceGlobal(String& strInput, const Char* strOld, size_t strOldLen, const Char* strNew, size_t strNewLen) {
	size_t x1 = strInput.find(strOld, 0, strOldLen);
	size_t x2 = strOldLen;
	size_t y2 = strNewLen;
	Char cPrv=' ';
	
	while (x1 != String::npos) {
	  if (x1>0) cPrv=strInput[x1-1];
	  Char cNxt=((x1+x2) < strInput.length() ? strInput[x1+x2] : ' '); // get next char
	  if (cNxt != ' ' || cPrv != ' ') {
	    x1 = strInput.find(strOld, x1+x2, strOldLen);
	    continue;
	  }
	  strInput.replace(x1, x2, strNew, strNewLen);
	  x1 = strInput.find(strOld, x1+y2, strOldLen); // search 
	}
      }


      
      /*!
       *	Checks if the stirng strInput ends with the string strEnd.
       *	\param strInput input string
       *	\param strEnd the end of the string
       *	\return Returns 1 if strInput ends with strEnd, 0 otherwise
       *	\exception IkException: UNKNOWN ERROR
       */
      static int EndsWith(const std::string& strInput, const std::string& strEnd);

	  // returns value in key=value pair, used for lexrep metadata.
	  static std::string GetMetaValue(const char* meta_data, const char* key);

      //For each token. For speed in, e.g., summarizer, this template function splits a string based on a single character
      //delimiter, and passed each token to a function-like object.
      template<typename StringT, typename OutFuncT>
	  static void Tokenize(const StringT& input, typename StringT::value_type delimiter, OutFuncT& out_func) {
	    size_t start = input.find_first_not_of(delimiter);
	    size_t end;
	    while (start != StringT::npos) {
	      end = input.find(delimiter, start);
	      size_t len = end == StringT::npos ? StringT::npos : end - start;
	      out_func(input.substr(start, len));
	      start = (end == StringT::npos ? end : end + 1);
	    }
      }

      //For each token. For speed in, e.g., summarizer, this template function splits a string based on a single character
      //delimiter, and passed each token to a function-like object.
      template<typename CharT, typename OutFuncT>
	static void Tokenize(const CharT* input_begin, const CharT* input_end, CharT delimiter, OutFuncT& out_func) {
	const CharT* token_begin = input_begin;
	while (input_begin != input_end) {
	  if (*input_begin == delimiter) {
	    out_func(token_begin, input_begin);
	    token_begin = input_begin  + 1;
	  }
	  ++input_begin;
	}
	if (token_begin != input_begin) out_func(token_begin, input_begin);
      }

      
	  //Same as previous, but extra flag to indicate the last piece (LP)
      template<typename StringT, typename OutFuncT>
	  static void TokenizeWithLPFlag(const StringT& input, typename StringT::value_type delimiter, OutFuncT& out_func) {
	    size_t start = input.find_first_not_of(delimiter);
	    size_t end;
	    while (start != StringT::npos) {
	      end = input.find(delimiter, start);
	      size_t len = end == StringT::npos ? StringT::npos : end - start;
	      out_func(input.substr(start, len), end == StringT::npos);
	      start = (end == StringT::npos ? end : end + 1);
	    }
      }

	  //Same as previous, but extra flag to indicate the last piece (LP)
      template<typename CharT, typename OutFuncT>
		static void TokenizeWithLPFlag(const CharT* input_begin, const CharT* input_end, CharT delimiter, OutFuncT& out_func) {
		const CharT* token_begin = input_begin;
		while (input_begin != input_end) {
			if (*input_begin == delimiter) {
				out_func(token_begin, input_begin, false);
				token_begin = input_begin  + 1;
			}
			++input_begin;
		}
		out_func(token_begin, input_begin, true);  // The last piece, can be empty if text ends with delimiter.
      }
    };
  }
}

#ifdef WIN32
#pragma warning (pop)
#endif

#endif // !defined(AFX_STRINGALG_H__09BB842C_9B36_4517_B42F_403CF465E646__INCLUDED_)

