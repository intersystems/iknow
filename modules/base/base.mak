MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/../..)

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

STRICT = 1

CPP_LANGUAGE = 14

INCLUDEDIRS = $(ROOT_DIR)/modules/base/src/headers $(ICUDIR)/include $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/System $(ROOT_DIR)/shared/Utility $(ROOT_DIR)/kernel/common/h

SOURCES = $(ROOT_DIR)/modules/base/src/*.cpp
LOCATION = bin
CREATELIBRARY = 1
IS_DYLIB = 1
PROJECT = libiknowbase

LIBRARIES = icuuc icui18n
## ISC SOH280+
ifeq ($(findstring $(PLATFORM), ppc ppc64), $(PLATFORM))
LIBRARIES = icuuc40 icui18n40
endif
## ISC SOH280-
LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin $(ICUDIR)/lib

include $(ROOT_DIR)/build/make/master.mak

