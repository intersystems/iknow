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



namespace iknow {
	namespace model {
		namespace MODELNS {

#if MODELINDEX == 0
#ifdef _IRIS
			static RawDataPointer kLanguageData = NULL; // empty version
#else
			static const unsigned char kb_data[] = { // memory block representing KB data
#include "kb_data.inl"
			};
			static RawDataPointer kLanguageData = &kb_data[0]; // empty version
#endif
#endif
		}
	}
}

static const DataModel kModel(kLexrepData);

void iknow::model::MODELNS::Register() {
  SetModel(STR(MODELID), MODELINDEX, &kModel);
#if MODELINDEX == 0 // RAW datablock only stored once
  SetRawDataPointer(STR(MODELID), &kLanguageData);
#endif
}

