#ifndef IKNOW_SHELL_KBLEXREP_H_
#define IKNOW_SHELL_KBLEXREP_H_
#include "IkTypes.h"
#include "KbLabel.h"
#include "RawBlock.h"
#include "IkStringEncoding.h"
#include "IkStringAlg.h"
#include "OffsetPtr.h"
#include <algorithm>

namespace iknow {
  namespace shell {
    struct token_cnt_symbols {
      token_cnt_symbols() : max_symbols_(0) {}
      void operator()(const iknow::base::String& token) {
        if (token.size() > max_symbols_)
          max_symbols_=token.size();
      }
      size_t max_symbols_;
    };

    class KbLexrep {
    public:
      template<typename MapT>
      KbLexrep(RawAllocator& allocator, const MapT& label_map, const std::string& token, const std::string& labels, const std::string meta=std::string()) : 
	    token_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(token))),
        meta_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(meta))) {
	    IndexPattern label_string;
	    AddIndexForLabelNameToString<MapT> adder(label_map, label_string);
	    iknow::base::IkStringAlg::Tokenize(labels, ';', adder);
	    labels_ = allocator.InsertString(label_string);
      }
      size_t TokenCount() {
	//It's painful how out of date the standard Rogue Wave STL implementation
	//is in Sun Studio.
#ifdef SOLARIS
	size_t n = 0;
	std::count(token_->begin(), token_->end(), (unsigned short)' ', n);
	return n + 1;
#else
	return std::count(token_->begin(), token_->end(), (unsigned short)' ') + 1;
#endif
      }
      iknow::base::String Token() const {
        return iknow::base::String(*token_);
      }
      const CountedBaseString* PointerToToken() const {
        return token_;
      }
      iknow::base::String Meta() const {
        return iknow::base::String(*meta_);
      }
      const CountedBaseString* PointerToMeta() const {
        return meta_;
      }
      IndexPattern Labels() const {
	    return IndexPattern(*labels_);
      }
      const CountedIndexString* PointerToLabels() const {
	    return labels_;
      }
      size_t maxTokenSize() const {
        struct token_cnt_symbols cnt_symbols;
        iknow::base::String lexrep=*token_;
        iknow::base::IkStringAlg::Tokenize(lexrep, ' ', cnt_symbols);
        return (size_t) cnt_symbols.max_symbols_;
      }

    private:
      OffsetPtr<const CountedBaseString> token_;
      OffsetPtr<const CountedBaseString> meta_;
      OffsetPtr<const CountedIndexString> labels_;
    };
  }
}


#endif //IKNOW_SHELL_KBLEXREP_H_
