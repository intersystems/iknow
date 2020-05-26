MAKEPATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))).)
ROOT_DIR = $(realpath $(MAKEPATH)/../..)

#TODO: TRW - generalize!
PLATFORM = $(IKNOWPLAT)

CPP_LANGUAGE = 14

INCLUDEDIRS = $(ROOT_DIR)/modules/shell/src $(ROOT_DIR)/modules/shell/src/SDK/headers $(ROOT_DIR)/modules/base/src/headers $(ROOT_DIR)/modules/ali $(ROOT_DIR)/modules/core/src/headers $(ROOT_DIR)/modules/aho $(ROOT_DIR)/modules/engine/src $(ROOT_DIR)/shared/System/unix $(ROOT_DIR)/shared/System $(ROOT_DIR)/shared/Utility $(ROOT_DIR)/kernel/common/h $(ICUDIR)/include

SOURCES = $(ROOT_DIR)/modules/enginetest/enginetest.cpp $(ROOT_DIR)/modules/enginetest/iKnowUnitTests.cpp

LOCATION = bin

CREATEEXECUTABLE = 1

PROJECT = iknowenginetest

OBJECTFLAGS = -D_DOUBLEBYTE -DCACHE_COM_DISABLE

LIBRARIES = iknowengine iknowbase

LIBRARYDIRS = $(ROOT_DIR)/kit/$(PLATFORM)/$(MODE)/bin

include $(ROOT_DIR)/build/make/master.mak

