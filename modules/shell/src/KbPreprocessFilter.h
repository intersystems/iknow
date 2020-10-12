#ifndef IKNOW_SHELL_KBPREPROCESSFILTER_H_
#define IKNOW_SHELL_KBPREPROCESSFILTER_H_
#include "IkTypes.h"
#include "RawBlock.h"
#include "IkStringEncoding.h"
#include "IkPreprocessFilter.h"
#include "Reader.h"
#include "IkStringAlg.h"
#include "OffsetPtr.h"
#include <algorithm>

namespace iknow {
  namespace shell {
    
    class KbPreprocessFilter {
    public:
      enum Type {
		kExact,				//exact match of whole token in between space symbols (as original)
		kExactInNonText,	//exact match of whole token in beween everything non-text (punctuations)
		kAtBegin,			//match at begining of token
		kAtEnd,				//match at end of token
		kEverywhere			//match anywhere
      };
      KbPreprocessFilter(RawAllocator& allocator, const std::string& input_token, const std::string& output_token) :
		output_token_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(output_token))) {
			if (input_token.empty()) throw ExceptionFrom<KbPreprocessFilter>("Empty preprocess filter.");
			iknow::base::String pattern = iknow::base::IkStringEncoding::UTF8ToBase(input_token);
			bool separator_is_punctuation = false, match_begin = false, match_end = false;
			if (pattern[0] == '~') {
				separator_is_punctuation = true;
				pattern.erase(0, 1);
			}
			if (pattern[0] == '\\') {
				match_begin = true;
				pattern.erase(0,1);
			}
			if (pattern[pattern.size() - 1] == '\\') {
				match_end = true;
				pattern.erase(pattern.size() - 1);
			}
			input_token_ = allocator.InsertString(pattern);
			if (match_begin && match_end) {	type_ = kExact;	}
			else if (match_begin) {	type_ = kAtBegin; }
			else if (match_end) { type_ = kAtEnd; }
			else { type_ = kEverywhere;	}
			if (type_ == kExact && separator_is_punctuation) type_ = kExactInNonText;
      }
	  KbPreprocessFilter(const KbPreprocessFilter& other) { // explicit copy constructor
		  this->input_token_ = other.input_token_;
		  this->output_token_ = other.output_token_;
		  this->type_ = other.type_;
	  }
      iknow::base::String InputToken() const { 	return iknow::base::String(*input_token_); }
      const CountedBaseString* PointerToInputToken() const { return input_token_; }
      iknow::base::String OutputToken() const { return iknow::base::String(*output_token_); }
      const CountedBaseString* PointerToOutputToken() const { return output_token_; }
      Type ApplicationType() const { return type_; }
      void ApplyImpl(iknow::base::String& str) const {
		switch (type_) {
		case kAtBegin:
			if (!str.compare(0, PointerToInputToken()->size, PointerToInputToken()->data(), PointerToInputToken()->size)) {
				str.replace(str.begin(), str.begin() + PointerToInputToken()->size,	PointerToOutputToken()->data(), PointerToOutputToken()->size);
			}
		break;
		case kAtEnd:
			if (!str.compare(str.size() - PointerToInputToken()->size, PointerToInputToken()->size, PointerToInputToken()->data(), PointerToInputToken()->size)) {
				str.replace(str.size() - PointerToInputToken()->size, PointerToInputToken()->size, PointerToOutputToken()->data(), PointerToOutputToken()->size);
			}
		break;
		case kEverywhere:
		case kExact:
		case kExactInNonText:
			size_t pos = str.find(PointerToInputToken()->data(), 0, PointerToInputToken()->size);
			while (pos != iknow::base::String::npos) { // Confirm non-text surrounded.
				if (type_ == kExact) { 
					if (!((pos == 0 || str[pos - 1] == ' ') && (pos + PointerToInputToken()->size == str.size() || str[pos + PointerToInputToken()->size] == ' '))) {
						pos = str.find(PointerToInputToken()->data(), pos + 1, PointerToInputToken()->size); // Not correctly surrounded, continue
						continue;
					}
				}
				else if (type_ == kExactInNonText) { 
					if (!((pos == 0 || iknow::core::token::GetType(str[pos - 1]) != iknow::core::token::kText) &&
						(pos + PointerToInputToken()->size == str.size() || iknow::core::token::GetType(str[pos + PointerToInputToken()->size]) != iknow::core::token::kText))) {
							pos = str.find(PointerToInputToken()->data(), pos + 1, PointerToInputToken()->size); // Not correctly surrounded, continue
							continue;
					}
				}
				str.replace(pos, PointerToInputToken()->size, PointerToOutputToken()->data(), PointerToOutputToken()->size);
				pos = str.find(PointerToInputToken()->data(), pos + PointerToOutputToken()->size, PointerToInputToken()->size);  // recompute iterator because it's invalidated by the replace.
			}
		}
      }
      void Apply(iknow::base::String& str) const {
		if (str.find(PointerToInputToken()->data(), 0, PointerToInputToken()->size) == iknow::base::String::npos) return;	//quick check if it's there at all.
		ApplyImpl(str);
      }
    private:
      OffsetPtr<const CountedBaseString> input_token_;
      OffsetPtr<const CountedBaseString> output_token_;
      Type type_;
    };
  }
}


#endif //IKNOW_SHELL_KBPREPROCESSFILTER_H_
