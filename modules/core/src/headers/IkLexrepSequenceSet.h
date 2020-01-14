#ifndef IKNOW_CORE_IKLEXREPSEQUENCESET_H_
#define IKNOW_CORE_IKLEXREPSEQUENCESET_H_
#include "IkLexrep.h"
#include <map>
#include <vector>
#include <utility>
#include "IkTypes.h"

#ifdef _WIN32
//TODO: TRW, Fix permanently, probably by way of pImpl
#pragma warning( disable: 4251 )
#endif //_WIN32

/*
  An IkLexrepSequenceSet is a set of sequences of lexreps (e.g. "NOT" "OVER" "THERE")

  It's designed for quick matching based on the assumption that most lexreps stored in a knowledge base
  are singular.

  All lexrep sequences that begin with a particular lexrep are stored together in a set, along with a value
  specifying the longest sequence in the set. If that value is one, and a signular lexrep is being looked up,
  we have it. If that value is greater than one, we iterate looking up the lexrep sequence trying to match the
  lesser of the count of lexreps or the longest sequence.
*/

namespace iknow {
  namespace core {

    typedef std::vector<const IkLexrep*> LexrepVector;

    typedef std::vector<iknow::base::String> TokenVector;

    //the sets returned via the initial prefix lookup
    class CORE_API IkLexrepSequences {
    public:
      ~IkLexrepSequences();
      size_t longest_sequence_length() const { return longest_sequence_length_; }
      //only valid if longest_sequence_length is 1!
      const IkLexrep* singular_lexrep() const { return singular_lexrep_; }
      //associate the given token vector with the given lexrep
      void Insert(const TokenVector& token_vector, const IkLexrep *lexrep);
      //returns the length of the longest match and the corresponding lexrep
      std::pair<size_t, const IkLexrep*> FindLongestMatch(const TokenVector& token_vector) const; 
    private: 
      size_t longest_sequence_length_;
      //cache if there's only one value
      const IkLexrep *singular_lexrep_;
      typedef std::map<TokenVector, const IkLexrep*> TokenVectorMap;
      TokenVectorMap token_vectors_;
    };

    class CORE_API IkLexrepSequenceSet {
    public:
      IkLexrepSequenceSet() : longest_sequence_length_(0) {};
      const static std::pair<size_t, const IkLexrep*> NotFound;
      void Insert(const IkLexrep* lexrep);
      size_t longest_sequence_length() const { return longest_sequence_length_; }
      //Finds a Lexrep that subsumes the maximum number of the given lexrep vector. returns the count subsumed
      //and a pointer to the subsuming lexrep.
      std::pair<size_t, const IkLexrep*> FindLongestMatch(const LexrepVector& lexrep_vector) const;
    private:
      typedef std::unordered_map<iknow::base::String, IkLexrepSequences> LexrepSequencePrefixMap;
      LexrepSequencePrefixMap prefix_map_;
      size_t longest_sequence_length_;
    };

}
}
#endif //IKNOW_CORE_IKLEXREPSEQUENCESET_H_
