#ifndef IKNOW_SHELL_SHAREDMEMORYLANGUAGEBASE_H_
#define IKNOW_SHELL_SHAREDMEMORYLANGUAGEBASE_H_
#define NO_UCHAR_TYPE //Don't want sysCommon's "String" type
#include "LanguageBase.h"
#include "Export.h"
#include "OffsetPtr.h"
#ifdef ISC_IRIS
#include "utlCacheList.h"
#else
#include<vector>
typedef std::vector<std::string> CacheList;
#endif

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

    /*
    class SHELL_API FileLanguagebase : public AbstractLanguagebase {
    public:
        FileLanguagebase(const char* file_path, bool is_compiled=false); // constructor
        bool IsCompiled() {
            return is_compiled_;
        }
        std::string GetName() {
            return std::string("language");
        }
        iknow::base::String GetHash() {
            return iknow::base::SpaceString();
        }
        size_t EntryCount() {
            return entry_count_;
        }
        Key NextKey(Key key = 0) {
            if (key == token_weight_collection_.size())
                return 0;
            return ++key;
        }
        CacheList GetEntry(Key key) {
            CacheList entry = { "dummy" };
            entry.push_back(token_weight_collection_[key-1].first); // token
            entry.push_back(token_weight_collection_[key-1].second); // weight
            return entry;
        }
    private:
        size_t entry_count_;
        typedef std::pair<std::string, std::string> token_weight_unit;

        std::vector<token_weight_unit> token_weight_collection_;
        bool is_compiled_;
    };
    */

    class Obj;
    class RawAllocator;
    struct RawLBData;
    class SHELL_API SharedMemoryLanguagebase : public iknow::ali::LanguageBase {
    public:
      SharedMemoryLanguagebase(RawAllocator& allocator, AbstractLanguagebase& lb, bool is_compiled);
      SharedMemoryLanguagebase(RawLBData* lb_data);
      SharedMemoryLanguagebase(unsigned char* lb_data);
      SharedMemoryLanguagebase(void);
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
