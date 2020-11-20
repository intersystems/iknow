#pragma once
#ifdef WIN32
#pragma warning (disable: 4251)
#endif

#include "iKnow_KB_Metadata.h"
#include "iKnow_KB_Acronym.h"
#include "iKnow_KB_Regex.h"
#include "iKnow_KB_Filter.h"
#include "iKnow_KB_Label.h"
#include "iKnow_KB_Lexrep.h"
#include "iKnow_KB_PreprocessFilter.h"
#include "iKnow_KB_Rule.h"

#include <string>
#include <vector>
#include <unordered_map>

#include "IkTypes.h"
#include "IkKnowledgebase.h"

namespace iknow {
	namespace csvdata {

		class Predicate
		{
		public:
			virtual bool Check(iKnow_KB_Lexrep&) {
				return true;
			}
		};

		class CSV_DataGenerator
		{
		public:
			CSV_DataGenerator(std::string csv_path, std::string aho_path, std::string ldata_path);
			~CSV_DataGenerator();

			std::string Hash; // Property Hash As% String;
			std::string GetHash() {
				return Hash;
			}
			// Index NameIndex On Name[Unique];

			void loadCSVdata(std::string language = "en", bool IsCompiled = true, std::ostream& os = std::cout);
			void generateRAW(bool IsCompiled = true);
			void generateAHO(void);

			std::string GetName() {
				return kb_language;
			}
			// Utility functions
			std::vector<std::string> split_row(std::string row_text, char split = ';');
			std::string merge_row(std::vector<std::string>& row_vector, char split = ';');
			void handle_UTF8_BOM(std::ifstream& ifs);

			// The data vectors
			
			static std::vector<iKnow_KB_Metadata> kb_metadata;
			static std::vector<iKnow_KB_Acronym> kb_acronyms;
			static std::vector<iKnow_KB_Regex> kb_regex;
			static std::vector<iKnow_KB_Filter> kb_filter;
			static std::vector<iKnow_KB_Label> kb_labels;
			iKnow_KB_Label* kb_concept_label; // Property ConceptLabel As Label;
			typedef std::vector<iKnow_KB_Lexrep>	lexreps_Type;
			static lexreps_Type kb_lexreps;
			static std::unordered_map<std::string, int> lexrep_index;
			static std::unordered_map<std::string, std::vector<int>> lexrep_segments_index;
			static std::vector<iKnow_KB_PreprocessFilter> kb_prepro;
			static std::vector<iKnow_KB_Rule> kb_rules;
			static const std::vector<std::pair<int, std::string>> kb_properties;
			typedef std::unordered_map<std::string, int>	labelIndexTable_type;
			labelIndexTable_type labelIndexTable; // mapping of labels to indexes

			size_t LabelCount() {
				return kb_labels.size();
			}
			
			iknow::base::String GetSpecialLabel(iknow::core::SpecialLabel label);
			iknow::base::String GetRegexPattern(iknow::base::String regexName); // Method GetRegexPattern(regexName As %String) As %String
			std::string MetadataLookup(std::string key) { // Method MetadataLookup(key As %String) As %String
				std::string val; // Set val = ""
				// Set kbid = ..%Id()
				for_each(kb_metadata.begin(), kb_metadata.end(), [&key, &val](iKnow_KB_Metadata& meta){ if (meta.Name == key) val = meta.Val; }); // &sql(Select Val Into : val From Metadata Where Knowledgebase = : kbid And Name = : key)
				return val; // Quit val
			}

		private:
			std::string kb_language = "";
			std::string csv_path_;
			std::string aho_path_; 
			std::string language_data_path_; // C:\P4\projects\ikNLP_DEV\modules\iknow\standalone\language_data
			void CompileLexrepDictionaryPhase(/*kb As %iKnow.KB.Knowledgebase,*/ std::string phase, bool phasePredicate /*Predicate *phasePredicate*/, std::string& outputDir_);
			std::vector<int> CreateLabelsIndexVector(iKnow_KB_Lexrep& lexrep, std::unordered_map<std::string, int>& table);
		};
	}
}

