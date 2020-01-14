#includes
-include $(ROOT_DIR)/build/make/platforms/superplatforms/unix.mak

###Universal tools
MKDIR = mkdir -p $(dir $@)
DELETE = rm -f

SYMBOLIC_COPY = cp -Rh

TOOL = xlC
ifeq ($(MULTITHREADED),1)
TOOL = xlC_r
endif
###Stage 1: Source->Objects
COMPILER = $(TOOL)
HEADERINCLUDEFLAG = -I
OBJECTOUTPUTFLAG = -o
OBJECTSUFFIX = .o
OBJECTFLAGS += -c -O2 -DMY_BIG_ENDIAN=1 -D_ISC_BIGENDIAN=1 -q32 -Q -qchars=signed -qlanglvl=newexcp -DUNIX -DAIX



###Stage 2a: Objects->Library
LIBRARIAN = $(TOOL)
LIBRARYDIRFLAG = -L
LIBRARYSEARCHFLAG = -l
LIBRARYFLAGS += -q32 -qmkshrobj -G -brtl -blibpath:/usr/lib:/lib
LIBRARYOUTPUTFLAG = -o
LIBRARYSUFFIX = .so

###Stage 2b: Objects->Executable
LINKER = $(TOOL)
EXECUTABLEDIRFLAG = -L
EXECUTABLESEARCHFLAG = -l
EXECUTABLEFLAGS += -q32 -brtl -blibpath:/usr/lib:/lib
EXECUTABLEOUTPUTFLAG = -o
EXECUTABLESUFFIX = 


#Thirdparty definitions
RUN_CONFIGURE_ICU_PARAMS = AIX --enable-threads=no --enable-64bit-libs=no
RUN_CONFIGURE_XERCES_PARAMS = -p aix -c xlc_r -x xlC_r -minmem -nsocket -ticu -rnone -b32 -l -q32 -z -q32 -z -Q -z -qchars=signed
PLAT_XERCES_CONFIGURE_OPTIONS = CC=xlc_r CXX=xlC_r CFLAGS="-q32 -Q -qchars=signed" CXXFLAGS="-q32 -Q -qchars=signed" LDFLAGS="-q32"
RUN_CONFIGURE_XALAN_PARAMS = -p aix -c xlc_r -x xlC_r -minmem -ticu -b32  -l -q32 -z -q32 -z -Q -z -qchars=signed
SHARED_LIB_VAR = LIBPATH
BUILD_THIRDPARTY = 1
PLAT_OPENSSL_CONFIGURE_OPTIONS = aix-cc
PLATSOEXT=so
CONFIG_PLAT_CFLAGS=-q32
CONFIG_PLAT_LDFLAGS=-q32
PLAT_MQ_LIBRARIES = imqb23ia imqc23ia C
PLAT_MQ_LIBRARY_FLAGS = 
PLAT_MOD_CSP22_INC = ppc
PLAT_MOD_CSP22_OBJECTFLAGS = 
PLAT_MOD_CSP22_LIBRARYFLAGS =
#The following corresponds to openldap version openldap-2.4.11
PLAT_OPENLDAP_OUTPUT = liblber-2.4.so.* ../../libraries/libldap/.libs/libldap-2.4.so.2 ../../libraries/libldap/.libs/libldap-2.4.so.2.1.0  ../../libraries/libldap/.libs/libldap.so liblber.so
PLAT_OPENLDAP_COMPILER_OPTIONS = -brtl -blibpath:/usr/lib:/lib
PLAT_MDNSRESPONDER_OS = netbsd
PLAT_MDNSRESPONDER_CC = xlC -q32 -Q -qcpluscmt -DNOT_HAVE_DAEMON -brtl
PLAT_MDNSRESPONDER_LD = xlC -q32 -Q -qmkshrobj -G -brtl -blibpath:/usr/lib:/lib

#Debugging flags
ifeq ($(MODE),debug)
OBJECTFLAGS += -g -DDEBUG
LIBRARYFLAGS += -g -DDEBUG
EXECUTABLEFLAGS += -g -DDEBUG
RUN_CONFIGURE_ICU_PARAMS += --enable-debug
RUN_CONFIGURE_XERCES_PARAMS += -d
RUN_CONFIGURE_XALAN_PARAMS += -d
CONFIG_PLAT_CFLAGS+=-g -DDEBUG
CONFIG_PLAT_LDFLAGS+=-g -DDEBUG
endif
