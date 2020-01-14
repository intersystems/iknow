#Sets up a standard environment based on the PLATFORM
#and ROOT_DIR variables without doing anything else.

include $(ROOT_DIR)/build/make/platforms/$(PLATFORM).mak
C_COMPILER ?= $(COMPILER)
CPP_COMPILER ?= $(COMPILER)
include $(ROOT_DIR)/build/make/variables.mak

