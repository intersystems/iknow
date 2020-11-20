#ifndef IKNOW_AHO_MODELREGEX_H_
#define IKNOW_AHO_MODELREGEX_H_
#include "IkTypes.h"
#include "unicode/regex.h"
#include "utlExceptionFrom.h"
#include "IkStringEncoding.h"
#include <memory>
//#include <cstdio>
//#include <sys/time.h>

namespace iknow {
  namespace model {
    typedef iknow::base::String String;
    class Regex {
    public:
      //TODO: Ownership/Cleanup code now that these objects can be created dynamically, repeatedly by dynamic compiled model loading
      Regex() {}
      //We can't throw an exception while these are being built statically, so we keep the error state around for checking on use.
      //TODO: Check size when truncating to 32-bit
      Regex(const String& regex) :
		error_code_(U_ZERO_ERROR),
		pattern_(icu::RegexPattern::compile(icu::UnicodeString(regex.data(), static_cast<int32_t>(regex.size())), parse_error_, error_code_)), matcher_(pattern_->matcher(error_code_)) /* , regex_text_(regex) */ {
		if (error_code_ != U_ZERO_ERROR) {
			std::string regex_string = iknow::base::IkStringEncoding::BaseToUTF8(regex);
			throw ExceptionFrom<Regex>("Syntax error in regular expression : \"" + regex_string + "\"");
		}
      }
      template<typename IterT>
      IterT Match(IterT begin, IterT end) const {
		if ((end - begin) == 1 && (*begin | 040) >= 'a' && (*begin | 040) <='z') return begin;
		/*
			struct timeval tv_start;
			struct timezone tz_start;
			gettimeofday(&tv_start, &tz_start);
		*/
		//This is a read-only "intrusive" constructor: The UnicodeString will use the passed buffer and not free it when destroyed.
		icu::UnicodeString input(false /* not terminated */,
				 &*begin /* begin must reference an addressable UChar */,
				 static_cast<int32_t>(end - begin));
		UErrorCode err = U_ZERO_ERROR;
		matcher_->reset(input);
		if (U_FAILURE(err)) {
			throw ExceptionFrom<Regex>("Failed to construct regex matcher.");
		}
		UBool has_match = matcher_->lookingAt(err);
		if (U_FAILURE(err)) {
			throw ExceptionFrom<Regex>("Failed lookingAt in regex matcher.");
		}
		/*
			struct timeval tv_end;
			struct timezone tz_end;
			gettimeofday(&tv_end, &tz_end);
			long elapsed = ((tv_end.tv_sec * 1000000) + tv_end.tv_usec) - ((tv_start.tv_sec * 1000000) + tv_start.tv_usec);
			printf("\n\r%ld ###Matching [ %s ] to \"%s\"\n\r", elapsed, iknow::base::IkStringEncoding::BaseToUTF8(regex_text_).c_str(), iknow::base::IkStringEncoding::BaseToUTF8(String(begin, end)).c_str());
		*/
		if (!has_match) {
			return begin;
		}
		int32_t end_index = matcher_->end(err);
		if (U_FAILURE(err)) {
			throw ExceptionFrom<Regex>("Failed to find end of match in regex matcher.");
		}
		return begin + end_index;
      }
      ~Regex() {
		delete pattern_;
		delete matcher_;
      }
      Regex(const Regex& other) :
		parse_error_(other.parse_error_), error_code_(other.error_code_), pattern_(other.pattern_->clone()), matcher_(pattern_->matcher(error_code_)) {
	    if (error_code_ != U_ZERO_ERROR) {
	      throw ExceptionFrom<Regex>("Syntax error in regular expression.");
	    }
      }
    private:
      UParseError parse_error_;
      UErrorCode error_code_;
      icu::RegexPattern* pattern_;
      icu::RegexMatcher* matcher_;
      //String regex_text_;
    };
  }
}

#endif //IKNOW_AHO_MODELREGEX_H_
