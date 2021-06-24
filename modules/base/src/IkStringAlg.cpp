// StringAlg.cpp: implementation of the IkStringAlg class.
//
//////////////////////////////////////////////////////////////////////
#include "IkStringAlg.h"
#include "unicode/ustring.h"
#include "utlExceptionFrom.h"
#include "unicode/uchar.h"
#include <math.h>
#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <functional>
#include <stdio.h>
#include <cstring>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace std;
using namespace iknow::base;

IkStringAlg::IkStringAlg() {}
IkStringAlg::~IkStringAlg() {}

static bool FastAsciiToLower(String& strInput) {
  //Much of the time, we'll be processing ASCII strings. If every
  //character is in the ASCII range, we can lowercase in-place and not worry
  //about calling ICU at all.
  //returns true if the string is all ASCII and now lowercased.
  //returns false as soon as the first non-ASCII character is found, leaving
  //any in progress lowercasing done.
  for (String::iterator i = strInput.begin(); i != strInput.end(); ++i) {
    if (*i > 127) return false;
    if (*i >= 'A' && *i <= 'Z') *i = *i - 'A' + 'a';
  }
  return true;
}

//TODO: TRW, move to IkStringEncoding
void IkStringAlg::ToLower(String& strInput) {
  //The ICU API is 32 bits even on 64 bit platforms.
  //TODO: To be safe, we have to be sure our strings are not
  //too big
  size_t input_size_full = strInput.size();
  size_t buf_size_full = input_size_full * 2;
  int32_t input_size = static_cast<int32_t>(input_size_full);
  int32_t buf_size = static_cast<int32_t>(buf_size_full); 
 
  static UChar *buf;
  static int32_t buf_max;
  if (buf_size > buf_max) {
    if (buf) delete[] buf;
    buf = new UChar[buf_size];
    buf_max = buf_size;
  }

  const UChar *in = strInput.data();
  UErrorCode converr = U_ZERO_ERROR;
  int32_t output_size = u_strToLower(buf, buf_size, in, input_size, NULL /* default locale */, &converr); 
  if (U_FAILURE(converr)) {
    throw MessageExceptionFrom<IkStringAlg>("IKFailedLowercase");
  }
  strInput.assign(buf, output_size);
}

IkStringAlg::CapitalizationClass IkStringAlg::FindCapitalizationClass(const String& string) {
  //TODO: Surrogate pairs will fail here.
  bool has_initial_capital = false;
  bool has_all_capitals = false;
  String::const_iterator c = string.begin();
  //Skip initial non-upper or lower characters. Set has_initial_capital as a side effect.
  while (c != string.end() && !(u_isULowercase(*c) != 0 ||
				(has_initial_capital = u_isUUppercase(*c) != 0) != 0)) {
    ++c;
  }
  if (c == string.end()) return NoCapitals;
  has_all_capitals = has_initial_capital;
  for(; c != string.end(); ++c) {
    bool is_upper = u_isUUppercase(*c) != 0;
    bool is_lower = u_isULowercase(*c) != 0;
    //skip non-letters
    if (!(is_lower || is_upper)) continue;
    //still seeing all caps?
    if (is_upper && has_all_capitals) continue;
    //mixed caps detected. We're not all caps
    //but we must have seen a non-capital before or
    //has_all_capitals would be set)
    if (is_upper) return MixedCapitals;
    //At this point, it must be lower.
    has_all_capitals = false;
  }
  if (has_all_capitals) return AllCapitals;
  if (has_initial_capital) return InitialCapital;
  return NoCapitals;
}


//TODO, TRW: This kind of normalization character information should be in the KB.

static const Char punctuation[] = {',','(',')','[',']','{','}',':', 0x00A1 /*inverted exclamation*/, 0x00BF /*inverted question mark*/, 0x2013 /*en dash*/, 0x2014 /*em dash*/, '\0'};
//Sorted. Someday all these should be that way.
static const Char punctuationAndQuotes[] = { 0x0022, // "
 					     0x0027, // '
						 0x0028, // (
						 0x0029, // )
					     0x002C, // ,
					     0x003A, // :
					     0x005B, // [
					     0x005D, // ]
					     0x0060, // `
					     0x007B, // {
					     0x007D, // }
					     0x00AB, // « left straight double quote
					     0x00B4, // acute accent
					     0x00BB, // » right straight double quote
					     0x05F3, // hebrew single "geresh"
					     0x05F4, // hebrew double "gershayim"
					     0x2013, // en dash
					     0x2014, // em dash
					     0x2019, // right single quote
					     0x201C, // left double quote
					     0x201D,  // right double quote
					     0x2028, // line separator
					     0x2032, // prime mark
					     0x2033  // double prime mark
};

static const Char OpenPunctuationPair[] = { // Left
  0x0028, // (
  0x005B, // [
  0x007B, // {
  0x201C, // "
  0x2036, // '
  0x3008, 0x300A, 0x300C, 0x300E, 0x3010, 0x3014, 0x3016, 0x3018, 0x301A, 0x301D, // http://unicode.org/charts/PDF/U3000.pdf
  0xFF08, // ( Half-Width forms
  0xFF3B, // [
  0xFF5B, // {
  0xFF5F, // ((
  0xFF62 // [
};
static const Char ClosePunctuationPair[] = { // Right
  0x0029, // )
  0x005D, // ]
  0x007D, // }
  0x201D, // "
  0x2033, // '
  0x3009, 0x300B, 0x300D, 0x300F, 0x3011, 0x3015, 0x3017, 0x3019, 0x301B, 0x301E, 0x301F, // http://unicode.org/charts/PDF/U3000.pdf
  0xFF09, // ) Half-Width forms
  0xFF3D, // ]
  0xFF5D, // }
  0xFF60, // ))
  0xFF63 // ]
};

static const Char* punctuationAndQuotesBegin = &(punctuationAndQuotes[0]);
static const Char* punctuationAndQuotesEnd = punctuationAndQuotesBegin + sizeof(punctuationAndQuotes) / sizeof(punctuationAndQuotes[0]);
static const Char* OpenPunctuationPairBegin = &(OpenPunctuationPair[0]);
static const Char* OpenPunctuationPairEnd = OpenPunctuationPairBegin + sizeof(OpenPunctuationPair) / sizeof(OpenPunctuationPair[0]);
static const Char* ClosePunctuationPairBegin = &(ClosePunctuationPair[0]);
static const Char* ClosePunctuationPairEnd = ClosePunctuationPairBegin + sizeof(ClosePunctuationPair) / sizeof(ClosePunctuationPair[0]);

static const Char sentenceSplits[] = {'.',';','!','?','\0'};
static const Char jpn_sentenceSplits[] = { 0x0021 /*!*/, 0x003F /*?*/, 0x3002, /*0xFF0E:double width point,*/ 0xFF01, 0xFF1F, 0xFE52, 0xFE56, 0xFE57, 0xFF61, 0x0000 }; // Japanese sentence splitters: sentence end, fullwith stop, exclamation and question, small stop, question and exclamation.
static const Char quotes[] = { 0x0022, 0x0027, 0x0060, 0x00AB, 0x00B4, 0x00BB, 0x05F4, 0x05F3, 0x2019, 0x2032, 0x2033, 0x2028, 0x201C, 0x201D, 0x0000 }; // 0x2028 DTI space

static const Char splitsAndQuotes[] = { 0x0021 /*!*/, 0x0022 /*"*/, 0x0027 /*'*/, 0x002E /*.*/, 0x003B /*;*/, 0x003F /*?*/, 0x0060, 0x00AB, 0x00B4, 0x00BB, 0x05F3, 0x05F4, 0x2019, 0x201C, 0x201D, 0x2028, 0x2032, 0x2033 };
static const Char* splitsAndQuotesBegin = &(splitsAndQuotes[0]);
static const Char* splitsAndQuotesEnd = splitsAndQuotesBegin + sizeof(splitsAndQuotes) / sizeof(splitsAndQuotes[0]);

static const Char punctuationExceptions[] = { 0x0023, 0x0024, 0x0025, 0x0026, 0x002A, 0x002B, 0x002D, 0x002F, 0x003C, 0x003D, 0x003E, 0x007E, 0x00A3, 0x00A5, 0x00A7, 0x00A9, 0x00AE, 0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00BC, 0x00BD, 0x00BE, 0x00D7, 0x2013, 0x2020, 0x2030, 0x20AC, 0x20B4, 0x2116, 0x2229, 0x2248, 0x2264, 0x2265, 0x222B };
static const Char* punctuationExceptionsBegin = &(punctuationExceptions[0]);
static const Char* punctuationExceptionsEnd = punctuationExceptionsBegin + sizeof(punctuationExceptions) / sizeof(punctuationExceptions[0]);

static const String strPunctuation(punctuation);
static const String strQuotes(quotes);
static const String strSplits(sentenceSplits);
static const String strJpnSplits(jpn_sentenceSplits); // Japanese sentence splitters
static const String strPunctuationAndQuotes = strPunctuation + strQuotes;
static const String strSplitsAndQuotes = strSplits + strQuotes;
static const String strSplitsAndPunctuation = strSplits + strPunctuation;
static const String strAllMarkers = strPunctuation + strQuotes + strSplits;

//Sort for binary search
static const String& GetSortedMarkers() {
  static String* sortedMarkers = 0;
  if (!sortedMarkers) {
    sortedMarkers = new String(strAllMarkers);
    std::sort(sortedMarkers->begin(), sortedMarkers->end());
  }
  return *sortedMarkers;
}

static inline bool IsPunctuationOrQuote(Char c) {
  const Char* d = std::lower_bound(punctuationAndQuotesBegin, punctuationAndQuotesEnd, c);
  return d != punctuationAndQuotesEnd && *d == c;
}
static inline bool IsSplitOrQuote(Char c) {
  const Char* d = std::lower_bound(splitsAndQuotesBegin, splitsAndQuotesEnd, c);
  return d != splitsAndQuotesEnd && *d == c;
}

bool IkStringAlg::IsOpenPunctuationPair(const Char c) {
	const Char* d = std::lower_bound(OpenPunctuationPairBegin, OpenPunctuationPairEnd, c);
	return d != OpenPunctuationPairEnd && *d == c;
}

bool IkStringAlg::IsClosePunctuationPair(const Char c) {
	const Char* d = std::lower_bound(ClosePunctuationPairBegin, ClosePunctuationPairEnd, c);
	return d != ClosePunctuationPairEnd && *d == c;
}

bool IkStringAlg::IsPunctuationException(const Char c) {
	const Char* d = std::lower_bound(punctuationExceptionsBegin, punctuationExceptionsEnd, c);
	return d != punctuationExceptionsEnd && *d == c;
}

//TODO: Make these faster with binary search on sorted static strings?
bool IkStringAlg::IsPunctuation(Char chr)
{
  return (strPunctuation.find(chr)!=String::npos);
}
bool IkStringAlg::IsQuote(Char chr)
{
  return (strQuotes.find(chr)!=String::npos);
}
bool IkStringAlg::IsJpnSplit(const Char chr) 
{
  return (strJpnSplits.find(chr)!=String::npos);
}
bool IkStringAlg::IsSentenceSplit(Char chr)
{
	return (strSplits.find(chr) != String::npos);
}

// Detector for Japanese Chars:
// currently detected Japanese symbols:
//
// Hiragana (u3040-u309F)                    http://unicode.org/charts/PDF/U3040.pdf
// Katakana (u30A0-u30FF)				      http://unicode.org/charts/PDF/U30A0.pdf
// CJK Unified Ideographs (u4E00-u9FCF)      http://www.unicode.org/charts/PDF/U4E00.pdf
// CJK Symbols and Punctuation (u3000-u303F) http://unicode.org/charts/PDF/U3000.pdf
// Halfwidth & Fullwidth forms (uFF00-uFFEF) http://unicode.org/charts/PDF/UFF00.pdf
//   - the ascii equivalents of these are converted to ascii in the Preprocess() function.
//	 - the rest (U+FF66 until U+FFEE) is still treated as Japanese.
// Extra graphical symbols (u25A0-u25FF)  // Extra graphical symbols
//
bool IkStringAlg::IsJpnChar(const Char chr) 
{
  return (IkStringAlg::IsJpnSplit(chr) // Sentence splitter for JPN
  || IsKatakana(chr)
  || IsHiragana(chr)
  || ((chr>=0x4E00)&&(chr<=0x9FCF))    // CJK Unified
  || ((chr>=0x3000)&&(chr<=0x303F))    // CJK Symbols
  || ((chr>=0xFF66)&&(chr<=0xFFEF))    // Halfwidth & Fullwidth remaining chars
  || ((chr>=0x25A0)&&(chr<=0x25FF)));  // Extra graphical symbols
}

bool IkStringAlg::IsKatakana(const String& text)
{
	bool bIsKatakana = true;
	for (String::const_iterator iChar = text.begin(); iChar != text.end() && bIsKatakana; ++iChar) {
		bIsKatakana = IkStringAlg::IsKatakana(*iChar);
	}
	return bIsKatakana;
}

//Used to find the range of the "alphabetic" part of a word. Bit of a misnomer, since we want
//to include symbols like % and $.
struct AlphaFinder {
  bool operator()(Char c) const {
    //Quick check for lowercase ASCII and...
    //"_" used in preprocesser rewrite symbols
	if (u_iscntrl(c)) return false; // controls are not-printable
	// if (c<0x20) return false; // range of non-printable ascii symbols
	if ((c > 0x60 && c < 0x7B) || (c > 0x2F && c < 0x3A) || c == '-' || c == '_' || c == '(' || c == ')' || c == '[' || c == ']') return true; // Parenthesis are now considered part of the text.
	//Is it in the list of things to strip?
    return !std::binary_search(GetSortedMarkers().begin(), GetSortedMarkers().end(), c);
  }
};
struct DigitFinder : public unary_function<const Char, bool> {
	bool operator()(Char c) const { return (c >= 0x0030 && c <= 0x0039); }
};

//Convert any unicode character with a digit value (Nd) to its ASCII
//numeric value. 
void IkStringAlg::NormalizeDigits(String& strInput) {
  for (String::iterator i = strInput.begin(); i != strInput.end(); ++i) {
    if (*i < 0x0660) continue; //The first non-ASCII Nd character
    int val = u_charDigitValue(*i);
    if (val == -1) continue; //Not a digit
    *i = (Char)('0' + val); //Replace with ASCII equivalent
  }
}

//Normalized width: "Full Width" Latin characters/punctuation
//used in Japanese should be transformed into their ASCII values.
//"Half Width" katakana characters should be replaced with their
//full width counterparts.
void IkStringAlg::NormalizeWidth(String& strInput) {
  for (String::iterator i = strInput.begin(); i != strInput.end(); ++i) {
    if (*i < 0xFF01 || *i > 0xFFEE) continue; //Is it in the half/full width block at all?
    if (*i <= 0xFF5E) { //Is it a full width ASCII char?
      *i = *i - 0xFF01 + '!';
      continue;
    }
    //Full width white parens and half width Katakana
    if (*i <= 0xFF9F) { 
      static const Char katakana_narrow_to_wide[] = {
        '(', ')', 0x3002, 0x300C, 0x300D, 0x3001, 0x30FB, 0x30F2,
        0x30A1, 0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30E3, 0x30E5, 0x30E7,
        0x30C3, 0x30FC, 0x30A2, 0x30A4, 0x30A6, 0x30A8, 0x30AA, 0x30AB,
        0x30AD, 0x30AF, 0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9, 0x30BB,
        0x30BD, 0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8, 0x30CA, 0x30CB,
        0x30CC, 0x30CD, 0x30CE, 0x30CF, 0x30D2, 0x30D5, 0x30D8, 0x30DB,
        0x30DE, 0x30DF, 0x30E0, 0x30E1, 0x30E2, 0x30E4, 0x30E6, 0x30E8,
        0x30E9, 0x30EA, 0x30EB, 0x30EC, 0x30ED, 0x30EF, 0x30F3, 0x3099,
        0x309A
      };
	  static const Char katakana_single2char_to_wide1char_set1[] = {
		  0x30AC, 0x30AE, 0x30B0, 0x30B2, 0x30B4, 0x30B6, 0x30B8, 0x30BA, 0x30BC, 0x30BE, 0x30C0, 0x30C2, 0x30C5, 0x30C7, 0x30C9
	  };
	  static const Char katakana_single2char_to_wide1char_set2[] = {
		  0x30D0, 0x30D3, 0x30D6, 0x30D9, 0x30DC
	  };
	  static const Char katakana_single2char_to_wide1char_set3[] = {
		  0x30D1, 0x30D4, 0x30D7, 0x30DA, 0x30DD
	  };
	  if (strInput.end() - i > 1) { // check for 2 characters single with to 1 character double-width conversion
		  bool bFired = false;
		  if (*(i + 1) == 0xFF9E) {
			  if (*i >= 0xFF76 && *i <= 0xFF84) *i = katakana_single2char_to_wide1char_set1[*i - 0xFF76], bFired = true; // Set1
			  if (*i >= 0xFF8A && *i <= 0xFF8E) *i = katakana_single2char_to_wide1char_set2[*i - 0xFF8A], bFired = true; // Set2
			  if (*i == 0xFF73) *i = 0x30F4, bFired = true;
			  if (*i == 0xFF9C) *i = 0x30F7, bFired = true;
			  if (*i == 0xFF66) *i = 0x30FA, bFired = true;
		  }
		  if (*(i + 1) == 0xFF9F) {
			  if (*i >= 0xFF8A && *i <= 0xFF8E) *i = katakana_single2char_to_wide1char_set3[*i - 0xFF8A], bFired = true; // Set3
		  }
		  if (bFired) {
			*(i + 1) = '\0';
			++i; // skip 2
			continue;
		  }
	  }
      *i = katakana_narrow_to_wide[*i - 0xFF5F];
      continue;
    }
    //Half width Hangul. Skip.
    if (*i <= 0xFFDF) {
      continue;
    }
    //Miscelleny (note that 0xFFE7 is not a character!)
    static const Char misc_wide_to_narrow[] = {
      0x00A2, 0x00A3, 0x00AC, 0x00AF, 0x00A6, 0x00A5, 0x20A9, 0xFFE7,
      0x2502, 0x2190, 0x2191, 0x2192, 0x2193, 0x25A0, 0x25CB
    };
    *i = misc_wide_to_narrow[*i - 0xFFE0];
  }
  strInput.erase(std::remove(strInput.begin(), strInput.end(), '\0'), strInput.end()); // remove inserted nulls, if any
}

// Determines whether the specified code point is a control character
// (as defined by this function).
// A control character is one of the following:
// - ISO 8-bit control character (U+0000..U+001f and U+007f..U+009f)
// - U_CONTROL_CHAR (Cc)
// - U_FORMAT_CHAR (Cf)
// - U_LINE_SEPARATOR (Zl)
// - U_PARAGRAPH_SEPARATOR (Zp)
void IkStringAlg::RemoveControlChars(String& strInput)
{
	strInput.erase(std::remove_if(strInput.begin(),	strInput.end(), u_iscntrl), strInput.end());
}

bool IkStringAlg::IsText(const String& strInput) { // contains at least 
	for (String::const_iterator i = strInput.begin(); i != strInput.end(); ++i) {
		if (u_isblank(*i)) continue; // blancs are not text
		if (u_iscntrl(*i)) continue; // controls are not text
		return true; // not blanc *and* not controls is considered text input
	}
	return false;
}
bool IkStringAlg::IsText(const Char *begin, const Char *end) {
	for (const Char *i = begin; i != end; ++i) {
		if (u_isblank(*i)) continue; // blancs are not text
		if (u_iscntrl(*i)) continue; // controls are not text
		return true; // not blanc *and* not controls is considered text input
	}
	return false;
}

void IkStringAlg::Normalize(String& strInput, bool bLowerCase, bool bStripPunct)
{
  if (strInput.empty()) // nothing to normalize :-)
    return;

  bool bHasSpaces = (strInput.find(' ') != String::npos); // preprocessor inserted spaces, extra cleaning step

  //Try the fast algorithm. If it works, we don't need to do any more
  //character processing since we have pure ASCII. If it doesn't work,
  //do the more complicated ICU lower casing and normalization.
  if (bLowerCase) {
	  if (!FastAsciiToLower(strInput)) {
		  ToLower(strInput);
		  NormalizeDigits(strInput);
	  }
  }
  else {
	  NormalizeDigits(strInput);
  }
  if (!bStripPunct) // done: no punctuations to handle
	  return;

  const static size_t kMaxOutputSize = 1024;
  static Char output[kMaxOutputSize];
  size_t output_off = 0;

  //Currently, the maximum a string can grow in normalization is 2x, but
  //we'll leave some buffer just in case.
  //Unnormalizable strings are returned with just the character normalization,
  //not "spreading" of punctuation.
  //Doing this check now allows us to skip it as we process each character later.
  if (strInput.size() * 2 > kMaxOutputSize  - 16) return;

  //We'll look for the first and last alphabetic character
  //and not touch anything between them.

  String::iterator begin_alpha = std::find_if(strInput.begin(), strInput.end(), AlphaFinder());
  String::reverse_iterator rend_alpha = std::find_if(strInput.rbegin(), strInput.rend(), AlphaFinder());
  //If there is no last, there was no first, so we set last_alpha to the meaningless strInput.end()
  //Otherwise we convert it to a normal iterator (actually one past the last alpha) through the base() member.
  String::iterator end_alpha = rend_alpha != strInput.rend() ? rend_alpha.base() : strInput.end();
  bool bStartsWithDigit = false;
  bool bStartsWithDoubleDigits = false; 

  if (begin_alpha != strInput.end()) { // We do have alphanumericals...
	  if (DigitFinder()(*begin_alpha)) { // starts with numerical
		  bStartsWithDigit = true; // Indicator if text starts with numerics
		  String::iterator it_non_digit = std::find_if(begin_alpha + 1, end_alpha, not1(DigitFinder()));
		  bStartsWithDoubleDigits = ((it_non_digit - begin_alpha) == 2); // starts with double digits
	  }
	  // handle parenthesis
	  if (*begin_alpha == '(') { // starts with opening parenthesis
		  if (*(end_alpha - 1) == ')') { // ends with closing parenthesis
			  String::iterator has_non_digit = std::find_if(begin_alpha + 1, end_alpha - 1, not1(DigitFinder()));
			  if (has_non_digit != end_alpha - 1 || end_alpha - begin_alpha == 2) { // contains non digits, or empty
				  int cntOpen = 1; // handle uneven parenthesis
				  for (String::iterator it = begin_alpha + 1; it != end_alpha - 1; ++it) { // count parenthesis couples
					  if (*it == '(') ++cntOpen;
					  if (*it == ')') --cntOpen;
				  }
				  if (cntOpen >= 1) begin_alpha++;
				  if (cntOpen <= 1) end_alpha--;
			  }
		  }
		  else { // does not end with closing parenthesis
			  int cntOpen = 1, offsetLeft = 0; // number of open parenthesis
			  while ((end_alpha - begin_alpha >= 2) && *(begin_alpha + 1) == '(') ++begin_alpha, ++cntOpen, ++offsetLeft; // find the closest one
			  String::iterator it = begin_alpha + 1; // text cursor
			  for (; it != end_alpha && cntOpen != 0; ++it) {
				  if (*it == '(') ++cntOpen;
				  if (*it == ')') --cntOpen;
			  }
			  begin_alpha -= offsetLeft; // restore original begin
			  if (it == end_alpha) { // no closing
				  begin_alpha++;
				  while (begin_alpha < end_alpha && !AlphaFinder()(*begin_alpha)) ++begin_alpha; // correct alpha begin
			  }
		  }
	  }
	  else {
		  if (*(end_alpha - 1) == ')') { // ends with closing parenthesis
			  int cnt_close = 1;
			  for (String::iterator it = begin_alpha; it != end_alpha - 1; ++it) {
				  if (*it == ')') ++cnt_close;
				  if (*it == '(') --cnt_close;
			  }
			  if (cnt_close >= 1) while (end_alpha>begin_alpha && *(end_alpha - 1) == ')' && cnt_close >= 1) --end_alpha, --cnt_close; // strip off right side parenthesis
			  if (cnt_close <= 1 && *begin_alpha == '(') ++begin_alpha, --end_alpha; // if opening bracket, trim parenthesis.
			  while (begin_alpha != end_alpha && !AlphaFinder()(*begin_alpha)) ++begin_alpha; // strip off left side non-alphabetics
			  while (begin_alpha != end_alpha && !AlphaFinder()(*(end_alpha - 1))) --end_alpha; // strip off right side non-alphabetics
		  }
	  }
	  if (*begin_alpha == '[') { // starts with opening bracket
		  int cnt_open = 1;
		  for (String::iterator it = begin_alpha + 1; it != end_alpha; ++it) {
			  if (*it == ']') --cnt_open;
			  if (*it == '[') ++cnt_open;
		  }
		  if (cnt_open >= 1) while (*begin_alpha == '[' && cnt_open >= 1) ++begin_alpha, --cnt_open; // strip off left side brackets
		  if (cnt_open <= 1 && *(end_alpha - 1) == ']') ++begin_alpha, --end_alpha; // if closing bracket, trim brackets
		  while (begin_alpha != end_alpha && !AlphaFinder()(*begin_alpha)) ++begin_alpha; // strip off left side non-alphabetics
		  while (begin_alpha != end_alpha && !AlphaFinder()(*(end_alpha - 1))) --end_alpha; // strip off right side non-alphabetics
	  }
	  else {
		  if (end_alpha>begin_alpha && *(end_alpha - 1) == ']') { // ends with closing bracket
			  int cnt_close = 1;
			  for (String::iterator it = begin_alpha; it != end_alpha - 1; ++it) {
				  if (*it == ']') ++cnt_close;
				  if (*it == '[') --cnt_close;
			  }
			  if (cnt_close >= 1) while (*(end_alpha - 1) == ']' && cnt_close >= 1) --end_alpha, --cnt_close; // strip off right side brackets
			  if (cnt_close <= 1 && *begin_alpha == '[') ++begin_alpha, --end_alpha; // if opening bracket, trim brackets.
			  while (begin_alpha != end_alpha && !AlphaFinder()(*begin_alpha)) ++begin_alpha; // strip off left side non-alphabetics
			  while (begin_alpha != end_alpha && !AlphaFinder()(*(end_alpha - 1))) --end_alpha; // strip off right side non-alphabetics
		  }
	  }
  }
  //Special treatment of "." for abbreviations. Include it in the normalized token if it's the first
  //punctuation on the trailing non-alphanumeric part...
  if (end_alpha != strInput.end() && *end_alpha == '.') {
    ++end_alpha;
    //...But only if the next character is not also a '.'
    if (end_alpha != strInput.end() && *end_alpha == '.') --end_alpha;
  }
  // Process left non-alpha part 
  for (String::iterator i = strInput.begin(); i != begin_alpha; ++i) { // isolate punctuation symbols, remove everything else...
    if (IsPunctuation(*i)) {
      output[output_off++] = *i;
      output[output_off++] = ' ';
	}
	else {
		if (bStartsWithDigit) {
			if (*i == '.' && *(i+1) != '.') output[output_off++] = *i; // do not isolate the point if a number is following.
			bStartsWithDigit = false; // only copy once
			if (bStartsWithDoubleDigits) { // double digit for a start
				if (*i == '\'' && *(i + 1) != '\'') output[output_off++] = *i; // do not isolate single quote if a number is following.
				bStartsWithDoubleDigits = false;
			}
		}
	}
  }
  if (begin_alpha == strInput.end() && output_off) {  // If there's no content to follow, drop that last space
    --output_off;
  }
  //Copy the "content" of the token, but replace "<quote_or_split> " with " "
  for (String::iterator i = begin_alpha; i != end_alpha; ++i) { // Copy the "content" of the token
	  if (i + 1 == end_alpha || *(i + 1) != ' ' || !IsSplitOrQuote(*i)) {
		  if ((*i) == ' ') { // remove preceding SplitsOrQuotes
			  while (output_off > 0 && IsSplitOrQuote(output[output_off - 1]))
				  output_off--;
		  }
		  output[output_off++] = *i;
	  }
  }
  // Process right non-alpha part
  for (String::iterator i = end_alpha; i != strInput.end(); ++i) { // isolate punctuation symbols, remove everything else...
      if (IsPunctuation(*i)) {
        output[output_off++] = ' ';
        output[output_off++] = *i;
      }
  }
  strInput.replace(0, strInput.size(), &(output[0]), output_off);

  if (bHasSpaces) { // extra cleaning of parts
	  vector<String> space_split_index;
	  size_t start = strInput.find_first_not_of(' ');
	  size_t end;
	  while (start != String::npos) {
		  end = strInput.find(' ', start);
		  size_t len = end == base::String::npos ? base::String::npos : end - start;
		  space_split_index.push_back(strInput.substr(start, len));
		  start = (end == String::npos ? end : end + 1);
	  }
	  strInput.clear(); // reconstruct strIndex
	  for (size_t i = (size_t)0; i < space_split_index.size(); ++i) {
		  if (!strInput.empty() && *(strInput.end() - 1) != ' ')
			  strInput += ' '; // insert delimiting space
		  if (space_split_index[i].size() == 1) { // no need for isolation if single character
			  strInput += space_split_index[i]; // copy original
			  continue;
		  }
		  String::iterator it = std::find_if(space_split_index[i].begin(), space_split_index[i].end(), [](Char c) { return (c >= 0x0030 && c <= 0x0039); });
		  if (it != space_split_index[i].end()) { // if it contains digits, don't touch'
			  strInput += space_split_index[i]; // copy original
			  continue;
		  }
		  it = std::find_if(space_split_index[i].begin(), space_split_index[i].end(), [](Char c) { return IkStringAlg::IsPunctuation(c); });
		  if (it == space_split_index[i].end()) { // no punctuations found, do *not* split.
			  strInput += space_split_index[i]; // copy original
			  continue;
		  }
		  Char start_char = space_split_index[i][0];
		  if (start_char == '(') {
			  it = std::find_if(space_split_index[i].begin() + 1, space_split_index[i].end() - 1, [](Char c) { return c == ')'; });
			  if (it != space_split_index[i].end() - 1) { // closing bracket in text, do *not* split.
				  strInput += space_split_index[i]; // copy original
				  continue;
			  }
		  }
		  if (start_char == '[') {
			  it = std::find_if(space_split_index[i].begin() + 1, space_split_index[i].end() - 1, [](Char c) { return c == ']'; });
			  if (it != space_split_index[i].end() - 1) { // closing bracket in text, do *not* split.
				  strInput += space_split_index[i]; // copy original
				  continue;
			  }
		  }

		  {
			  static Char strIsolatePunctuation[128];
			  int idx = 0;
			  String strSubject(space_split_index[i]);
			  String::iterator begin_alpha = std::find_if_not(strSubject.begin(), strSubject.end(), [](Char c) { return (IkStringAlg::IsPunctuation(c) || IkStringAlg::IsSentenceSplit(c) || c == char16_t(0x00AB)); }); // char16_t(0x00AB) = "«"
			  String::reverse_iterator rend_alpha = std::find_if_not(strSubject.rbegin(), strSubject.rend(), [](Char c) { return (IkStringAlg::IsPunctuation(c) || IkStringAlg::IsSentenceSplit(c) || c == '\"' || c == char16_t(0x00BB)); }); // char16_t(0x00BB) = "»"
			  String::iterator end_alpha = rend_alpha != strSubject.rend() ? rend_alpha.base() : strSubject.end();
			  for (String::iterator it = strSubject.begin(); it != strSubject.end(); ++it) {
				  if (it < begin_alpha) { // left side punctuations
					  strIsolatePunctuation[idx++] = *it;
					  strIsolatePunctuation[idx++] = ' ';
				  }
				  if (it >= begin_alpha && it < end_alpha) { // middle token
					  strIsolatePunctuation[idx++] = *it;
				  }
				  if (it >= end_alpha) { // right side punctuations
					  strIsolatePunctuation[idx++] = ' ';
					  strIsolatePunctuation[idx++] = *it;
				  }
			  }
			  strIsolatePunctuation[idx] = '\0';
			  strInput += strIsolatePunctuation; // copy back
		  }
	  }
  }
}

String IkStringAlg::RemoveLeadingNonDigitWords(const String& strInput)
{
	// from left to right, find first digit symbol
	String::const_iterator it_first_digit = std::find_if(strInput.begin(), strInput.end(), DigitFinder());
	if (it_first_digit == strInput.end()) return strInput; // no digits detected
	// find start of word
	String::const_iterator it_first_digit_word = it_first_digit;
	while (it_first_digit_word != strInput.begin() && *(it_first_digit_word - 1) != static_cast<Char>(' '))
		--it_first_digit_word;
	
	return String(it_first_digit_word, strInput.end()); // copy word and consecutive words into output.
}

String IkStringAlg::RemoveLeadingWord(const String& strInput)
{
	// from left to right, find first space
	String::const_iterator it_first_space = std::find(strInput.begin(), strInput.end(), static_cast<Char>(' '));
	if (it_first_space == strInput.end()) return String(); // empy, no more words.
	return String(it_first_space + 1, strInput.end()); // we assume single space separators
}

void IkStringAlg::RemovePunctuationAndQuotes(String& strInput) {
  if (strInput.empty()) return;
  size_t firstNonPQ = 0;
  for (; firstNonPQ < strInput.size(); ++firstNonPQ) {
    if (!IsPunctuationOrQuote(strInput[firstNonPQ])) break;
  }
  size_t lastNonPQ = strInput.size() - 1;
  for (; lastNonPQ > firstNonPQ; --lastNonPQ) {
    if (!IsPunctuationOrQuote(strInput[lastNonPQ])) break;
  }
  if (firstNonPQ == 0 && lastNonPQ == strInput.size() - 1) return;
  if (firstNonPQ > 0) {
    strInput.replace(0, strInput.size(),
		     strInput.data() + firstNonPQ, lastNonPQ - firstNonPQ + 1);
  }
  else {
    strInput.erase(lastNonPQ + 1);
  }
} 

void IkStringAlg::RemoveSentenceMarkers(String& strInput)
{
  if (strInput.empty()) return;
  Trim(strInput, strSplitsAndPunctuation);
}

void IkStringAlg::RemoveAllMarkers(String& strInput) {
  if (strInput.empty()) return;
  Trim(strInput, strAllMarkers);
}


std::string IkStringAlg::ToString(int const & nInput)
{
  std::ostringstream strBuffer;
  strBuffer << nInput;
  return strBuffer.str();
}


std::string IkStringAlg::ToString(double const & fInput)
{
  std::ostringstream strBuffer;
  strBuffer << fInput;
  return strBuffer.str();
}

std::string IkStringAlg::ToString(unsigned long const & lnInput)
{
  std::ostringstream strBuffer;
  strBuffer << lnInput;
  return strBuffer.str();
}

int IkStringAlg::EndsWith(const std::string& strInput, const std::string& strEnd)
{
  size_t strEndSize = strEnd.length();
  if(strEndSize <= strInput.length()) {
    return (strInput.substr(strInput.length()-strEndSize, strEndSize) == strEnd);
  }
  else {
    return 0;
  } 
}

// in "c=9," : key selector would be "c=", return value = "9"
std::string IkStringAlg::GetMetaValue(const char* meta_data, const char* key)
{
	// String::const_iterator it_first_space = std::find(strInput.begin(), strInput.end(), static_cast<Char>(' '));
	const char* p_key = strstr(meta_data, key);
	if (p_key) {
		const char* p_val = p_key + strlen(key);
		return std::string(p_val,p_val+1); // value is supposed to be 1 digit
	}
 else
	return std::string("");
}

