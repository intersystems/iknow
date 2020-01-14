#ifndef IKNOW_SHELL_KERNELINTERFACE_H_
#define IKNOW_SHELL_KERNELINTERFACE_H_
#include "IkTypes.h"
#include "Export.h"
#define ZF_DLL
#include "callin.h"

namespace iknow {
  namespace shell {
    //The iKnow Cache kernel interface
    class SHELL_API iKnowKernel {
    public:
      static unsigned char* LoadData(const iknow::base::String& name, void* udata, CACHE_IKNOWLOADFUNC load_func, CACHE_IKNOWRELEASEFUNC release_func);
      static void UnloadData(unsigned char* data);
      static void Initialize(CACHE_IKNOWFUNCSP funcs);
      static void SetError(const std::string& message_id, const std::string& message_val);
    };
  }
}

#endif //IKNOW_SHELL_KERNELINTERFACE_H_
