#pragma once

#include "SharedMemoryKnowledgebase.h"

#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Metadata.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Acronym.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Regex.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Filter.h"
// #include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Label.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Lexrep.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_PreprocessFilter.h"
#include "..\..\compiler\iKnowLanguageCompiler\iKnow_KB_Rule.h"

class UserKnowledgeBase : public iknow::shell::AbstractKnowledgebase {
	bool IsCompiled() {
		return false;
	}
	std::string GetName() {
		return "en";
	}
	iknow::base::String GetHash() {
		return iknow::base::IkStringEncoding::UTF8ToBase("hash");
	}
	size_t AcronymCount() {
		return 0;
	}
	Key NextAcronym(Key key = 0) {
		return 0;
	}
	CacheList GetAcronym(Key key) {
		return CacheList();
	}
	size_t RegexCount() {
		return 0;
	}
	Key NextRegex(Key key = 0) {
		return 0;
	}
	CacheList GetRegex(Key key) {
		return  CacheList();
	}

	size_t LabelCount() {
		return 0;
	}
	Key NextLabel(Key key = 0) {
		return 0;
	}
	CacheList GetLabel(Key key) {
		return  CacheList();
	}

	CacheList GetSpecialLabel(iknow::core::SpecialLabel label) {
		return CacheList();
	}

	size_t LexrepCount() {
		return 0;
	}
	Key NextLexrep(Key key = 0) {
		return 0;
	}
	CacheList GetLexrep(Key key) {
		return  CacheList();
	}

	size_t RuleCount() {
		return 0;
	}
	Key NextRule(Key key = 0) {
		return 0;
	}
	CacheList GetRule(Key key) {
		return  CacheList();
	}

	size_t PreprocessFilterCount() {
		return 0;
	}
	Key NextPreprocessFilter(Key key = 0) {
		return 0;
	}
	CacheList GetPreprocessFilter(Key key) {
		return  CacheList();
	}

	size_t ConceptFilterCount() {
		return 0;
	}
	Key NextConceptFilter(Key key = 0) {
		return 0;
	}
	CacheList GetConceptFilter(Key key) {
		return  CacheList();
	}

	size_t InputFilterCount() {
		return 0;
	}
	Key NextInputFilter(Key key = 0) {
		return 0;
	}
	CacheList GetInputFilter(Key key) {
		return  CacheList();
	}

	size_t PropertyCount() {
		return 0;
	}
	Key NextProperty(Key key = 0) {
		return 0;
	}
	CacheList GetProperty(Key key) {
		return  CacheList();
	}

	size_t MetadataCount() {
		return 0;
	}
	Key NextMetadata(Key key = 0) {
		return 0;
	}
	CacheList GetMetadata(Key key) {
		return CacheList();
	}

};

