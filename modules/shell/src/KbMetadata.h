#ifndef IKNOW_SHELL_KBMETADATA_H_
#define IKNOW_SHELL_KBMETADATA_H_
namespace iknow {
  namespace shell {
    class KbMetadata {
    public:
      KbMetadata(RawAllocator& allocator, const std::string& name, const std::string& val) :
	name_(allocator.InsertString(name)),
	val_(allocator.InsertString(iknow::base::IkStringEncoding::UTF8ToBase(val))) {}
      
      std::string Name() const {
	return std::string(name_->data(), name_->size);
      }
      const CountedCharString* PointerToName() const {
	return name_;
      }
      iknow::base::String Val() const {
	return iknow::base::String(val_->data(), static_cast<size_t>(val_->size));
      }
      const CountedBaseString* PointerToVal() const {
	return val_;
      }
    private:
      OffsetPtr<const CountedCharString> name_;
      OffsetPtr<const CountedBaseString> val_;
    };
  }
}
#endif //IKNOW_SHELL_KBMETADATA_H_
