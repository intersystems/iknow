# distutils: language=c++
# cython: c_string_type=unicode, c_string_encoding=utf8

from engine cimport CPPiKnowEngine

cdef class iKnowEngine:
	"""Python wrapper for C++ iKnowEngine class."""
	cdef CPPiKnowEngine engine

	def __cinit__(self):
		self.engine = CPPiKnowEngine()

	def get_languages_set(self):
		return self.engine.GetLanguagesSet()

	def index(self, str text_source, str language):
		return self.engine.index(text_source, language)

	def add_udct_annotation(self, size_t start, size_t stop, str UdctLabel):
		return self.engine.addUdctAnnotation(start, stop, UdctLabel)

	@property
	def m_index(self):
		return self.engine.m_index
