#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

#include "lexrep/Metadata.inl"

      size_t MaxWordCount() { return static_cast<size_t>(Metadata[0]); }
      
      bool IsIdeographic() { return static_cast<bool>(Metadata[1]); }

      bool HasRegex() { return static_cast<bool>(Metadata[2]); }
      
    }
  }
}
