#ifndef IKNOW_SHELL_COMPILEDKNOWLEDGEBASE_H_
#define IKNOW_SHELL_COMPILEDKNOWLEDGEBASE_H_
#include "SharedMemoryKnowledgebase.h"
#include <string>
#include <vector>

namespace iknow {
  namespace model {
    class Model;
    typedef const unsigned char* RawDataPointer;
  }
}

namespace iknow {
  namespace shell {
    class SHELL_API CompiledKnowledgebase : public SharedMemoryKnowledgebase {
    public:
      static const iknow::model::Model* GetModel(const std::string& name, size_t index);
      static bool IsCompiledModel(const std::string& name);
      static iknow::model::RawDataPointer GetRawData(const std::string& name);
      CompiledKnowledgebase(RawKBData* kb_data, const std::string& model_id);
      CompiledKnowledgebase(const unsigned char* kb_data, const std::string& model_id);
      CompiledKnowledgebase(SharedMemoryKnowledgebase* sm_kb, const std::string& model_id);
      iknow::core::IkLexrep NextLexrep(iknow::core::Lexreps::iterator& current, iknow::core::Lexreps::iterator end) const;
      bool MoreLexrepsBuffered() const { return !buffered_lexrep_vector_.empty(); }
	  void ResetLexrepsBuffer() const { buffered_lexrep_vector_.clear(); } // needed for reentrancy
	  size_t longest_lexrep_length() const;
   private:
      void ConstructModels(const std::string& model_id);
      typedef std::vector<iknow::core::IkLexrep> LexrepVector;
      mutable LexrepVector buffered_lexrep_vector_;
      mutable LexrepVector::iterator next_output_;
      typedef std::vector<const iknow::model::Model*> ModelVector;
      ModelVector models_;
      //explicitly disabled because Visual C++ warns
      //that the reference member  makes it impossible to generate.
      void operator=(const CompiledKnowledgebase& other);
    };
  }
}

#endif //IKNOW_SHELL_COMPILEDKNOWLEDGEBASE_H_
