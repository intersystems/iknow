#ifndef IKNOW_CORE_READER_H_
#define IKNOW_CORE_READER_H_
#include <algorithm>
#include <cctype>
#include "IkTypes.h"
#include "IkExportCore.h"
#include "Utility.h"

//Reader for splitting and classifying tokens for iKnow

namespace iknow {
  namespace core {
    namespace token {
      enum TokenType {
	kText = 0x0,
	kPunctuation = 0x1,
	kSpace = 0x2,
	kLine = 0x3,
	kControl = 0x4
      };
      template<typename IterT>
      struct Token {
	Token(IterT begin, IterT end, TokenType type) : begin_(begin), end_(end), type_(type) {}
	//For hashing
	bool operator==(const Token& other) const {
	  return std::equal(begin_, end_, other.begin_);
	}
	bool operator<(const Token& other) const {
	  return std::lexicographical_compare(begin_, end_, other.begin_, other.end_);
	}
	size_t size() const {
	  return end_ - begin_;
	}
	IterT begin_;
	IterT end_;
	TokenType type_;
      };

      template<typename IterT>
      size_t hash_value(const Token<IterT>& token) {
	return hash_range(token.begin_, token.end_) ^ std::hash<decltype(token.type_)>(token.type_);
      }
 
      template<typename CharT>
      TokenType GetTypeAscii(CharT c) {
	if ((c >= '0' && c <= '9') ||
	    (c >= 'A' && c <= 'Z') ||
	    (c >= 'a' && c <= 'z')) return kText;
	if (c == ' ' || c == '\t') return kSpace;
	if (c == '\r' || c == '\n' || c == '\f') return kLine;
	if (c < ' ') return kControl;
	return kPunctuation;
      }

      CORE_API TokenType GetTypeUnicode(iknow::base::Char c);

      template<typename CharT>
      TokenType GetType(CharT c) {
	if (c < 128) return GetTypeAscii(c);
	return GetTypeUnicode(c);
      }

      template<typename InIterT, typename OutOp>
      void SplitByClass(InIterT begin, InIterT end, OutOp& out) {
	TokenType last_type = kText;
	InIterT last_begin = begin;
	for (; begin != end; ++begin) {
	  TokenType type = GetType(*begin);
	  if (type != last_type) {
	    if (last_begin != begin) out(Token<InIterT>(last_begin, begin, last_type));
	    last_begin = begin;
	    last_type = type;
	  }
	}
	if (last_begin != end) out(Token<InIterT>(last_begin, end, last_type));
      }
    }
  }
}


#endif //IKNOW_CORE_READER_H_
