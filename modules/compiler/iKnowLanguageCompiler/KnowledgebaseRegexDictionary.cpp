#include "KnowledgebaseRegexDictionary.h"
#include "CSV_DataGenerator.h"

using namespace iknow::AHO;
using namespace iknow::base;
using namespace std;

String KnowledgebaseRegexDictionary::Lookup(String name) { // Method Lookup(name As %String) As %String
	return Knowledgebase->GetRegexPattern(name); // Quit ..Knowledgebase.GetRegexPattern(name)
}

KnowledgebaseRegexDictionary::KnowledgebaseRegexDictionary()
{
}


KnowledgebaseRegexDictionary::~KnowledgebaseRegexDictionary()
{
}
