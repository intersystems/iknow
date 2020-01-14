#ifndef IKNOW_AHO_MODELREGISTRY_H_
#define IKNOW_AHO_MODELREGISTRY_H_
#include <string>
#include <map>
#include <vector>
#include <utility>
#include "AhoCorasick.h"

namespace iknow {
  namespace model {
    template<typename T>
    class ModelRegistry {
    public:
      typedef unsigned int Handle;
      static const Handle kNoSuchHandle = static_cast<Handle>(-1);
      Handle Register(const std::string& name, size_t index, const T* model) {
	//Add the model to the vector, indexed by its handle
	model_vector_.push_back(model);
	//It's the last one now.
	Handle handle = static_cast<Handle>(model_vector_.size() - 1);
	//Associate the name with the handle.
	handle_map_[HandleMap::key_type(name, index)] = handle;
	return handle;
      }
      Handle Lookup(const std::string& name, size_t index = 0) {
        HandleMap::const_iterator i = handle_map_.find(HandleMap::key_type(name, index));
	if (i == handle_map_.end()) return kNoSuchHandle;
	if (!Retrieve(i->second)) return kNoSuchHandle; //is null
	return i->second;
      }
      void Unregister(Handle handle) {
	model_vector_.at(handle) = 0;
      }
      const T* Retrieve(Handle handle) {
	return model_vector_.at(handle);
      }
    private:
      typedef std::vector<const T*> ModelVector;
      ModelVector model_vector_;
      typedef std::map<std::pair<std::string, size_t>, Handle> HandleMap;
      HandleMap handle_map_;
   };
  }
}
#endif //IKNOW_AHO_MODELREGISTRY_H_
