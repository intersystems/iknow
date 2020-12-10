#include "AhoCorasick.h"
#include "ModelRegistry.h"

namespace iknow {
  namespace model {

    typedef ModelRegistry<Model> LexrepModelRegistry;
    typedef ModelRegistry<ALIModel> ALIModelRegistry;

    typedef ModelRegistry<RawDataPointer> KbRawDataRegistry;

    static LexrepModelRegistry& GetModelRegistry() {
      static LexrepModelRegistry kModelRegistry;
      return kModelRegistry;
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

    static ALIModelRegistry& GetALIModelRegistry() {
        static ALIModelRegistry kALIModelRegistry;
        return kALIModelRegistry;
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

    static KbRawDataRegistry& GetKbRawDataRegistry() {
        static KbRawDataRegistry kKbRawDataRegistry;
        return kKbRawDataRegistry;
    }
    RawDataPointer GetRawDataPointer(const std::string& name, size_t index) {
        KbRawDataRegistry::Handle handle = GetKbRawDataRegistry().Lookup(name, index);
        if (handle == KbRawDataRegistry::kNoSuchHandle) return 0;
        return *GetKbRawDataRegistry().Retrieve(handle);
    }
    void SetRawDataPointer(const std::string& name, RawDataPointer* raw_data_address) {
            GetKbRawDataRegistry().Register(name, 0, raw_data_address);
    }
    void CleaRAWrModel(const std::string& name) {
        size_t i = 0;
        for (;;) {
            LexrepModelRegistry::Handle handle = GetKbRawDataRegistry().Lookup(name, i++);
            if (handle == LexrepModelRegistry::kNoSuchHandle) return;
            GetKbRawDataRegistry().Unregister(handle);
        }
    }

  }
}
