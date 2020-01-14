#ifndef IKNOW_CORE_IFIND_H_
#define IKNOW_CORE_IFIND_H_
#include "Reader.h"
#include "IkTypes.h"
#include "BoostSupport.h"
#include "PoolAllocator.h"
#include <algorithm>
#include <vector>

namespace iknow {
  namespace core {

    typedef token::Token<const iknow::base::Char*> CharToken;
    
    class BasicProcessor {
    public:
      typedef int Id;
      typedef iknow::base::PoolAllocator<Id> IdAllocator;
      typedef std::vector<Id, IdAllocator> Ids;
      typedef boost_optional::pool_unordered_map<CharToken, Id>::type TokenIdMap;
      typedef iknow::base::PoolAllocator<CharToken> TokenAllocator;
      typedef std::vector<CharToken, TokenAllocator> Tokens;
      BasicProcessor(size_t reserve = 0) : max_id_(0), space_id_(-1), space_count_(0) {
	ids_.reserve(reserve);
	tokens_.reserve(reserve);
      }
      void operator()(const CharToken& token) {
	Id id;
	TokenIdMap::const_iterator it = token_id_map_.find(token);
	if (it == token_id_map_.end()) {
	  id = ++max_id_;
	  token_id_map_.insert(TokenIdMap::value_type(token, id));
	  tokens_.push_back(token);
	  //Special case " "
	  if (space_id_ == -1 && token.size() == 1 && *token.begin_ == ' ') {
	    space_id_ = id;
	    ++space_count_;
	  }
	}
	else {
	  id = it->second;
	  if (id == space_id_) ++space_count_;
	}
	ids_.push_back(id);
      }
      Ids::const_iterator ids_begin() const {
	return ids_.begin();
      }
      Ids::const_iterator ids_end() const {
	return ids_.end();
      }
      size_t ids_size() const {
	return ids_.size();
      }
      Tokens::const_iterator tokens_begin() const {
	return tokens_.begin();
      }
      Tokens::const_iterator tokens_end() const {
	return tokens_.end();
      }
      size_t tokens_size() const {
	return tokens_.size();
      }
      Id space_id() const {
	return space_id_;
      }
      size_t space_count() const {
	return space_count_;
      }
    private:
      Id max_id_;
      Id space_id_;
      Ids ids_;
      Tokens tokens_;
      TokenIdMap token_id_map_;
      size_t space_count_;
    };

    template<typename InIterT, typename OutOp>
    void iFindBasic(InIterT begin, InIterT end, OutOp& out) {
      //Windows defines a non-standard max macro, so we have to use "max" below without
      //the std qualifier.
#ifndef WIN32
      using std::max;
      using std::min;
#endif //WIN32
      //Allocate space in input size chunks
      using iknow::base::PoolAllocator;
      static const size_t kMinPoolSize = 16384;
      PoolAllocator<int>::Reset(max(static_cast<size_t>((end - begin) * 128), kMinPoolSize));
      {
	BasicProcessor basic_processor((end - begin) / 4);
	token::SplitByClass(begin, end, basic_processor);

	//Calculate the total occurrence count:
	//Equal to the number of occurrences, minus the count of space occurrences,
	//plus one if the first one is a space, because it won't be collapsed into its predecessor.
	size_t occurrence_count;
	if (basic_processor.ids_size()) {
	  occurrence_count = 
	    basic_processor.ids_size() - 
	    basic_processor.space_count() +
	    (*basic_processor.ids_begin() == basic_processor.space_id() ? 1 : 0);
	}
	else {
	  occurrence_count = 0;
	}
	out.BeginDocument(basic_processor.tokens_size(), occurrence_count);
	//Output token strings
	out.BeginTokens();
	for (BasicProcessor::Tokens::const_iterator i = basic_processor.tokens_begin();
	     i != basic_processor.tokens_end(); ++i) {
	  out.Token(i->begin_, i->end_);
	}
	out.EndTokens();

	//Output token types as a sequence of bytes
	out.BeginTypes();
	typedef iknow::base::PoolAllocator<char> CharAllocator;
	typedef std::vector<char, CharAllocator> TypeVector;
	static const size_t kTypeStringBlockSize = 32000;
	TypeVector types;
	types.reserve(min(basic_processor.tokens_size(), kTypeStringBlockSize));
	for (BasicProcessor::Tokens::const_iterator i = basic_processor.tokens_begin();
	     i != basic_processor.tokens_end(); ++i) {
	  types.push_back(static_cast<char>(i->type_));
	  if (types.size() >= kTypeStringBlockSize) {
	    out.TypeString(&*types.begin(), &*types.end());
	    types.clear();
	  }
	}
	if (!types.empty()) out.TypeString(&*types.begin(), &*types.end());
	out.EndTypes();

	//Output token occurrences, with negative values for "next is <space>"
	out.BeginOccurrences();
	for (BasicProcessor::Ids::const_iterator i = basic_processor.ids_begin();
	     i != basic_processor.ids_end(); ++i) {
	  BasicProcessor::Id id = *i;
	  bool is_last = i+1 == basic_processor.ids_end();
	  if (is_last || //last is always negative because there's nothing next!
	      *(i+1) != basic_processor.space_id()) {
	    id = -id; //tag as not followed by <space>
	  }
	  if (!is_last && id > 0) ++i; //skip <space>
	  out.Occurrence(id);
	}
	out.EndOccurrences();
	out.EndDocument();
      }
      //Not critical if we miss this. We'll just have some memory in the
      //pool until the next processing is done.
      PoolAllocator<int>::Clear();
    }
  }
}

#endif //IKNOW_CORE_IFIND_H_
