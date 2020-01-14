#include "Reader.h"
#include "unicode/uchar.h"

namespace iknow {
  namespace core {
    namespace token {
      TokenType GetTypeUnicode(iknow::base::Char c) {
	if (u_isalnum(c)) return kText;
	if (u_isUWhiteSpace(c)) return kSpace;
	if (u_iscntrl(c)) return kControl;
	return kPunctuation;
      }
    }
  }
}
