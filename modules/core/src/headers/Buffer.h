#ifndef IKNOW_CORE_BUFFER_H_
#define IKNOW_CORE_BUFFER_H_
#include "IkTypes.h"

//An Buffer represents a block of undifferentiated text.
namespace iknow {
  namespace core {
    class Buffer {
    public:
      explicit Buffer(const iknow::base::IString& buf) : buf_(buf) {}
    private:
      iknow::base::IString buf_;
    };
  }
}

#endif //IKNOW_CORE_BUFFER_H_
