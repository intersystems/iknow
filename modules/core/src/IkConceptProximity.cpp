/*
** JDN: Separated module as of june 2018
*/
#include "IkConceptProximity.h"

using namespace iknow::core;

void IkConceptProximity::operator()(const ProxPoints_t& concept_proximity) {
	for (ProxPoints_t::const_iterator itX = concept_proximity.begin(); itX != concept_proximity.end(); itX++) {
		for (ProxPoints_t::const_iterator itY = itX + 1; itY != concept_proximity.end(); itY++) {
			if (itX->ent_id_ == itY->ent_id_) // same entity, no proximity calculation
				continue;
			Proximity proximity = 256 / (std::abs(static_cast<int>(itY->pos_ - itX->pos_)) + 2); // scale on base256
			// TODO: Send Trace data to evaluate proximity
			std::pair<EntityId, EntityId> pair1(itX->ent_id_, itY->ent_id_), pair2(itY->ent_id_, itX->ent_id_);

			Proximity_t::iterator it_xy = entProximity_.find(pair1); // XY
			if (it_xy != entProximity_.end()) { // if xy
				it_xy->second += proximity;
			}
			else { // no xy
				Proximity_t::iterator it_yx = entProximity_.find(pair2); // YX
				if (it_yx != entProximity_.end()) { // if yx
					it_yx->second += proximity;
				}
				else { // no xy, no yx, new !
					entProximity_.insert(Proximity_t::value_type(pair1, proximity));
				}
			}
		}
	}
}

IkConceptProximity::ProximityPairVector_t IkConceptProximity::GetSortedProximityPairs()
{
	ProximityPairVector_t proximity_pairs;
	proximity_pairs.reserve(entProximity_.size());
	for (Proximity_t::const_iterator it = entProximity_.begin(); it != entProximity_.end(); it++) {
		proximity_pairs.push_back(IkConceptProximity::ProximityPair_t(it->first, it->second));
		// TODO: trace the list
	}
	std::sort(proximity_pairs.begin(), proximity_pairs.end(), 
		[](const IkConceptProximity::ProximityPair_t& a, const IkConceptProximity::ProximityPair_t& b) { return a.second > b.second; });

	return proximity_pairs; // uses move semantics
}

IkConceptProximity::ProximityPairVector_t IkConceptProximity::GetSortedProximityPairs(StemMap_t& stem_ent_map)
{
	ProximityPairVector_t proximity_pairs;
	proximity_pairs.reserve(entProximity_.size());
	for (Proximity_t::const_iterator it = entProximity_.begin(); it != entProximity_.end(); it++) {
		proximity_pairs.push_back(IkConceptProximity::ProximityPair_t(it->first, it->second));
		// TODO: trace the list
	}
	// Adjust proximity for stemming
	if (stem_ent_map.size()) {
		for (IkConceptProximity::ProximityPairVector_t::iterator ppFirst = proximity_pairs.begin(); ppFirst != proximity_pairs.end(); ++ppFirst) {
			typedef std::pair<EntityId, EntityId> EntityIdPair;
			EntityIdPair stemFirst(ppFirst->first);
			bool bFirstStemmed = false;
			StemMap_t::const_iterator x = stem_ent_map.find(stemFirst.first);
			if (x != stem_ent_map.end()) bFirstStemmed = true, stemFirst.first = x->second;
			StemMap_t::const_iterator y = stem_ent_map.find(stemFirst.second);
			if (y != stem_ent_map.end()) bFirstStemmed = true, stemFirst.second = y->second;
			if (bFirstStemmed) { // search equal pairs
				Proximity firstProx = ppFirst->second; // incremental value to add to stemmed equal pairs
				for (IkConceptProximity::ProximityPairVector_t::iterator ppSecond = ppFirst + 1; ppSecond != proximity_pairs.end(); ++ppSecond) {
					EntityIdPair stemSecond(ppSecond->first);
					bool bSecondStemmed = false;
					StemMap_t::const_iterator xx = stem_ent_map.find(stemSecond.first);
					if (xx != stem_ent_map.end()) bSecondStemmed = true, stemSecond.first = xx->second;
					StemMap_t::const_iterator yy = stem_ent_map.find(stemSecond.second);
					if (yy != stem_ent_map.end()) bSecondStemmed = true, stemSecond.second = yy->second;
					if (bSecondStemmed) { // both pairs have stemmed equivalents
						if ((stemFirst.first == stemSecond.first && stemFirst.second == stemSecond.second) ||
							(stemFirst.first == stemSecond.second && stemFirst.second == stemSecond.first)) { // adjust proximity value
							ppFirst->second += ppSecond->second;
							ppSecond->second += firstProx;
							// TODO: Trace stem correction
						}
					}
				}
			}
		}
	}
	std::sort(proximity_pairs.begin(), proximity_pairs.end());
	return proximity_pairs; // uses move semantics
}
