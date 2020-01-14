#include "StaticModel.h"
namespace iknow {
  namespace model {
    namespace MODELNS {

      static const SmallState failure_table[] = {
#include "lexrep/FailureTable.inl"
      };

      const SmallState* FailureTable() { return failure_table; }
    }
  }
}
