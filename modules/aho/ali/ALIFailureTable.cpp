#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState ali_failure_table[] = {
#include "ali/FailureTable.inl"
      };

      const SmallState* ALIFailureTable() { return ali_failure_table; }
    }
  }
}
