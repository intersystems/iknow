#includes
-include $(ROOT_DIR)/build/make/platforms/superplatforms/unix.mak

###Universal tools
MKDIR = test -e $(dir $@) || mkdir -p $(dir $@)
DELETE = rm -f

#CLANG_MODE = 1
STDLIB=libc++
CPPLANGFLAG=-std=c++11 -Wno-non-template-friend

###Stage 1: Source->Objects
ifeq ($(CLANG_MODE),1)
C_COMPILER = clang
CPP_COMPILER = clang++ $(CPPLANGFLAG)
else
C_COMPILER = gcc
CPP_COMPILER = g++ $(CPPLANGFLAG)
endif
###C vs. C++
GNUTOOL = $(if $(CMODE),$(C_COMPILER),$(CPP_COMPILER))

COMPILER = $(GNUTOOL)

HEADERINCLUDEFLAG = -I
OBJECTOUTPUTFLAG = -o
OBJECTSUFFIX = .o
LIBFLAGS = -fPIC
OBJECTFLAGS += -c -O2 -DBIT64PLAT -DSIZEOF_LONG=8 -Wno-deprecated-declarations  -DLINUX -DUNIX $(if $(CREATELIBRARY),$(LIBFLAGS)) 

###Japanese language model is optimized forever...
ifeq ($(UNOPTIMIZED), 1)
OBJECTFLAGS += -O0
endif

###Stage 2a: Objects->Library
LIBRARIAN = $(GNUTOOL)

LIBRARYDIRFLAG = -L
LIBRARYSEARCHFLAG = -l
ifeq ($(USE_LIBRARIAN_LD),1)
LIBRARYFLAGS += -shared -rpath .
else
LIBRARYFLAGS += -shared -Wl,-rpath,.
endif
LIBRARYOUTPUTFLAG = -o
LIBRARYSUFFIX = .so

###Stage 2b: Objects->Executable
LINKER = $(GNUTOOL)
EXECUTABLEDIRFLAG = -L
EXECUTABLESEARCHFLAG = -l
EXECUTABLEFLAGS += -ldl -Wno-non-template-friend -DLINUX 
EXECUTABLEOUTPUTFLAG = -o
EXECUTABLESUFFIX = 
RUNTIMELOADPATHVAR = LD_LIBRARY_PATH


ifeq ($(MULTITHREADED),1)
C_COMPILER += -pthread
CPP_COMPILER += -pthread
LINKER += -pthread
LIBRARIAN += -pthread
endif


ifeq ($(HIDE_SYMBOLS),1)
LIBRARYFLAGS += -Wl,-Bsymbolic
EXECUTABLEFLAGS += -Wl,-Bsymbolic
endif


#Thirdparty definitions for building ICU/Xerces/Xalan
BUILD_THIRDPARTY = 1
RUN_CONFIGURE_ICU_PARAMS = LinuxRedHat --enable-threads=no LDFLAGS="-Wl,-rpath,."

PLAT_XERCES_CONFIGURE_OPTIONS = LDFLAGS="-Wl,-rpath,."
RUN_CONFIGURE_XALAN_PARAMS = -p linux -c gcc -x g++ -m inmem -t icu -b 64
SHARED_LIB_VAR = LD_LIBRARY_PATH

PLAT_OPENSSL_CONFIGURE_OPTIONS = linux-x86_64
PLAT_CMQL_CONFIGURE_OPTIONS = --enable-64bit
PLAT_ANTLR_CONFIGURE_OPTIONS = --enable-64bit
PLATSOEXT = so

PLAT_MQ_LIBRARIES = imqb23gl imqc23gl
PLAT_MQ_LIBRARY_FLAGS =

PLAT_MOD_CSP22_INC = linux 
PLAT_MOD_CSP22_OBJECTFLAGS = -fpic -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
PLAT_MOD_CSP22_LIBRARYFLAGS = 
PLAT_ZLIB_COMPILER = gcc
PLAT_ZLIB_LIBRARIAN = gcc
PLAT_ZLIB_OBJECTFLAGS = -O3 -DBIT64PLAT -DSIZEOF_LONG=8 -DLINUX -DUNIX $(if $(CREATELIBRARY),$(LIBFLAGS))
PLAT_MDNSRESPONDER_OS = linux
PLAT_MDNSRESPONDER_CC = gcc
PLAT_MDNSRESPONDER_LD = gcc -shared

PLAT_LIBSSH2_LIBS = -ldl

#Debugging flags
ifeq ($(MODE),debug)
OBJECTFLAGS += -g -O0 -DDEBUG
LIBRARYFLAGS += -g -DDEBUG
EXECUTABLEFLAGS += -g -DDEBUG
RUN_CONFIGURE_ICU_PARAMS += --enable-debug

CONFIG_PLAT_CFLAGS+=-g -O0 -DDEBUG
CONFIG_PLAT_LDFLAGS+=-g -DDEBUG
PLAT_MDNSRESPONDER_CC += -g -O0
endif

#Strict flags
ifeq ($(STRICT),1)
OBJECTFLAGS += -Werror
LIBRARYFLAGS += -Werror
EXECUTABLEFLAGS += -Werror
endif
