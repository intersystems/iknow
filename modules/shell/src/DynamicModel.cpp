#include "DynamicModel.h"
#include "utlExceptionFrom.h"
#include "AhoCorasick.h"
#include "IkTypes.h"
#include <map>
#include <utility>

namespace iknow {
  namespace shell {

    template<typename T>
    T CacheListItemAs(const CacheListItem& item) {
      //Default implementation just casts a numeric value
      //down to the type, as almost all tables are composed of
      //32-bit or smaller integers.
      return static_cast<T>(item.AsLongLong());
    }

    template<>
    iknow::model::Regex CacheListItemAs(const CacheListItem& item) {
#ifdef AIX
      //Annoying wchar_t = unsigned short on AIX
      WString wpattern = item.AsWString();
      iknow::base::String pattern(wpattern.begin(), wpattern.end());
      return iknow::model::Regex(pattern);
#else
      return iknow::model::Regex(item.AsC16String());
#endif
    }

    template<>
    iknow::model::Match CacheListItemAs(const CacheListItem& item) {
      //We'll create a match with a new[]'d label array and then fix them
      //up into the LabelIdsVec storage later.
      CacheList list = item.AsCacheList();
      CacheListIterator i = list.begin();
      if (i == list.end()) throw ExceptionFrom<DynamicModelData>("Ill-formed Match $LIST");
      iknow::model::LabelCount label_count = CacheListItemAs<iknow::model::LabelCount>(*i); ++i;
      if (i == list.end()) throw ExceptionFrom<DynamicModelData>("Ill-formed Match $LIST");
      iknow::model::WordCount word_count = CacheListItemAs<iknow::model::WordCount>(*i); ++i;
      iknow::model::LabelId* labels = new iknow::model::LabelId[label_count];
      for (size_t j = 0; j < label_count; ++j) {
        if (i == list.end()) { 
          delete[] labels; 
          throw ExceptionFrom<DynamicModelData>("Ill-formed Match $LIST");
        }
        labels[j] = CacheListItemAs<iknow::model::LabelId>(*i); ++i;
      }
	  if (i != list.end()) { // optional metadata string
		  const std::string* meta_data = new std::string((*i).AsAString());
		  return iknow::model::Match(labels, label_count, word_count, meta_data->c_str());
	  }
      return iknow::model::Match(labels, label_count, word_count);
    }

    template<typename M, typename T>
    void LoadTable(M& m, size_t phase, const std::string& name, std::vector<T>& vec) {
      m.ReadTable(name, phase);
      while (!m.TableAtEnd()) {
	//TODO: Tune kFetch Count for performance
	const static size_t kFetchCount = 256;
	CacheList list = m.FetchRows(kFetchCount);
	for (CacheListIterator i = list.begin(); i != list.end(); ++i) {
	  vec.push_back(CacheListItemAs<T>(*i));
	}
      }
      m.CloseTable();
    }

    void DynamicModelData::StoreMatchLabels() {
      labels_vec_.reserve(matches_.size());
      for (Matches::iterator i = matches_.begin(); i != matches_.end(); ++i) {
	const iknow::model::LabelId* labels = i->label_id;
	//Add a vector of labels equal to the match. Should never be
	//reallocated (and thus change address) if the labels_vec is never
	//reallocated.
	labels_vec_.push_back(LabelIds(labels, labels + i->label_count));
	i->label_id = &(*labels_vec_.back().begin());
	//Kill the old match labels
	delete[] labels;
      }
    }

    DynamicModelData::DynamicModelData(AbstractKnowledgebase& kb, size_t phase) {
      if (!kb.IsDynamic()) {
	throw ExceptionFrom<DynamicModelData>("Cannot crate a dynamic model from this abstract knowledgebase.");
      }
      name_ = kb.GetName();
      LoadTable(kb, phase, "OneStateMap", one_state_map_);
      LoadTable(kb, phase, "NextStateMap", next_state_map_);
      LoadTable(kb, phase, "SymbolMap", symbol_map_);
      LoadTable(kb, phase, "StateMap", state_map_);
      LoadTable(kb, phase, "RegexMap", regex_map_);
      LoadTable(kb, phase, "RegexOneStateMap", regex_one_state_map_);
      LoadTable(kb, phase, "RegexNextStateMap", regex_next_state_map_);
      LoadTable(kb, phase, "RegexOffsetMap", regex_offset_map_);
      LoadTable(kb, phase, "RegexStateMap", regex_state_map_);
      LoadTable(kb, phase, "FailureTable", failure_table_);
      LoadTable(kb, phase, "Matches", matches_);
      //The Matches are constructed by LoadTable with raw heap pointers.
      //We want to capture these into the LabelIdsVec so their storage will
      //be managed with this object.
      StoreMatchLabels();
      LoadTable(kb, phase, "MatchSet", match_set_);
      LoadTable(kb, phase, "MatchCount", match_count_);
      std::vector<size_t> metadata;
      LoadTable(kb, phase, "Metadata", metadata);
      //TODO: Compiled model metadata needs a more robust
      //interface
      max_word_count_ = metadata[0];
      is_ideographic_ = static_cast<bool>(metadata[1]);
      has_regex_ = static_cast<bool>(metadata[2]);
    }

    DynamicALIModelData::DynamicALIModelData(AbstractLanguagebase& lb, size_t phase) {
      if (!lb.IsDynamic()) {
	throw ExceptionFrom<DynamicALIModelData>("Cannot crate a dynamic model from this abstract languagebase.");
      }
      name_ = lb.GetName();
      LoadTable(lb, phase, "OneStateMap", one_state_map_);
      LoadTable(lb, phase, "NextStateMap", next_state_map_);
      LoadTable(lb, phase, "SymbolMap", symbol_map_);
      LoadTable(lb, phase, "StateMap", state_map_);
      LoadTable(lb, phase, "FailureTable", failure_table_);
      LoadTable(lb, phase, "Output", output_);
    }

    iknow::model::LexrepData DynamicModelData::GetLexrepData() const {
      return iknow::model::LexrepData(P(one_state_map_),
				      P(next_state_map_),
				      P(symbol_map_),
				      P(state_map_),
				      P(regex_map_),
				      P(regex_one_state_map_),
				      P(regex_next_state_map_),
				      P(regex_offset_map_),
				      P(regex_state_map_),
				      P(failure_table_),
				      P(matches_),
				      P(match_set_),
				      P(match_count_),
				      max_word_count_,
				      has_regex_,
				      is_ideographic_);
    }

    iknow::model::ALIData DynamicALIModelData::GetALIData() const {
      return iknow::model::ALIData(P(one_state_map_),
				   P(next_state_map_),
				   P(symbol_map_),
				   P(state_map_),
				   P(failure_table_),
				   P(output_));
    }
    

    
    //The physical storage of the dynamic models is maintained here, as
    //the ModelRegistry just deals with interface pointers.
    typedef std::pair<std::string, size_t> ModelId;

    typedef std::map<ModelId, DynamicModel*> DynamicModelMap;
    
    static DynamicModelMap& ModelMap() {
      static DynamicModelMap map;
      return map;
    }

    void DynamicModel::Unregister(const std::string& name) {
      iknow::model::ClearModel(name);
      size_t i = 0;
      for(;;) {
	DynamicModelMap::iterator model = ModelMap().find(std::make_pair(name, i++));
	if (model == ModelMap().end()) return;
	delete model->second;
	ModelMap().erase(model);
      }
    }

    void DynamicModel::Register(const std::string& name, AbstractKnowledgebase& kb) {
      Unregister(name);
      size_t phase_count = kb.PhaseCount();
      for (size_t i=0; i < phase_count; ++i) {
	DynamicModel* model = new DynamicModel(new DynamicModelData(kb, i));
	ModelMap()[std::make_pair(name, i)] = model;
	iknow::model::SetModel(name, i, model);
      }
    }

    //TODO: Very annoying cut+paste job here.

    typedef std::map<ModelId, DynamicALIModel*> DynamicALIModelMap;
    
    static DynamicALIModelMap& ALIModelMap() {
      static DynamicALIModelMap map;
      return map;
    }

    void DynamicALIModel::Unregister(const std::string& name) {
      iknow::model::ClearALIModel(name);
      size_t i = 0;
      for (;;) {
	DynamicALIModelMap::iterator model = ALIModelMap().find(std::make_pair(name, i++));
	if (model == ALIModelMap().end()) return;
	delete model->second;
	ALIModelMap().erase(model);
      }
    }

    void DynamicALIModel::Register(const std::string& name, AbstractLanguagebase& lb) {
      Unregister(name);
      size_t phase_count = lb.PhaseCount();
      for (size_t i=0; i < phase_count; ++i) {
	DynamicALIModel* model = new DynamicALIModel(new DynamicALIModelData(lb, i));
	ALIModelMap()[std::make_pair(name, i)] = model;
	iknow::model::SetALIModel(name, i, model);
      }
    }
    
  }
}
