#includes
include $(ROOT_DIR)/build/make/platforms/superplatforms/windows.mak

RUN_CONFIGURE_ICU_PARAMS = Cygwin/MSVC

PLAT_OPENSSL_CONFIGURE_OPTIONS = VC-WIN32
PLAT_OPENSSL_ASSEMBLER_SCRIPT = ms\\do_nasm
