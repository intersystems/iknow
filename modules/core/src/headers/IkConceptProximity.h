#if !defined(AFX_IKCONCEPT_PROXIMITY_H__20D2CB14_6C49_4DF7_876C_08354C36CCAD__INCLUDED_)
#define AFX_IKCONCEPT_PROXIMITY_H__20D2CB14_6C49_4DF7_876C_08354C36CCAD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IkExportCore.h"

#include "PoolAllocator.h"
#include <vector>
#include <map>
#include <functional>
#include "Utility.h"
#include <unordered_map>

using iknow::base::PoolAllocator;

namespace iknow
{
	namespace core
	{
		typedef size_t OccurrenceId_, EntityId_; // should be included from defining modules

		class CORE_API IkConceptProximity
		{
		public:
			typedef size_t Proximity;
			typedef EntityId_ EntityId;
			typedef OccurrenceId_ OccurrenceId;
			typedef struct ProxPoint {
				OccurrenceId pos_;
				EntityId ent_id_;
				ProxPoint(OccurrenceId pos, EntityId ent_id) : pos_(pos), ent_id_(ent_id) {}
			} ProxPoint_t;
			typedef std::pair<std::pair<EntityId, EntityId>, Proximity> ProximityPair_t; // single proximity pair

			typedef PairUnorderedMap<std::pair<EntityId, EntityId>, Proximity> Proximity_t;
			typedef std::vector<ProximityPair_t> ProximityPairVector_t;
			typedef std::vector<ProxPoint_t, PoolAllocator<ProxPoint_t> > ProxPoints_t;
			typedef std::map<EntityId, EntityId, std::less<EntityId>, PoolAllocator<std::pair<const EntityId, EntityId> > > StemMap_t; // TODO: include from stemming module
			explicit IkConceptProximity(size_t capacity) {
				entProximity_.rehash(capacity);
			}
			IkConceptProximity(){}; // unfortunately, default is still needed
			void rehash(size_t capacity) { 
				entProximity_.rehash(capacity); 
			}

			void operator()(const ProxPoints_t& concept_proximity);
			ProximityPairVector_t GetSortedProximityPairs(); // no stemming
			ProximityPairVector_t GetSortedProximityPairs(StemMap_t&); // use stemming

		private:
			Proximity_t entProximity_;
			void operator=(const IkConceptProximity&);
		};
	}
}
#endif // AFX_IKCONCEPT_PROXIMITY_H__20D2CB14_6C49_4DF7_876C_08354C36CCAD__INCLUDED_
