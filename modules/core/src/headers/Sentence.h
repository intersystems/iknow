#ifndef IKNOW_CORE_SENTENCE_H_
#define IKNOW_CORE_SENTENCE_H_

//A Sentence is a subsequence of a Block, with some attributes for language certainty
//and summarization.

namespace iknow {
  namespace core {

    typedef double ALICertainty;
    class SummarizationAttributes {
      //TODO
    };
    class Sentence {
    public:
      Sentence(const Block& block, const iknow::base::IString& sentence, ALICertainty ali_certainty, const SummarizationAttributes& summarization_attributes) :
	block_(&block), sentence_(sentence), ali_certainty_(ali_certainty), summarization_attributes_(summarization_attributes) {}
    private:
      const Block* block_;
      iknow::base::IString sentence_;
      ALICertainty ali_certainty_;
      SummarizationAttributes summarization_attributes_;
    };
  }
}
#endif //IKNOW_CORE_SENTENCE_H_
