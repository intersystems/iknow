#pragma once

#include "IkTypes.h"
#include "IkKnowledgebase.h"

#include <unordered_map>

#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_Metadata.h"
#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_Acronym.h"
#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_Regex.h"
#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_Filter.h"
#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_Label.h"
#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_Lexrep.h"
#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_PreprocessFilter.h"
#include "../../compiler/iKnowLanguageCompiler/iKnow_KB_Rule.h"

namespace iknow {
	namespace csvdata {
		class UserKnowledgeBase {

		public:
			UserKnowledgeBase();
			~UserKnowledgeBase() {}

			unsigned char* generateRAW(bool IsCompiled);

			std::vector<iKnow_KB_Metadata> kb_metadata;
			std::vector<iKnow_KB_Acronym> kb_acronyms;
			std::vector<iKnow_KB_Regex> kb_regex;
			std::vector<iKnow_KB_Filter> kb_filter;
			std::vector<iKnow_KB_Label> kb_labels;
			iKnow_KB_Label* kb_concept_label; // Property ConceptLabel As Label;
			typedef std::vector<iKnow_KB_Lexrep>	lexreps_Type;
			lexreps_Type kb_lexreps;
			std::unordered_map<std::string, int> lexrep_index;
			std::vector<iKnow_KB_PreprocessFilter> kb_prepro;
			std::vector<iKnow_KB_Rule> kb_rules;
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
			void clear(void) {
				kb_lexreps.clear();
				kb_acronyms.clear();
				m_IsDirty = true;
			}

			int addLexrepLabel(const std::string& token, const std::string& label) {
				bool bIsLabel = false;
				for (auto it = kb_labels.begin(); it != kb_labels.end() && !bIsLabel; ++it) {
					if (label == it->Name) bIsLabel = true;
				}
				if (!bIsLabel)
					return -1;
				
				kb_lexreps.push_back(iKnow_KB_Lexrep(token, label));
				m_IsDirty = true; // need recompilation
				return 0;
			}
			void addSEndCondition(const std::string& literal, const bool b_end) {
				kb_acronyms.push_back(iKnow_KB_Acronym(literal, b_end));
				m_IsDirty = true; // need recompilation
			}

		private:
			bool m_IsDirty; // if true, needs reloading

		};

	}
}

