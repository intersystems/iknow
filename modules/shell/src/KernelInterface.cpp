#include "KernelInterface.h"
#include "utlExceptionFrom.h"
#include "callin.h"

using namespace iknow::shell;
using namespace iknow::base;

static CACHE_IKNOWFUNCSP iknow_funcs = 0;

static CACHE_IKNOWFUNCSP GetIKnowFuncs() {
  if (!iknow_funcs) throw ExceptionFrom<iKnowKernel>("iKnow kernel functions uninitialized.");
  return iknow_funcs;
}

unsigned char* iKnowKernel::LoadData(const iknow::base::String& name, void* udata, CACHE_IKNOWLOADFUNC load_func, CACHE_IKNOWRELEASEFUNC release_func) {
  CACHE_IKNOWSTRING id;
  //Absurd name size limit.
  if (name.size() > 8192) {
    throw ExceptionFrom<iKnowKernel>("Model name too long.");
  }
  //Now safe to cast
  id.string_len = static_cast<int>(name.size());
  id.string = reinterpret_cast<const unsigned short*>(name.data());
  CACHE_IKNOWDATA block;
  int result = GetIKnowFuncs()->LoadData(&id, udata, load_func, release_func, &block);
  if (result != CACHE_SUCCESS) {
    const char* message_id;
    const char* message_val;
    GetIKnowFuncs()->GetError(&message_id, &message_val);
    throw MessageExceptionFrom<iKnowKernel>(message_id, message_val);
  } 

  return block.data;
}

void iKnowKernel::UnloadData(unsigned char* data) {
  int result = GetIKnowFuncs()->UnloadData(data);
  if (result != CACHE_SUCCESS) {
    const char* message_id;
    const char* message_val;
    GetIKnowFuncs()->GetError(&message_id, &message_val);
    throw MessageExceptionFrom<iKnowKernel>(message_id, message_val);
  } 
}

void iKnowKernel::Initialize(CACHE_IKNOWFUNCSP funcs) {
  iknow_funcs = funcs;
}

void iKnowKernel::SetError(const std::string& message_id, const std::string& message_val) {
  GetIKnowFuncs()->SetError(message_id.c_str(), message_val.c_str());
}
