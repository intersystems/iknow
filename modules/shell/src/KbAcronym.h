#ifndef IKNOW_SHELL_KBACRONYM_H_
#define IKNOW_SHELL_KBACRONYM_H_
#include "IkTypes.h"
#include "RawBlock.h"
#include "IkStringEncoding.h"

namespace iknow {
  namespace shell {
    class KbAcronym {
    public:
      KbAcronym(RawAllocator& allocator, const std::string& token, bool is_sentence_end) :
	token_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(token))),
	is_sentence_end_(is_sentence_end) {}
      iknow::base::String Token() const {
	return iknow::base::String(*token_);
      }
      const CountedBaseString* PointerToToken() const {
	return token_;
      }
      bool IsSentenceEnd() const {
	return is_sentence_end_;
      }
    private:
      OffsetPtr<const CountedBaseString> token_;
      bool is_sentence_end_;
    };
  }
}


#endif //IKNOW_SHELL_KBACRONYM_H_
