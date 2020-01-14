#ifndef IKNOW_CORE_IKSTEMMER_H_
#define IKNOW_CORE_IKSTEMMER_H_

namespace iknow {
  namespace core {

    //Templated so we don't force a particular choice of input and output
    //types in this very simple interface. Basically, trying to avoid the need
    //for a std::string allocation/deallocation.
    template<typename InputT, typename OutputT>
    class IkStemmer {
    public:
      //From %IKPublic.inc
      enum LexType { Concept = 0, Relation = 1, NonRelevant = 2 };
      virtual OutputT Stem(const InputT& input, const InputT& /* language */, LexType /* type */) {
	//Default implementation just tries to convert the input to output.
	return OutputT(input);
      }
    };
  }
}

#endif //IKNOW_CORE_IKSTEMMER_H_
