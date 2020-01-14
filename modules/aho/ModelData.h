#ifndef IKNOW_AHO_MODEL_DATA_H_
#define IKNOW_AHO_MODEL_DATA_H_
#include "ModelId.h"
#include "AhoCorasick.h"

namespace iknow {
  namespace model {
    namespace MODELNS {
      //Initialize and registery this mode
      MODEL_API void Register();
      MODEL_API void RegisterALI();
      class ModelData {
	const static Model* kModelPtr;
      };
    }
  }
}



#endif //IKNOW_AHO_MODEL_DATA_H_
