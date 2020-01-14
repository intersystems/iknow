#ifndef IKNOW_SHELL_COMPILEDLANGUAGEBASE_H_
#define IKNOW_SHELL_COMPILEDLANGUAGEBASE_H_
#include "SharedMemoryLanguagebase.h"
#include <string>

namespace iknow {
  namespace model {
    class ALIModel;
  }
}

namespace iknow {
  namespace shell {
    class SHELL_API CompiledLanguagebase : public SharedMemoryLanguagebase {
    public:
      CompiledLanguagebase(RawLBData* lb_data, const std::string& model_id);
      CompiledLanguagebase(unsigned char* lb_data, const std::string& model_id);
      CompiledLanguagebase(SharedMemoryLanguagebase* sm_lb, const std::string& model_id);
      double GetSimilarity(const iknow::base::Char* sentence, size_t n);
    private:
      const iknow::model::ALIModel& GetModel(const std::string& model_id);
      const iknow::model::ALIModel& model_;
      //explicitly disabled because Visual C++ warns
      //that it cannot be created due to the const reference
      //member
      void operator=(const CompiledLanguagebase& other);
    };
  }
}

#endif //IKNOW_SHELL_COMPILEDLANGUAGEBASE_H_
