#include "CompiledKnowledgebase.h"
#include "Scanner.h"
#include "IkLabel.h"
#include <iterator>
#include "AhoCorasick.h"
#include "utlExceptionFrom.h"
#include "IkStringAlg.h"
#ifdef ISC_IRIS
#include "DynamicModel.h"
#endif

using namespace iknow::shell;
using iknow::core::IkLexrep;
using iknow::core::Lexreps;
using iknow::core::FastLabelSet;
using iknow::base::String;
using std::vector;
using namespace iknow::model;

#define MODELNS(id,phase) id ## phase

#define MODELINITPHASE(id,phase)				\
  namespace iknow {						\
    namespace model {						\
      namespace MODELNS(id,phase) {				\
	void MODEL_API Register();				\
	void MODEL_API RegisterALI();				\
      }								\
    }								\
  }								\
  namespace iknow {						\
    namespace shell {						\
      namespace MODELNS(id,phase) {				\
        static int Register() {					\
          iknow::model::MODELNS(id,phase)::Register();		\
          iknow::model::MODELNS(id,phase)::RegisterALI();	\
          return 1;      					\
        }							\
        static const int ModelRegistered = Register();		\
      }								\
    }								\
  }

#define MODELINIT(id)				\
  MODELINITPHASE(id,0)				\
  MODELINITPHASE(id,1)				

MODELINIT(de)
MODELINIT(en)
MODELINIT(es)
MODELINIT(fr)
MODELINIT(ja)
MODELINIT(nl)
MODELINIT(pt)
MODELINIT(ru)
MODELINIT(sv)
MODELINIT(uk)
MODELINIT(cs)

RawDataPointer CompiledKnowledgebase::GetRawData(const std::string& name) {
    return iknow::model::GetRawDataPointer(name);
}

const Model* CompiledKnowledgebase::GetModel(const std::string& name, size_t index) {
  return iknow::model::GetModel(name, index);
}

bool CompiledKnowledgebase::IsCompiledModel(const std::string& name) {
  //Is there a 0-index model in the registry?
  const Model* model = iknow::model::GetModel(name, 0);
  return model ? 1 : 0;
}

//Construct the models vector from an already compiled, registered set of models
void CompiledKnowledgebase::ConstructModels(const std::string& model_id) {
  size_t index = 0;
  for(;;) {
    const Model* model = GetModel(model_id, index++);
    if (!model) return;
    models_.push_back(model);
  }
}

CompiledKnowledgebase::CompiledKnowledgebase(RawKBData* kb_data, const std::string& model_id) :
  SharedMemoryKnowledgebase(kb_data) {
  ConstructModels(model_id);
}

CompiledKnowledgebase::CompiledKnowledgebase(const unsigned char* kb_data, const std::string& model_id) :
    SharedMemoryKnowledgebase(kb_data) {
  ConstructModels(model_id);

  // SharedMemoryKnowledgebase skb = models_[0].;
}

CompiledKnowledgebase::CompiledKnowledgebase(SharedMemoryKnowledgebase* sm_kb, const std::string& model_id) :
  SharedMemoryKnowledgebase(sm_kb->RawData()) {
  ConstructModels(model_id);
}

IkLexrep CompiledKnowledgebase::NextLexrep(Lexreps::iterator& current, Lexreps::iterator end) const {
  if (!MoreLexrepsBuffered()) {
    if (buffered_lexrep_vector_.capacity() < static_cast<size_t>(end - current)) buffered_lexrep_vector_.reserve((end - current) * 2);
    
    Scanner scanner;
    scanner.Identify(current, end, std::back_inserter(buffered_lexrep_vector_), models_.begin(), models_.end(), this);
    current = end;
    next_output_ = buffered_lexrep_vector_.begin();
  }
  IkLexrep output = *next_output_;
  ++next_output_;
  if (next_output_ == buffered_lexrep_vector_.end()) {
    buffered_lexrep_vector_.clear();
  }
  return output;
}

size_t CompiledKnowledgebase::longest_lexrep_length() const {
  //TODO: Another hack... but maybe this isn't used?
  return (*models_.begin())->MaxWordCount();
}
