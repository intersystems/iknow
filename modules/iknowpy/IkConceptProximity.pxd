"""Export definitions from IkConceptProximity.h"""

from libcpp.vector cimport vector
from libcpp.utility cimport pair

cdef extern from "../core/src/headers/IkConceptProximity.h" namespace "iknow::core" nogil:
	ctypedef size_t EntityId_

cdef extern from "../core/src/headers/IkConceptProximity.h" namespace "iknow::core::IkConceptProximity" nogil:
	ctypedef size_t Proximity
	ctypedef EntityId_ EntityId
	ctypedef pair[pair[EntityId, EntityId], Proximity] ProximityPair_t
	ctypedef vector[ProximityPair_t] ProximityPairVector_t
