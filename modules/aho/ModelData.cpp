#include "ModelData.h"
#include "LexrepData.h"
#include "AhoCorasick.h"
#include "StaticModel.h"
#define QUOTE(x) #x
#define STR(x) QUOTE(x)

using namespace iknow::model;

using namespace iknow::model::MODELNS;

static const LexrepData kLexrepData(OneStateMap(),
				    NextStateMap(),
				    SymbolMap(),
				    StateMap(),
				    RegexMap(),
				    RegexOneStateMap(),
				    RegexNextStateMap(),
				    RegexOffsetMap(),
				    RegexStateMap(),
				    FailureTable(),
				    Matches(),
				    MatchSet(),
				    MatchesCount(),
				    MaxWordCount(),
				    HasRegex(),
				    IsIdeographic());

static const DataModel kModel(kLexrepData);

void iknow::model::MODELNS::Register() {
  SetModel(STR(MODELID), MODELINDEX, &kModel);
}

