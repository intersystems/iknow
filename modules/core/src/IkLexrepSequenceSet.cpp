#include "IkLexrepSequenceSet.h"
#include "IkStringAlg.h"
#include <algorithm>

using namespace iknow::core;
using iknow::base::String;
using iknow::base::SpaceString;

#ifndef _WIN32
//Visual C++ 2008 doesn't have these in std:: yet
#define max std::max
#define min std::min
#endif //_WIN32

IkLexrepSequences::~IkLexrepSequences() {
  //In the Ik model, containers of pointers are responsible for deleting
  //their contents
  for (TokenVectorMap::iterator i = token_vectors_.begin(); i != token_vectors_.end(); ++i) {
    delete i->second;
  }
}

void IkLexrepSequences::Insert(const TokenVector& token_vector, const IkLexrep *lexrep) {
  //note we don't check if it actually belongs in this set (share a common prefix)
  //we assume IkLexrepSequenceSet is doing the right thing.
  longest_sequence_length_ = max(longest_sequence_length_, token_vector.size());
  //cache if there's only one lexrep here
  if (longest_sequence_length_ == 1) singular_lexrep_ = lexrep;
  //delete the existing Lexrep pointer if present.
  TokenVectorMap::iterator i = token_vectors_.find(token_vector);
  if (i != token_vectors_.end()) {
    delete i->second;
    token_vectors_.erase(i);
  }
  token_vectors_.insert(TokenVectorMap::value_type(token_vector, lexrep));
}

std::pair<size_t, const IkLexrep*> IkLexrepSequences::FindLongestMatch(const TokenVector& token_vector) const {
  size_t max_match_length = min(token_vector.size(), longest_sequence_length_);

  //TODO, TRW: use STL algorithms
  for(size_t i=max_match_length; i > 0; --i) {
    TokenVector test(token_vector.begin(), token_vector.begin() + i);
    TokenVectorMap::const_iterator p = token_vectors_.find(test);
    if (p != token_vectors_.end()) return std::make_pair(i, p->second);
  }

  //not found
  return IkLexrepSequenceSet::NotFound;
}

struct LexrepPtrToValue {
  String operator() (const IkLexrep* pLexrep) { String val(pLexrep->GetNormalizedValue()); return val; } //TODO: TRW, Why does GetNormalizedValue return a reference??
};

//I'd use IkStringAlg's version of this but it's too baroque: I don't want an IkArrayList (of pointers to IkObjects...)

struct AddToVector {
  TokenVector& token_vector_;
  AddToVector(TokenVector& token_vector) : token_vector_(token_vector) {}
  void operator()(const String& token) {
    token_vector_.push_back(token);
  }
private:
  void operator=(const AddToVector&);
};

static void SpaceSplitValue(const String& value, TokenVector& output) {
  AddToVector add_to_vector(output);
  iknow::base::IkStringAlg::Tokenize(value, String::value_type(' '), add_to_vector);
}

void IkLexrepSequenceSet::Insert(const IkLexrep* lexrep) {
  TokenVector tokens;
  tokens.reserve(4); //TODO: TRW, find the constant for the max # of words in a lexrep
  SpaceSplitValue(lexrep->GetNormalizedValue(), tokens);
  if (!tokens.size()) {
     //don't insert 0-length lexreps
    delete lexrep;
    return;
  }
  String prefix_token = tokens[0];
  prefix_map_[prefix_token].Insert(tokens, lexrep);
  longest_sequence_length_ = max(longest_sequence_length_, tokens.size());
}

typedef std::pair<size_t, const IkLexrep*> MatchT;

const MatchT IkLexrepSequenceSet::NotFound = MatchT(0, (const IkLexrep*)0);

std::pair<size_t, const IkLexrep*> IkLexrepSequenceSet::FindLongestMatch(const LexrepVector& lexrep_vector) const {
  if (!lexrep_vector.size()) return NotFound; //can't match against empty list
  //use the first normalized value as a prefix
  String val(lexrep_vector[0]->GetNormalizedValue());
  LexrepSequencePrefixMap::const_iterator p = prefix_map_.find(val);
  if (p == prefix_map_.end()) return NotFound;
  //if there's only one lexrep here of length 1, return it from the cache
  if (p->second.longest_sequence_length() == 1) return MatchT(1, p->second.singular_lexrep());
  //otherwise build a vector of normalized values
  TokenVector tokens;
  tokens.resize(lexrep_vector.size());
  std::transform(lexrep_vector.begin(), lexrep_vector.end(), tokens.begin(), LexrepPtrToValue());
  return p->second.FindLongestMatch(tokens);
}
