#######################
# BEGIN CONFIGURATION #
#######################

VER_MAJOR = 2
VER_MINOR = 3
VER_UPDATE = 0

TARGET_SYSTEM = native
#TARGET_SYSTEM = i686

#BUILD_TYPE = release
BUILD_TYPE = debug

all:
	@echo "Building Base ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.base
	@echo "Building Core ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.core
	@echo "Building Shell ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.shell

strip:
	@echo "Building stripped Base ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.base strip
	@echo "Building stripped Core ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.core strip
	@echo "Building stripped Shell ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.shell strip

clean:
	@echo "Cleaning Base ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.base clean
	@echo "Cleaning Core ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.core clean
	@echo "Cleaning Shell ..."
	@VER_MAJOR=$(VER_MAJOR) VER_MINOR=$(VER_MINOR) VER_UPDATE=$(VER_UPDATE) BUILD_TYPE=$(BUILD_TYPE) TARGET_SYSTEM=$(TARGET_SYSTEM) make -f Makefile.shell clean

rebuild: clean all

