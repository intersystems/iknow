/*
** RegExServices.cpp
*/
// #include <fstream>
#include "RegExServices.h"
#include "IkStringAlg.h"

using iknow::base::String;
using iknow::base::IkStringEncoding;
using iknow::base::IkStringAlg;
using icu::UnicodeString;
using icu::RegexMatcher;
using namespace iknow::core;


RegExServices::RegExServices()
{
	pattern_ = NULL;
	matcher_ = NULL;
	matcher2_ = NULL;
}

RegExServices::~RegExServices()
{
	clean_regex();
}

bool RegExServices::SplitValueUnit(const String& valunit, String& value, String& unit)
{
		if (matcher_) {
			UErrorCode status = U_ZERO_ERROR;
			UnicodeString US_valunit(valunit.data(), static_cast<int32_t>(valunit.length()));
			matcher_->reset(US_valunit);
			UBool has_match = matcher_->matches(status);
			if (U_FAILURE(status)) {
				throw ExceptionFrom<RegExServices>("Failed lookingAt in regex matcher.");
			}
			if (!has_match) { // partial find ?
				if (matcher_->find()) { // partial match
					String stripped = IkStringAlg::RemoveLeadingNonDigitWords(valunit);
					if (stripped.length() < valunit.length()) { // stripped must be smaller than valunit
						if (SplitValueUnit(stripped, value, unit)) return true; // recursive second try
					}
					UnicodeString us_unit = matcher_->group(1, status);
					UnicodeString us_value = matcher_->group(2, status);
					if (us_unit.length() && us_value.length()) {
						value = String(us_value.getBuffer(), us_value.length());
						unit = String(us_unit.getBuffer(), us_unit.length());
						return true;
					}
				}
				return false;  // does not match
			}

			int32_t group_count = matcher_->groupCount();
			UnicodeString complete_match = matcher_->group(0, status);
			for (int i = 1; i <= group_count; ++i) {
				UnicodeString group_match = matcher_->group(i, status);
				iknow::base::String copy_match(group_match.getBuffer(), group_match.length());
				UnicodeString all_match = matcher_->group(0, status);
			}
			// int32_t start_index = matcher_->start(status);
			// int32_t end_index = matcher_->end(status);
			UnicodeString US_unit_1 = matcher_->group(1, status);
			UnicodeString US_unit_2 = matcher_->group(group_count, status);
			UnicodeString US_value = matcher_->group(2, status);
			iknow::base::String str_unit_1(US_unit_1.getBuffer(), US_unit_1.length());
			iknow::base::String str_unit_2(US_unit_2.getBuffer(), US_unit_2.length());
			iknow::base::String str_value(US_value.getBuffer(), US_value.length());
			iknow::base::String str_unit = str_unit_1 + str_unit_2;

			value = str_value;
			unit = str_unit;
			return true;

		}
		else
			return false;
}

int RegExServices::Parser2(const String& valunit, String& value, String& unit, String& value2, String& unit2)
{
		if (matcher2_ == NULL) return 0; // regex failed

		UErrorCode status = U_ZERO_ERROR;
		UnicodeString US_valunit(valunit.data(), static_cast<int32_t>(valunit.length()));
		value = String(), unit = String(), value2 = String(), unit2 = String(); // initialize return parameters
		matcher2_->reset(US_valunit);
		if (matcher2_->matches(status)) { // full match
			UnicodeString us_percent_range = matcher2_->group(1, status);
			if (us_percent_range.length()) { // 7%-8% is matching...
				UnicodeString us_value = matcher2_->group(2, status);
				UnicodeString us_unit = matcher2_->group(3, status);
				UnicodeString us_value2 = matcher2_->group(5, status);
				UnicodeString us_unit2 = matcher2_->group(6, status);
				value = String(us_value.getBuffer(), us_value.length());
				unit = String(us_unit.getBuffer(), us_unit.length());
				value2 = String(us_value2.getBuffer(), us_value2.length());
				unit2 = String(us_unit2.getBuffer(), us_unit2.length());
				return 4;
			}
			else {
				UnicodeString us_year_old = matcher2_->group(12, status);
				if (us_year_old.length()) { // xx-year-old is matching...
					UnicodeString us_value = matcher2_->group(13, status);
					UnicodeString us_unit = matcher2_->group(14, status);
					value = String(us_value.getBuffer(), us_value.length());
					unit = String(us_unit.getBuffer(), us_unit.length());
					return 2;
				}
			}
			return 0;
		}
		if (matcher2_->find()) { // partial match
			UnicodeString us_money_million = matcher2_->group(7, status);
			if (us_money_million.length()) {
				UnicodeString us_unit = matcher2_->group(8, status);
				UnicodeString us_value1 = matcher2_->group(9, status);
				// UnicodeString us_value2 = matcher_->group(10, status); // optional .xx
				UnicodeString us_value3 = matcher2_->group(11, status);
				value = String(us_value1.getBuffer(), us_value1.length())
					+ iknow::base::SpaceString()
					+ String(us_value3.getBuffer(), us_value3.length());

				unit = String(us_unit.getBuffer(), us_unit.length());
				return 2;
			}
		}
		return 0;
}

void RegExServices::swich_kb(const iknow::core::IkKnowledgebase* kb) 
{
	if (kb->GetMetadata<iknow::core::kLanguageCode>() != language_code_) { // need to switch language
			clean_regex();
			String val_unit_regex_ = kb->GetMetadata<iknow::core::kRegexSplitter>();
			if (val_unit_regex_ != String()) { // regex not empty
				UParseError parse_error_;
				UErrorCode error_code_(U_ZERO_ERROR);
				pattern_ = icu::RegexPattern::compile(icu::UnicodeString(val_unit_regex_.data(), static_cast<int32_t>(val_unit_regex_.size())), parse_error_, error_code_);
				if (U_FAILURE(error_code_)) return; // error in compile, DELVE code should check
				matcher_ = pattern_->matcher(error_code_);
				if (error_code_ != U_ZERO_ERROR) {
					throw ExceptionFrom<RegExServices>("Syntax error in regular expression.");
				}
				UErrorCode status = U_ZERO_ERROR;
				matcher2_ = new RegexMatcher("((\\d+)(%)(-| to )(\\d+)(%))|(([a-z$]+)(\\d+(\\.\\d+)?) (million|billion)?)|((\\d+|one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|million)\\-(year.*))", 0, status);

				if (U_FAILURE(status)) { // error in regex
					matcher2_ = NULL;
				}
			}
			language_code_ = kb->GetMetadata<iknow::core::kLanguageCode>();
		}
}

void RegExServices::clean_regex(void) 
{
		if (pattern_ != NULL) {
			delete pattern_;
			pattern_ = NULL;
		}
		if (matcher_ != NULL) {
			delete matcher_;
			matcher_ = NULL;
		}
		if (matcher2_ != NULL) {
			delete matcher2_;
			matcher2_ = NULL;
		}
}

