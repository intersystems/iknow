#include "CompiledLanguagebase.h"
#include "AhoCorasick.h"
#include "utlExceptionFrom.h"

using namespace iknow::shell;
using namespace iknow::model;

const iknow::model::ALIModel& CompiledLanguagebase::GetModel(const std::string& model_id) {
  const iknow::model::ALIModel* model = iknow::model::GetALIModel(model_id);
  if (!model) throw ExceptionFrom<CompiledLanguagebase>("No model found for model_id: " + model_id);
  return *model;
}

CompiledLanguagebase::CompiledLanguagebase(RawLBData* lb_data,  const std::string& model_id) :
  SharedMemoryLanguagebase(lb_data), model_(GetModel(model_id)) {
}

CompiledLanguagebase::CompiledLanguagebase(unsigned char* lb_data, const std::string& model_id) :
  SharedMemoryLanguagebase(lb_data), model_(GetModel(model_id)) {
}

CompiledLanguagebase::CompiledLanguagebase(SharedMemoryLanguagebase* sm_lb, const std::string& model_id) :
  SharedMemoryLanguagebase(sm_lb->RawData()), model_(GetModel(model_id)) {
}

double CompiledLanguagebase::GetSimilarity(const iknow::base::Char* sentence, size_t n) {
  return model_.ScoreText(sentence, sentence + n, TotalScore());
}
