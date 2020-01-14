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
ifeq ($(CMODE),1)
TOOL = xlc
ifeq ($(MULTITHREADED),1)
TOOL = xlc_r
endif
endif

###Stage 1: Source->Objects
COMPILER = $(TOOL)

PLATCFLAGS = -q64 -qchars=signed
PLATCXXFLAGS = -q64 -qchars=signed
PLATLDFLAGS = -q64

HEADERINCLUDEFLAG = -I
OBJECTOUTPUTFLAG = -o
OBJECTSUFFIX = .o
OBJECTFLAGS += -c -DMY_BIG_ENDIAN=1 -D_ISC_BIGENDIAN=1 -DPNTBYTES=8 -q64 -qchars=signed -qlanglvl=newexcp -DBIT64PLAT -DAIX -DUNIX -Q -I/opt/freeware/include
OBJECTFLAGSNODASHC += -DMY_BIG_ENDIAN=1 -D_ISC_BIGENDIAN=1 -DPNTBYTES=8 -q64 -qchars=signed -qlanglvl=newexcp -DBIT64PLAT -DAIX -DUNIX -Q -I/opt/freeware/include
ifneq ($(MODE),debug)
# Only optimize for a release build, otherwise dbx can't debug ...
ifneq ($(UNOPTIMIZED),1)
OBJECTFLAGS += -O2
OBJECTFLAGSNODASHC += -O2
endif
endif


###Stage 2a: Objects->Library
LIBRARIAN = $(TOOL)
LIBRARYDIRFLAG = -L
LIBRARYSEARCHFLAG = -l
LIBRARYFLAGS += -q64 -qmkshrobj -G -brtl -blibpath:/usr/lib:/lib
LIBRARYOUTPUTFLAG = -o
LIBRARYSUFFIX = .so

###Stage 2b: Objects->Executable
LINKER = $(TOOL)
EXECUTABLEDIRFLAG = -L
EXECUTABLESEARCHFLAG = -l
EXECUTABLEFLAGS += -q64 -brtl -blibpath:/usr/lib:/lib
EXECUTABLEOUTPUTFLAG = -o
EXECUTABLESUFFIX = 
RUNTIMELOADPATHVAR = LIBPATH


#Thirdparty definitions
PLAT_OPENSSL_CONFIGURE_OPTIONS = aix64-cc -qkeyword=inline
PLAT_CMQL_CONFIGURE_OPTIONS = --enable-64bit
PLAT_ANTLR_CONFIGURE_OPTIONS = --enable-64bit
RUN_CONFIGURE_ICU_PARAMS = AIX --enable-threads=no --build=powerpc-ibm-aix5.3.0.0 --host=powerpc-ibm-aix5.3.0.0 --target=powerpc-ibm-aix5.3.0.0
PLAT_XERCES_CONFIGURE_OPTIONS = CC=xlc_r CXX=xlC_r CFLAGS=-q64 CXXFLAGS=-q64 LDFLAGS=-q64 --build=powerpc-ibm-aix5.3.0.0 --host=powerpc-ibm-aix5.3.0.0 --target=powerpc-ibm-aix5.3.0.0 --without-curl
RUN_CONFIGURE_XALAN_PARAMS = -p aix -c xlc_r -x xlC_r -minmem -ticu -b64 -z -qchars=signed -l -bbigtoc -C --build=powerpc-ibm-aix5.3.0.0 -C --host=powerpc-ibm-aix5.3.0.0 -C --target=powerpc-ibm-aix5.3.0.0
SHARED_LIB_VAR = LIBPATH
BUILD_THIRDPARTY = 1
PLATSOEXT=so
CONFIG_PLAT_CFLAGS=-q64
CONFIG_PLAT_LDFLAGS=-q64
PLAT_MQ_LIBRARIES = imqb23ia imqc23ia mqic C
PLAT_MQ_LIBRARY_FLAGS = 
PLAT_MOD_CSP22_INC = ppc
PLAT_MOD_CSP22_OBJECTFLAGS = 
PLAT_MOD_CSP22_LIBRARYFLAGS =
#The following corresponds to openldap version openldap-2.4.11
PLAT_OPENLDAP_OUTPUT = liblber-2.4.so.* ../../libraries/libldap/.libs/libldap-2.4.so.2 ../../libraries/libldap/.libs/libldap-2.4.so.2.1.0  ../../libraries/libldap/.libs/libldap.so liblber.so
PLAT_OPENLDAP_COMPILER_OPTIONS = -brtl -blibpath:/usr/lib:/lib
PLAT_BJAM_OPTIONS = toolset=vacpp address-model=64
PLAT_ZLIB_OBJECTFLAGS = $(OBJECTFLAGSNODASHC)

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
