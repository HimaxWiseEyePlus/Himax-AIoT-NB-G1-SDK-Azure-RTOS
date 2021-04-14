##
# \defgroup	MK_OS	OS Makefile Configurations
# \brief	makefile related to operation system configurations
##

##
# OS Root
##
OSES_ROOT_DIR = $(EMBARC_ROOT)/os

OS_CSRCDIR	=
OS_ASMSRCDIR	=
OS_INCDIR	=

# source files and object files
OS_CSRCS =
OS_CXXSRCS =
OS_ASMSRCS =
OS_ALLSRCS =

OS_COBJS =
OS_CXXOBJS =
OS_ASMOBJS =
OS_ALLOBJS =

ifeq ($(OS_SEL), tx)
#os definition
OS_ID = OS_TX
#os usage settings
COMMON_COMPILE_PREREQUISITES += $(OSES_ROOT_DIR)/tx/tx.mk
include $(OSES_ROOT_DIR)/tx/tx.mk
#end of tx#
endif #end of tx

##
# \brief	add defines for os
##
ifdef OS_ID
OS_DEFINES += -D$(OS_ID) -DENABLE_OS
else
OS_DEFINES +=
endif

# include dependency files
ifneq ($(MAKECMDGOALS),clean)
-include $(OS_DEPS)
endif