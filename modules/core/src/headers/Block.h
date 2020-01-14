#ifndef IKNOW_CORE_BLOCK_H_
#define IKNOW_CORE_BLOCK_H_
#include "IkTypes.h"
#include "IkKnowledgebase.h"

//An Block is a segment of an IkBuffer processed with a single knowledgebase.

namespace iknow {
  namespace core {
    class Block {
    public:
      Block(const iknow::base::IString& block, const IkKnowledgebase* kb) : block_(block), kb_(kb) {}
    private:
      iknow::base::IString block_;
      const IkKnowledgebase* kb_;
    };
  }
}

#endif //IKNOW_CORE_BLOCK_H_
