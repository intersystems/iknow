#ifndef IKNOW_SHELL_DYNAMICMODEL_H_
#define IKNOW_SHELL_DYNAMICMODEL_H_
#include "lexrep/LexrepData.h"
#include "ali/ALIData.h"
#include "SharedMemoryKnowledgebase.h"
#include "SharedMemoryLanguagebase.h"
#include "AhoCorasick.h"
#include "Export.h"
#include <vector>

namespace iknow {
  namespace shell {   
    //This class stores the state machine table data for a dynamic model, in support of
    //a LexrepData object.
    class DynamicModelData {
    public:
      DynamicModelData(AbstractKnowledgebase& kb, size_t phase = 0);
      iknow::model::LexrepData GetLexrepData() const;
    private:
      //Convenience converter from a vector of <T> to a
      //pointer to T pointing to the first element of the
      //vector. Should obviously be used only after the vector
      //is fully constructed!
      template<typename T>
      const T* P(const std::vector<T>& v) const {
	return v.empty() ? 0 : &(v.front());
      }
      typedef std::vector<iknow::model::Symbol> Symbols;
      typedef std::vector<iknow::model::SmallState> States;
      typedef std::vector<iknow::model::SmallOffset> Offsets;
      typedef std::vector<iknow::model::Regex> Regexes;
      typedef std::vector<iknow::model::Match> Matches;
      typedef std::vector<iknow::model::SmallCount> Counts;
      //We need special storage for the Match label lists, which
      //are intrusive.
      typedef std::vector<iknow::model::LabelId> LabelIds;
      typedef std::vector<LabelIds> LabelIdsVec;
      void StoreMatchLabels();
      std::string name_;
      Symbols one_state_map_;
      States next_state_map_;
      Symbols symbol_map_;
      States state_map_;
      Regexes regex_map_;
      Offsets regex_one_state_map_;
      States regex_next_state_map_;
      Offsets regex_offset_map_;
      States regex_state_map_;
      States failure_table_;
      Matches matches_;
      LabelIdsVec labels_vec_;
      Offsets match_set_;
      Counts match_count_;
      size_t max_word_count_;
      bool has_regex_;
      bool is_ideographic_;
    };

    class SHELL_API DynamicModel : public iknow::model::DataModel {
    public:
      //The DynamicModel takes ownership of the model data
      DynamicModel(const DynamicModelData* data) :
	DataModel(data->GetLexrepData()), data_(data) {}
      static void Register(const std::string& name, AbstractKnowledgebase& kb);
      static void Unregister(const std::string& name);
      //We own it so we destroy it.
      ~DynamicModel() { delete data_; }
    private:
      const DynamicModelData* data_;
    };

    class DynamicALIModelData {
    public:
      DynamicALIModelData(AbstractLanguagebase& lb, size_t phase = 0);
      iknow::model::ALIData GetALIData() const;
    private:
      //Convenience converter from a vector of <T> to a
      //pointer to T pointing to the first element of the
      //vector. Should obviously be used only after the vector
      //is fully constructed!
      template<typename T>
      const T* P(const std::vector<T>& v) const {
	return &(v.front());
      }
      typedef std::vector<iknow::model::Symbol> Symbols;
      typedef std::vector<iknow::model::SmallState> States;
      typedef std::vector<iknow::model::Score> Scores;
      std::string name_;
      Symbols one_state_map_;
      States next_state_map_;
      Symbols symbol_map_;
      States state_map_;
      States failure_table_;
      Scores output_;
    };

    class SHELL_API DynamicALIModel : public iknow::model::ALIDataModel {
    public:
      //The DynamicModel takes ownership of the model data
      DynamicALIModel(const DynamicALIModelData* data) :
	ALIDataModel(data->GetALIData()), data_(data) {}
      static void Register(const std::string& name, AbstractLanguagebase& kb);
      static void Unregister(const std::string& name);
      //We own it so we destroy it.
      ~DynamicALIModel() { delete data_; }
    private:
      const DynamicALIModelData* data_;
    };

  }
}

#endif //IKNOW_SHELL_DYNAMICMODEL_H_
