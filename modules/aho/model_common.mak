MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/../..)

STRICT = 1

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

CPP_LANGUAGE = 14
INCLUDEDIRS =  $(ROOT_DIR)/modules/aho $(ROOT_DIR)/modules/base/src/headers $(ROOT_DIR)/modules/ali $(ROOT_DIR)/modules/core/src/headers \
	 $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/System $(ROOT_DIR)/shared/Utility $(ICUDIR)/include $(ROOT_DIR)/kernel/common/h

SOURCES = $(ROOT_DIR)/modules/aho/ModelRegistry.cpp $(ROOT_DIR)/modules/aho/AhoCorasick.cpp

LOCATION = bin
CREATELIBRARY = 1
PROJECT = libiknowmodelcom
OBJECTFLAGS = -D_DOUBLEBYTE -DCACHE_COM_DISABLE -DMODELID=en
IS_DYLIB = 1

LIBRARIES = iknowbase iknowali iknowcore
ifeq ($(findstring $(PLATFORM), ppc ppc64), $(PLATFORM))
LIBRARIES += icuuc40 icui18n40
else
LIBRARIES += icuuc icui18n
endif
LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin $(ICUDIR)/lib

include $(ROOT_DIR)/build/make/master.mak
