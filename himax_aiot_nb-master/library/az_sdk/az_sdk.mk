# directory declaration
LIB_AZ_SDK_DIR = $(LIBRARIES_ROOT)/az_sdk
$(warning build az_sdk lib) 
ifeq ($(LIB_AZ_SDK_PREBUILT), 1)

LIB_AZ_SDK_ASMSRCDIR	=
LIB_AZ_SDK_CSRCDIR	= 
LIB_AZ_SDK_CXXSRCSDIR    = 

else
LIB_AZ_SDK_ASMSRCDIR	= $(LIB_AZ_SDK_DIR)/src/azure/core $(LIB_AZ_SDK_DIR)/src/azure/iot $(LIB_AZ_SDK_DIR)/src/azure/platform
LIB_AZ_SDK_CSRCDIR	= $(LIB_AZ_SDK_DIR)/src/azure/core $(LIB_AZ_SDK_DIR)/src/azure/iot $(LIB_AZ_SDK_DIR)/src/azure/platform
LIB_AZ_SDK_CXXSRCSDIR    = $(LIB_AZ_SDK_DIR)/src/azure/core $(LIB_AZ_SDK_DIR)/src/azure/iot $(LIB_AZ_SDK_DIR)/src/azure/platform
endif
LIB_AZ_SDK_INCDIR	= $(LIB_AZ_SDK_DIR)/inc $(LIB_AZ_SDK_DIR)/inc/azure 

# find all the source files in the target directories
LIB_AZ_SDK_CSRCS = $(call get_csrcs, $(LIB_AZ_SDK_CSRCDIR))
LIB_AZ_SDK_CXXSRCS = $(call get_cxxsrcs, $(LIB_AZ_SDK_CXXSRCSDIR))
LIB_AZ_SDK_ASMSRCS = $(call get_asmsrcs, $(LIB_AZ_SDK_ASMSRCDIR))


#$(warning build neaz_sdkduo lib $(LIB_AZ_SDK_CSRCS)) 
# get object files
LIB_AZ_SDK_COBJS = $(call get_relobjs, $(LIB_AZ_SDK_CSRCS))
LIB_AZ_SDK_CXXOBJS = $(call get_relobjs, $(LIB_AZ_SDK_CXXSRCS))
LIB_AZ_SDK_ASMOBJS = $(call get_relobjs, $(LIB_AZ_SDK_ASMSRCS))
LIB_AZ_SDK_OBJS = $(LIB_AZ_SDK_COBJS) $(LIB_AZ_SDK_ASMOBJS) $(LIB_AZ_SDK_CXXOBJS)
#$(warning build neaz_sdkduo lib $(LIB_AZ_SDK_OBJS)) 
# get dependency files
LIB_AZ_SDK_DEPS = $(call get_deps, $(LIB_AZ_SDK_OBJS))

# extra macros to be defined
LIB_AZ_SDK_DEFINES = -DLIB_AZ_SDK

LIB_INCDIR += $(LIB_AZ_SDK_INCDIR)

ifeq ($(LIB_AZ_SDK_PREBUILT), 1)

AZ_SDK_LIB = $(BOARD_OUT_DIR)/$(BUILD_INFO)/libaz_sdk.a
AZ_SDK_PREBUILT_LIB = ./library/az_sdk/prebuilt_lib/libaz_sdk.a

$(AZ_SDK_LIB) :
	$(CP) $(AZ_SDK_PREBUILT_LIB) $(AZ_SDK_LIB)


$(warning $(AZ_SDK_LIB))
$(warning $(AZ_SDK_PREBUILT_LIB))

else

# genearte library
AZ_SDK_LIB = $(OUT_DIR)/libaz_sdk.a


# library generation rule
$(AZ_SDK_LIB): $(LIB_AZ_SDK_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(LIB_AZ_SDK_OBJS)
	$(CP) $(BOARD_OUT_DIR)/$(BUILD_INFO)/libaz_sdk.a ./library/az_sdk/prebuilt_lib/libaz_sdk.a
$(warning ****build LIB_LIB_AZ_SDK lib $(AZ_SDK_LIB) $(Q)$(AR) $(AR_OPT)) 
 
# specific compile rules
# user can add rules to compile this middleware
# if not rules specified to this middleware, it will use default compiling rules

# Middleware Definitions

AZ_SDK_LIB_CSRCDIR += $(LIB_AZ_SDK_CSRCDIR)
AZ_SDK_LIB_CXXSRCDIR += $(LIB_AZ_SDK_CXXSRCDIR)
AZ_SDK_LIB_ASMSRCDIR += $(LIB_AZ_SDK_ASMSRCDIR)

AZ_SDK_LIB_CSRCS += $(LIB_AZ_SDK_CSRCS)
AZ_SDK_LIB_CXXSRCS += $(LIB_AZ_SDK_CXXSRCS)
AZ_SDK_LIB_ASMSRCS += $(LIB_AZ_SDK_ASMSRCS)
AZ_SDK_LIB_ALLSRCS += $(LIB_AZ_SDK_CSRCS) $(LIB_AZ_SDK_ASMSRCS)

AZ_SDK_LIB_COBJS += $(LIB_AZ_SDK_COBJS)
AZ_SDK_LIB_CXXOBJS += $(LIB_AZ_SDK_CXXOBJS)
AZ_SDK_LIB_ASMOBJS += $(LIB_AZ_SDK_ASMOBJS)
AZ_SDK_LIB_ALLOBJS += $(LIB_AZ_SDK_OBJS)

AZ_SDK_LIB_DEFINES += $(LIB_AZ_SDK_DEFINES)
AZ_SDK_LIB_DEPS += $(LIB_AZ_SDK_DEPS)
AZ_SDK_LIB_LIBS += $(LIB_LIB_AZ_SDK)

endif
