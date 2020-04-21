// IkLabel.cpp: implementation of the IkLabel class.
//
//////////////////////////////////////////////////////////////////////

#include "IkLabel.h"
#include "IkKnowledgebase.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace iknow::core;
using namespace iknow::base;

IkLabel::IkLabel()
{
  m_nType = Unknown;
}

IkLabel::IkLabel(const iknow::base::String& strName, Type nType) : m_strName(strName), m_nType(nType) {}

IkLabel::~IkLabel()
{
}

struct LabelTypeMap {
	LabelTypeMap() {
		map.insert(TypeMap::value_type("typeNonRelevant", iknow::core::IkLabel::Nonrelevant));
		map.insert(TypeMap::value_type("typeAmbiguous", iknow::core::IkLabel::Ambiguous));
		map.insert(TypeMap::value_type("typeAttribute", iknow::core::IkLabel::Attribute));
		map.insert(TypeMap::value_type("typeConcept", iknow::core::IkLabel::Concept));
		map.insert(TypeMap::value_type("typeRelation", iknow::core::IkLabel::Relation));
		map.insert(TypeMap::value_type("typeBeginConcept", iknow::core::IkLabel::BeginConcept));
		map.insert(TypeMap::value_type("typeEndConcept", iknow::core::IkLabel::EndConcept));
		map.insert(TypeMap::value_type("typeBeginEndConcept", iknow::core::IkLabel::BeginEndConcept));
		map.insert(TypeMap::value_type("typeBeginRelation", iknow::core::IkLabel::BeginRelation));
		map.insert(TypeMap::value_type("typeEndRelation", iknow::core::IkLabel::EndRelation));
		map.insert(TypeMap::value_type("typeBeginEndRelation", iknow::core::IkLabel::BeginEndRelation));
		map.insert(TypeMap::value_type("typeLiteral", iknow::core::IkLabel::Literal));
		map.insert(TypeMap::value_type("typeOther", iknow::core::IkLabel::Other));
		map.insert(TypeMap::value_type("typePathRelevant", iknow::core::IkLabel::PathRelevant));
	}
	iknow::core::IkLabel::Type Lookup(const std::string& type_name) const {
		TypeMap::const_iterator i = map.find(type_name);
		return ((i == map.end()) ? iknow::core::IkLabel::Unknown : i->second);
	}
	std::string Lookup(const iknow::core::IkLabel::Type label_type) const {
		for (TypeMap::const_iterator it = map.begin(); it != map.end(); ++it) { // lookup the label type
			if (it->second == label_type) return it->first;
		}
		return std::string("typeUnknown"); // Unknown type
	}
	typedef std::map<std::string, enum iknow::core::IkLabel::Type> TypeMap;
	TypeMap map;
};

const static LabelTypeMap label_type_map_;

iknow::core::IkLabel::Type IkLabel::TypeStringToType(const std::string& typeString) {
	return label_type_map_.Lookup(typeString);
}
std::string IkLabel::LabelTypeToString(iknow::core::IkLabel::Type label_type) {
	return label_type_map_.Lookup(label_type);
}


