##
# \defgroup	MK_OS_TX	FreeRTOS Related Makefile Configurations
# \ingroup	MK_OS
# \brief	makefile related to freertos os configurations
##

##
# \brief	current os directory definition
##
OS_TX_DIR = $(OSES_ROOT_DIR)/tx
$(warning "OS TX")

##
# \brief 		TX os related source and header
##
OS_TX_CSRCDIR	= $(OS_TX_DIR)/src
OS_TX_ASMSRCDIR	=  $(OS_TX_DIR)/src
OS_TX_INCDIR	= $(OS_TX_DIR)/inc

# find all the source files in the target directories
OS_TX_CSRCS = $(call get_csrcs, $(OS_TX_CSRCDIR))
OS_TX_ASMSRCS = $(call get_asmsrcs, $(OS_TX_ASMSRCDIR))

# get object files
OS_TX_COBJS = $(call get_relobjs, $(OS_TX_CSRCS))
OS_TX_ASMOBJS = $(call get_relobjs, $(OS_TX_ASMSRCS))
OS_TX_OBJS = $(OS_TX_COBJS) $(OS_TX_ASMOBJS)

# get dependency files
OS_TX_DEPS = $(call get_deps, $(OS_TX_OBJS))

# extra macros to be defined
OS_TX_DEFINES = -DOS_TX
ifeq ($(TOOLCHAIN), mw)
OS_LIB_TX_PREBUILT = $(OS_TX_DIR)\prebuilt_lib\libtx_mw.a
endif
ifeq ($(TOOLCHAIN), gnu)
OS_LIB_TX_PREBUILT = $(OS_TX_DIR)/prebuilt_lib/libtx_gnu.a
endif
OS_LIB_TX = $(OUT_DIR)/libtx.a

ifeq ($(OS_TX_PREBUILT), 1)
$(warning "+++++++++++++++++++")
$(warning $(OS_LIB_TX_PREBUILT))
$(warning $(OS_LIB_TX))
$(warning $(LIB_TX_PREBUILT))
$(warning "-------------------")
$(OS_LIB_TX) : 
	$(CP) $(OS_LIB_TX_PREBUILT) $(OS_LIB_TX) 
else
$(warning $(CC))
$(warning $(AR))
$(warning $(AR_OPT))
#$(warning $(OS_TX_DEPS))
# library generation rule
$(OS_LIB_TX): $(OS_TX_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(OS_TX_OBJS) $(OS_TX_DEPS)
ifeq ($(TOOLCHAIN), mw)	
	$(CP) $(BOARD_OUT_DIR)\$(BUILD_INFO)\libtx.a .\os\tx\prebuilt_lib\libtx_mw.a
endif
ifeq ($(TOOLCHAIN), gnu)
	$(CP) $(BOARD_OUT_DIR)/$(BUILD_INFO)/libtx.a ./os/tx/prebuilt_lib/libtx_gnu.a
endif	
endif
# specific compile rules
# user can add rules to compile this OS
# if not rules specified to this OS, it will use default compiling rules

# OS Definitions
OS_INCDIR += $(OS_TX_INCDIR)
OS_CSRCDIR += $(OS_TX_CSRCDIR)
OS_ASMSRCDIR += $(OS_TX_ASMSRCDIR)

OS_CSRCS += $(OS_TX_CSRCS)
OS_CXXSRCS +=
OS_ASMSRCS += $(OS_TX_ASMSRCS)
OS_ALLSRCS += $(OS_TX_CSRCS) $(OS_TX_ASMSRCS)

OS_COBJS += $(OS_TX_COBJS)
OS_CXXOBJS +=
OS_ASMOBJS += $(OS_TX_ASMOBJS)
OS_ALLOBJS += $(OS_TX_OBJS)

OS_DEFINES += $(OS_TX_DEFINES)
OS_DEPS += $(OS_TX_DEPS)
OS_LIBS += $(OS_LIB_TX)
