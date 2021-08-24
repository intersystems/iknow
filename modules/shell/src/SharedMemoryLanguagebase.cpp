#include "SharedMemoryLanguagebase.h"
#include "RawBlock.h"
#include "StaticHashTable.h"
#include <algorithm>
#include "IkStringEncoding.h"
#include <map>
#include <unordered_map>
#include <fstream>

using namespace iknow::base;
using namespace iknow::shell;
using namespace iknow::shell::StaticHash;

using std::for_each;
using std::ifstream;

namespace iknow {
  namespace shell {
    struct RawLBData {
      RawLBData() : total_score(0) {}
      typedef Table<String, size_t> ScoreTable;
      OffsetPtr<ScoreTable> scores;
      size_t total_score;
    };
  }
}

//Sum entries. We have to build an intermediate map for duplicates.
#if defined(LINUX) || defined(UNIX)
typedef std::map<String, size_t> ScoreMap;
#else
typedef std::unordered_map<String, size_t> ScoreMap;
#endif
struct EntryInserter {
  EntryInserter(ScoreMap& score_map) : score_map_(score_map) {}
  void operator()(CacheList& entry) {
#ifdef ISC_IRIS
    size_t score = entry[3].AsLong();
    score_map_[IkStringEncoding::UTF8ToBase(entry[2].AsAString())] += score;
#else
    size_t score = std::stol(entry[3 - 1]);
    score_map_[IkStringEncoding::UTF8ToBase(entry[2 - 1])] += score;
#endif
  }
  ScoreMap& score_map_;
private:
  void operator=(const EntryInserter&);
};


//The final score for each token is based on its original score ordered position
static void Flatten(const ScoreMap& input, ScoreMap& output)
{
  // make a multimap based where the score is indexed.
  // highest scores first
  typedef std::multimap<size_t, String, std::greater<size_t> > ScoreOrder;
  ScoreOrder score_order;
  for (ScoreMap::const_iterator i = input.begin(); i != input.end(); ++i) {
    score_order.insert(ScoreOrder::value_type(i->second, i->first));
  }

  size_t count = score_order.size();
  for (ScoreOrder::const_iterator i = score_order.begin(); i != score_order.end(); ++i) {
    output.insert(ScoreMap::value_type(i->second, count));
    count--;
  }
}


struct TableBuilder {
  typedef Builder<String, size_t> BuilderT;
  TableBuilder(RawAllocator& allocator, BuilderT& builder) : allocator_(allocator), builder_(builder) {}
  void operator()(const std::pair<const String, size_t>& p) {
    builder_.Insert(allocator_.InsertString(p.first), allocator_.Insert(p.second));
  }
  RawAllocator& allocator_;
  BuilderT& builder_;
private:
  void operator=(const TableBuilder&);
};

/*
FileLanguagebase::FileLanguagebase(const char* lang, bool is_compiled) : entry_count_(0), is_compiled_(is_compiled)
{
    if (!is_compiled) {
        std::string lb_path = std::string("C:\\Users\\jdenys\\source\\repos\\iknow\\ali_models\\") + lang + ".lb";
        ifstream ifs = ifstream(lb_path, ifstream::in);
        if (ifs.is_open()) {
            // kb.handle_UTF8_BOM(ifs);

            int count = 0; // Set count = 0
            for (std::string line; getline(ifs, line);) // while ('stream.AtEnd) { // Set line = $ZCONVERT(stream.ReadLine(), "O", "UTF8")
            {
                if (line.empty())
                    continue;
                size_t separator = line.find(';');
                if (separator == std::string::npos) // invalid line
                    continue;
                std::string token = std::string(&line[0], &line[separator]);
                std::string weight = std::string(&line[separator + 1], &line[line.length()]);

                token_weight_collection_.push_back(std::make_pair(token, weight)); // collect
                ++count; // Set count = count + 1
            }
            ifs.close();
            entry_count_ = (size_t)count;
        }
    }
    else
        entry_count_ = 20000; // compiled is fixed at 20000
}
*/

SharedMemoryLanguagebase::SharedMemoryLanguagebase(RawAllocator& allocator,AbstractLanguagebase& lb, bool is_compiled) {
  //The RawLBData must be the first thing in the block, we'll use its address for the base of all
  //OffsetPtrs in the structure.
  lb_data_ = allocator.Insert(RawLBData());
  SetOffset();
  size_t entry_count = lb.EntryCount();
  if (!is_compiled) {
     ScoreMap original_score_map;
     EntryInserter entry_inserter(original_score_map);
     AbstractLanguagebase::Key key = lb.NextKey();
     while (key) {
       //TODO: Clean up all the copying that must be going on here.
       CacheList list(lb.GetEntry(key));
       entry_inserter(list);
       key = lb.NextKey(key);
     }
     ScoreMap flat_score_map;
     Flatten(original_score_map, flat_score_map);
     TableBuilder::BuilderT builder(flat_score_map.size());
     TableBuilder table_builder(allocator, builder);
     for_each(flat_score_map.begin(), flat_score_map.end(), table_builder);
     lb_data_->scores = allocator.Insert(builder.Build(allocator));
  }
  lb_data_->total_score = static_cast<unsigned long>(entry_count * (entry_count + 1) / 2);  
}

SharedMemoryLanguagebase::SharedMemoryLanguagebase(void) {
    const int kRawSize = 100;
    static unsigned char buf_[kRawSize];
    iknow::shell::Raw raw(buf_, kRawSize);
    iknow::shell::RawAllocator allocator(raw);
    lb_data_ = allocator.Insert(RawLBData());
    size_t entry_count = 20000;
    lb_data_->total_score = static_cast<unsigned long>(entry_count * (entry_count + 1) / 2);
}
SharedMemoryLanguagebase::SharedMemoryLanguagebase(RawLBData* lb_data) : lb_data_(lb_data) {}
SharedMemoryLanguagebase::SharedMemoryLanguagebase(unsigned char* lb_data) : lb_data_(reinterpret_cast<RawLBData*>(lb_data)) {}

//All member functions must set the offset ptr
void SharedMemoryLanguagebase::SetOffset() const {
  SetBasePointer(reinterpret_cast<unsigned char*>(lb_data_));
}

size_t SharedMemoryLanguagebase::MinClusterSize() const {
  return 4; //hardcoded!
}

size_t SharedMemoryLanguagebase::MaxClusterSize() const {
  return 4;
}

size_t SharedMemoryLanguagebase::ScoreFor(const Char* c, size_t n) const {
  SetOffset();
  size_t* score = (size_t*)lb_data_->scores->Lookup(c, c + n);
  return score ? *score : 0;
}

size_t SharedMemoryLanguagebase::TotalScore() const {
  SetOffset();
  return lb_data_->total_score;
}
