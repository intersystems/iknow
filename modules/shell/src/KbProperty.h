#ifndef IKNOW_SHELL_KBPROPERTY_H_
#define IKNOW_SHELL_KBPROPERTY_H_
#include "IkTypes.h"
#include "RawBlock.h"
#include "IkStringEncoding.h"
#include "IkStringAlg.h"
#include "IkKnowledgebase.h"
#include "OffsetPtr.h"

namespace iknow {
  namespace shell {
    class KbProperty {
    public:
      KbProperty(RawAllocator& allocator, iknow::core::PropertyId id, const std::string& name) :
        id_(id), 
        name_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(name))) { }
      KbProperty(const KbProperty& other) {
          this->id_ = other.id_;
          this->name_ = other.name_;
      }
      iknow::core::PropertyId Id() const {
        return id_;
      }
      iknow::base::String Name() const {
        return iknow::base::String(*name_);
      }
      const CountedBaseString* PointerToName() const {
        return name_;
      }
    private:
      iknow::core::PropertyId id_;
      OffsetPtr<const CountedBaseString> name_;
    };
  }
}


#endif //IKNOW_SHELL_KBPROPERTY_H_
