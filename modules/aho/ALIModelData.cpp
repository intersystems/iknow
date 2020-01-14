#include "ModelData.h"
#include "ALIData.h"
#include "LexrepData.h"
#include "AhoCorasick.h"
#include "StaticModel.h"
#define QUOTE(x) #x
#define STR(x) QUOTE(x)

using namespace iknow::model;

using namespace iknow::model::MODELNS;

static const ALIData kALIData(ALIOneStateMap(),
			      ALINextStateMap(),
			      ALISymbolMap(),
			      ALIStateMap(),
			      ALIFailureTable(),
			      ALIOutput());


static const ALIDataModel kModel(kALIData);

void iknow::model::MODELNS::RegisterALI() {
  SetALIModel(STR(MODELID), MODELINDEX, &kModel);
}
