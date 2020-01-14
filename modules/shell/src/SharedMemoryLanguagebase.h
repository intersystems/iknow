#ifndef IKNOW_SHELL_SHAREDMEMORYLANGUAGEBASE_H_
#define IKNOW_SHELL_SHAREDMEMORYLANGUAGEBASE_H_
#define NO_UCHAR_TYPE //Don't want sysCommon's "String" type
#include "LanguageBase.h"
#include "Export.h"
#include "OffsetPtr.h"
#include "utlCacheList.h"

namespace iknow {
  namespace shell {

    class AbstractLanguagebase {
    public:
      typedef size_t Key;
      virtual bool IsCompiled() = 0;
      virtual std::string GetName() = 0;
      virtual iknow::base::String GetHash() = 0;
      virtual size_t EntryCount() = 0;
      virtual Key NextKey(Key key = 0) = 0;
      virtual CacheList GetEntry(Key key) = 0;
      //These will have default dummy implementations for KBs that don't support
      //dynamic loading.
      virtual bool IsDynamic()  { return false; }
      virtual size_t PhaseCount() { return 0; }
      virtual void ReadTable(const std::string&, size_t) { }
      virtual bool TableAtEnd() { return true; }
      virtual CacheList FetchRows(size_t) { return CacheList(); }
      virtual void CloseTable() { }
    };

    class Obj;
    class RawAllocator;
    struct RawLBData;
    class SHELL_API SharedMemoryLanguagebase : public iknow::ali::LanguageBase {
    public:
      SharedMemoryLanguagebase(RawAllocator& allocator, AbstractLanguagebase& lb, bool is_compiled);
      SharedMemoryLanguagebase(RawLBData* lb_data);
      SharedMemoryLanguagebase(unsigned char* lb_data);
      unsigned char* RawData() { return reinterpret_cast<unsigned char*>(lb_data_); }
    protected:
      //The minimum and maximum cluster sizes for this language base.
      size_t MinClusterSize() const;
      size_t MaxClusterSize() const;
      //Score the cluster at c with length n
      size_t ScoreFor(const iknow::base::Char* c, size_t n) const;
      //Total score for all clusters in this language base.
      size_t TotalScore() const;
    private:
      void SetOffset() const;
      RawLBData* lb_data_;
    };
  }
}

#endif //IKNOW_SHELL_SHAREDMEMORYLANGUAGEBASE_H_
