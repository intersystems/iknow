#The master make file. Sets defaults and derived variables
#for compiler flags and output locations.
#Variables required to be set:
#	ROOT_DIR	- the root of the build/source tree
#	PLATFORM - the build platform
#	PROJECT - the name of the project being built
#Variables that optionally can be set:
#	MODE - release/debug build
#	LOCATION - output location in kit directory, e.g. "bin"
#	INCLUDEDIRS - directories to search for .h files
#	SOURCEDIRS - directories to search for all files
#	INCLUDEFLAG - flag for the compiler to include a directory
#	SOURCES - files to use as source code

#Check some required variables
ifneq ($(wildcard $(ROOT_DIR)/build/make/master.mak),$(ROOT_DIR)/build/make/master.mak)
$(error ROOT_DIR must be set to the root of the source tree)
endif

ifndef PROJECT
$(error PROJECT must be set to the name of the project)
endif

SUPPORTED_PLATFORMS = $(basename $(notdir $(wildcard $(ROOT_DIR)/build/make/platforms/*.mak)))
ifneq ($(findstring $(PLATFORM), $(SUPPORTED_PLATFORMS)), $(PLATFORM))
$(error PLATFORM must be set to one of $(SUPPORTED_PLATFORMS))
endif

#Override all the built in implied suffix rules
.SUFFIXES:

#Build the project by default, but check SKIP_PLATFORMS
#to see if we should skip
ifeq ($(findstring $(PLATFORM), $(SKIP_PLATFORMS)), $(PLATFORM))
all :
	echo Skipping on $(PLATFORM)...
else
ifdef CLEAN
all : clean
	
else
all : $(PROJECT)
	
endif
endif

#Set defaults for MODE and LOCATION
#MODE - release or bin
#LOCATION - where to put files in the "installation" we output to
MODE ?= release
LOCATION ?= bin

#Set defaults for finding source
INCLUDEDIRS ?= .
INCLUDEEXTENSIONS = .h .hpp .inc .inl
SOURCEEXTENSIONS = .c .cpp
SOURCES ?= $(addprefix *, $(SOURCEEXTENSIONS))
INCLUDES ?= $(foreach GLOB, $(addprefix *, $(INCLUDEEXTENSIONS)), $(foreach DIR, $(INCLUDEDIRS), $(DIR)/$(GLOB)))

GLOBIFY = $(foreach GLOB, $(1), $(wildcard $(GLOB)))
SOURCES := $(call GLOBIFY, $(SOURCES))
EXCLUDESOURCES := $(call GLOBIFY, $(EXCLUDESOURCES))
INCLUDES := $(call GLOBIFY, $(INCLUDES))
EXCLUDEHEADERS := $(call GLOBIFY, $(EXCLUDEHEADERS))


SOURCES := $(filter-out $(EXCLUDESOURCES),$(SOURCES))
INCLUDES := $(filter-out $(EXCLUDEHEADERS),$(INCLUDES))

SOURCEDIRS ?= $(foreach SOURCE, $(SOURCES), $(dir $(SOURCE)))

#Set up the virtual path to search for h files
#in the include dirs
vpath %.h $(INCLUDEDIRS)
vpath % $(SOURCEDIRS)


#Create the output and intermediate object dirs
OUTPUTDIR = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/$(LOCATION)
OBJECTDIR = $(ROOT_DIR)/built/$(PLATFORM)/$(MODE)/$(PROJECT)

#Add the include directories to the CPP compiler options



include $(ROOT_DIR)/build/make/locations.mak
include $(ROOT_DIR)/build/make/platforms/$(PLATFORM).mak

OBJECTFLAGS += $(OBJECTFLAGS_$(PLATFORM))
LIBRARYFLAGS += $(LIBRARYFLAGS_$(PLATFORM))
EXECUTABLEFLAGS += $(EXECUTABLEFLAGS_$(PLATFORM))
LIBRARIES += $(LIBRARIES_$(PLATFORM))

OBJECTIFY = $(OBJECTDIR)/$(addsuffix $(OBJECTSUFFIX), $(basename $(notdir $(1))))
OBJECTS ?= $(foreach FILE, $(SOURCES), $(call OBJECTIFY, $(FILE)))

#Separate C and C++ compilers if specified
C_COMPILER ?= $(COMPILER)
CPP_COMPILER ?= $(COMPILER)


#How to build .cpp files
$(call OBJECTIFY, %.cpp) : %.cpp $(INCLUDES)
	$(MKDIR)
	$(CPP_COMPILER) $(OBJECTFLAGS) $(INCLUDEFLAGS) $(OBJECTOUTPUTFLAG) $@ $<

#How to build .c files
$(call OBJECTIFY, %.c) : %.c $(INCLUDES)
	$(MKDIR)
	$(C_COMPILER) $(OBJECTFLAGS) $(INCLUDEFLAGS) $(OBJECTOUTPUTFLAG) $@ $<

#How to create a library
LIBRARY ?= $(OUTPUTDIR)/$(PROJECT)$(LIBRARYSUFFIX)

$(LIBRARY) : $(OBJECTS) $(ADDITIONALOBJECTS)
	$(MKDIR)
	$(LIBRARIAN) $(LIBRARYFLAGS) $(LIBRARYOUTPUTFLAG) $@ $^ $(LINKEDLIBRARIES)


#How to create executable
EXECUTABLE ?= $(OUTPUTDIR)/$(PROJECT)$(EXECUTABLESUFFIX)

$(EXECUTABLE) : $(OBJECTS) $(ADDITIONALOBJECTS)
	$(MKDIR)
	echo $(RUNTIMELOADPATHVAR) = $($(RUNTIMELOADPATHVAR))
	export $(RUNTIMELOADPATHVAR) 
	$(LINKER) $(EXECUTABLEFLAGS) $(EXECUTABLEOUTPUTFLAG) $@ $^ $(LINKEDLIBRARIES)
	$(POSTEXELINKCOMMAND)

#How to clean up
clean :
	$(DELETE) $(OBJECTS)
	$(if $(CREATEEXECUTABLE), $(DELETE) $(EXECUTABLE))
	$(if $(CREATELIBRARY), $(DELETE) $(LIBRARY))	

#Save these files even if they are intermediates.
.PRECIOUS : $(OBJECTDIR)/%$(OBJECTSUFFIX)
.PRECIOUS : $(OUTPUTDIR)/%$(EXECUTABLESUFFIX)
.PRECIOUS : $(OUTPUTDIR)/%$(LIBRARYSUFFIX)



#Select whether the project is a library or executable
$(PROJECT) : $(if $(CREATELIBRARY),$(LIBRARY)) $(if $(CREATEEXECUTABLE), $(EXECUTABLE))

INCLUDEFLAGS := $(addprefix $(HEADERINCLUDEFLAG),$(INCLUDEDIRS))
LINKEDLIBRARIES := $(addprefix $(LIBRARYSEARCHFLAG),$(LIBRARIES)) $(LINKEDLIBRARIES)
LINKEDLIBRARIES := $(addprefix $(LIBRARYDIRFLAG), $(LIBRARYDIRS)) $(LINKEDLIBRARIES)

#Set up the runtime load path for building executables

ifdef RUNTIMELOADPATHVAR
RUNTIMELOADPATH = $(subst  ,:,$(LIBRARYDIRS))
$(RUNTIMELOADPATHVAR) = $(RUNTIMELOADPATH)
export $(RUNTIMELOADPATHVAR)
endif
