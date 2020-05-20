# distutils: language=c++
# cython: c_string_type=unicode, c_string_encoding=utf8

from cython.operator cimport dereference as deref, postincrement as postinc
from .engine cimport *


cdef char* eType_to_str(eType t) except NULL:
	"""Convert entity type from enum to string."""
	if t == NonRelevant:
		return 'NonRelevant'
	elif t == Concept:
		return 'Concept'
	elif t == Relation:
		return 'Relation'
	elif t == PathRelevant:
		return 'PathRelevant'
	raise ValueError('Entity type {} is unrecognized.'.format(t))


cdef char* aType_to_str(aType t) except NULL:
	"""Convert attribute type from enum to string."""
	if t == Negation:
		return 'Negation'
	elif t == DateTime:
		return 'DateTime'
	elif t == PositiveSentiment:
		return 'PositiveSentiment'
	elif t == NegativeSentiment:
		return 'NegativeSentiment'
	elif t == Frequency:
		return 'Frequency'
	elif t == Duration:
		return 'Duration'
	elif t == Measurement:
		return 'Measurement'
	elif t == Certainty:
		return 'Certainty'
	raise ValueError('Attribute type {} is unrecognized.'.format(t))


cdef class iKnowEngine:
	"""Python wrapper for C++ iKnowEngine class."""
	cdef CPPiKnowEngine engine

	def __cinit__(self):
		self.engine = CPPiKnowEngine()

	@staticmethod
	def get_languages_set():
		return CPPiKnowEngine.GetLanguagesSet()

	def index(self, str text_source, str language):
		if language not in self.get_languages_set():
			raise ValueError('Language {!r} is not supported.'.format(language))
		return self.engine.index(text_source, language)

	def add_udct_annotation(self, size_t start, size_t stop, str UdctLabel):
		return self.engine.addUdctAnnotation(start, stop, UdctLabel)

	@property
	def m_index(self):
		"""Build a Python representation of the index. The Python representation follows the Cython default type
		coercions with the exception of C++ enums, which are converted to Python strings."""
		cdef list sentences_mod = []
		cdef list entities_mod, sent_attrs_mod
		for sentence in self.engine.m_index.sentences:
			entities_mod = []
			sent_attrs_mod = []
			# use iterator instead of for-in syntax because Entity and Path_Attribute_Span structs don't have default
			# constructors, which are required with for-in syntax
			entity_iter = sentence.entities.begin()
			while entity_iter != sentence.entities.end():
				entities_mod.append({'type': eType_to_str(deref(entity_iter).type),
				                     'offset_start': deref(entity_iter).offset_start,
				                     'offset_stop': deref(entity_iter).offset_stop,
				                     'index': deref(entity_iter).index,
				                     'dominance_value': deref(entity_iter).dominance_value,
				                     'entity_id': deref(entity_iter).entity_id})
				postinc(entity_iter)
			sent_attr_iter = sentence.sent_attributes.begin()
			while sent_attr_iter != sentence.sent_attributes.end():
				sent_attrs_mod.append({'type': aType_to_str(deref(sent_attr_iter).type),
				                       'offset_start': deref(sent_attr_iter).offset_start,
				                       'offset_stop': deref(sent_attr_iter).offset_stop,
				                       'marker': deref(sent_attr_iter).marker,
				                       'value': deref(sent_attr_iter).value,
				                       'unit': deref(sent_attr_iter).unit,
				                       'value2': deref(sent_attr_iter).value2,
				                       'unit2': deref(sent_attr_iter).unit2,
				                       'entity_ref': deref(sent_attr_iter).entity_ref})
				postinc(sent_attr_iter)
			sentences_mod.append({'entities': entities_mod, 'sent_attributes': sent_attrs_mod, 'path': sentence.path,
			                      'path_attributes': sentence.path_attributes})
		return {'sentences': sentences_mod, 'proximity': self.engine.m_index.proximity}
