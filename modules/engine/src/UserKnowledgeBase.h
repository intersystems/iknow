#pragma once

#include "IkTypes.h"
#include "IkKnowledgebase.h"

#include <unordered_map>

#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Metadata.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Acronym.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Regex.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Filter.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Label.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Lexrep.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_PreprocessFilter.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Rule.h"

/*
class CSV_DataGenerator; // forward declaration
class iKnow_KB_Label
{
public:
	std::string Name; // Name As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
	std::string Type; // Type As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE")[Required];
	std::string Attributes; // Attributes As %String(MAXLEN = 256, XMLPROJECTION = "ATTRIBUTE");
	std::string PhaseList; // Property PhaseList As %String;

private:
	static void LoadSpecialLabels(CSV_DataGenerator& kb);
	static iKnow_KB_Label LabelFromString(std::vector<std::string>& row_label, std::string& isDefault);
};
*/

namespace iknow {
	namespace csvdata {
		class UserKnowledgeBase {

		public:
			UserKnowledgeBase();
			~UserKnowledgeBase() {}

			// SharedMemoryKnowledgebase(RawAllocator& allocator, AbstractKnowledgebase& kb, bool is_compiled = false);
			unsigned char* generateRAW(bool IsCompiled);

			static std::vector<iKnow_KB_Metadata> kb_metadata;
			static std::vector<iKnow_KB_Acronym> kb_acronyms;
			static std::vector<iKnow_KB_Regex> kb_regex;
			static std::vector<iKnow_KB_Filter> kb_filter;
			static std::vector<iKnow_KB_Label> kb_labels;
			iKnow_KB_Label* kb_concept_label; // Property ConceptLabel As Label;
			typedef std::vector<iKnow_KB_Lexrep>	lexreps_Type;
			static lexreps_Type kb_lexreps;
			static std::unordered_map<std::string, int> lexrep_index;
			static std::vector<iKnow_KB_PreprocessFilter> kb_prepro;
			static std::vector<iKnow_KB_Rule> kb_rules;
			static const std::vector<std::pair<int, std::string>> kb_properties;
			typedef std::unordered_map<std::string, int>	labelIndexTable_type;
			labelIndexTable_type labelIndexTable; // mapping of labels to indexes

			size_t LabelCount() {
				return kb_labels.size();
			}
			iknow::base::String GetSpecialLabel(iknow::core::SpecialLabel label);

			std::string Hash; // Property Hash As% String;
			std::string GetHash() {
				return Hash;
			}
			bool IsDirty(void) {
				return m_IsDirty;
			}

		private:
			bool m_IsDirty; // if true, needs reloading
		};

	}
}

