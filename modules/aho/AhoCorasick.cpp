#include "AhoCorasick.h"
#include "ModelRegistry.h"

namespace iknow {
  namespace model {

    typedef ModelRegistry<Model> LexrepModelRegistry;
    typedef ModelRegistry<ALIModel> ALIModelRegistry;

    static LexrepModelRegistry& GetModelRegistry() {
      static LexrepModelRegistry kModelRegistry;
      return kModelRegistry;
    }
    static ALIModelRegistry& GetALIModelRegistry() {
      static ALIModelRegistry kALIModelRegistry;
      return kALIModelRegistry;
    }
    const Model* GetModel(const std::string& name, size_t index) {
      LexrepModelRegistry::Handle handle = GetModelRegistry().Lookup(name, index);
      if (handle == LexrepModelRegistry::kNoSuchHandle) return 0;
      return GetModelRegistry().Retrieve(handle);
    }
    
    const Model* SetModel(const std::string& name, size_t index, const Model* model) {
      GetModelRegistry().Register(name, index, model);
      return model;
    }

    void ClearModel(const std::string& name) {
      size_t i = 0;
      for(;;) {
	LexrepModelRegistry::Handle handle = GetModelRegistry().Lookup(name, i++);
	if (handle == LexrepModelRegistry::kNoSuchHandle) return;
	GetModelRegistry().Unregister(handle);
      }
    }

    const ALIModel* GetALIModel(const std::string& name, size_t index) {
      ALIModelRegistry::Handle handle = GetALIModelRegistry().Lookup(name, index);
      if (handle == ALIModelRegistry::kNoSuchHandle) return 0;
      return GetALIModelRegistry().Retrieve(handle);
    }
    
    const ALIModel* SetALIModel(const std::string& name, size_t index, const ALIModel* model) {
      GetALIModelRegistry().Register(name, index, model);
      return model;
    }

    void ClearALIModel(const std::string& name) {
      size_t i = 0;
      for (;;) {
	ALIModelRegistry::Handle handle = GetALIModelRegistry().Lookup(name, i++);
	if (handle == ALIModelRegistry::kNoSuchHandle) return;
	GetALIModelRegistry().Unregister(handle);
      }
    }
  }
}
