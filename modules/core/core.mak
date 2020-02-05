MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/../..)

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

CPP_LANGUAGE = 14

STRICT = 1

INCLUDEDIRS = $(ROOT_DIR)/modules/core/src/headers $(ICUDIR)/include $(ROOT_DIR)/modules/base/src/headers $(ROOT_DIR)/modules/ali $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/System $(ROOT_DIR)/shared/Utility $(ROOT_DIR)/kernel/common/h

SOURCES = $(ROOT_DIR)/modules/core/src/*.cpp $(ROOT_DIR)/shared/Utility/utlCacheList.cpp $(ROOT_DIR)/shared/Utility/utlCountedByteArray.cpp
LOCATION = bin
CREATELIBRARY = 1
PROJECT = libiknowcore
IS_DYLIB = 1

OBJECTFLAGS = -D_DOUBLEBYTE -DCACHE_COM_DISABLE

LIBRARIES = iknowbase iknowali icuuc icui18n
## ISC SOH280+
ifeq ($(findstring $(PLATFORM), ppc ppc64), $(PLATFORM))
LIBRARIES = iknowbase iknowali icuuc40 icui18n40
endif
## ISC SOH280-
LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin $(ICUDIR)/lib

include $(ROOT_DIR)/build/make/master.mak

