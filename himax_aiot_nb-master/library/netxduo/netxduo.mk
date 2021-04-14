# directory declaration
LIB_NETXDUO_DIR = $(LIBRARIES_ROOT)/netxduo
$(warning LIB_NETXDUO)
ifeq ($(LIB_NETXDUO_PREBUILT), 1)
LIB_NETXDUO_ASMSRCDIR	= 
LIB_NETXDUO_CSRCDIR		=
LIB_NETXDUO_CXXSRCSDIR	=
else
LIB_NETXDUO_ASMSRCDIR	= $(LIB_NETXDUO_DIR) $(LIB_NETXDUO_DIR)/common/src $(LIB_NETXDUO_DIR)/crypto_libraries/src  $(LIB_NETXDUO_DIR)/addons $(LIB_NETXDUO_DIR)/addons/dns $(LIB_NETXDUO_DIR)/addons/mqtt $(LIB_NETXDUO_DIR)/addons/cloud $(LIB_NETXDUO_DIR)/nx_secure/src $(LIB_NETXDUO_DIR)/addons/azure_iot #$(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module  $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/collectors $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/src/runtime $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/utils $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/extensions $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/model $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/collectors $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/model/objects $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/utils
LIB_NETXDUO_CSRCDIR		= $(LIB_NETXDUO_DIR) $(LIB_NETXDUO_DIR)/common/src $(LIB_NETXDUO_DIR)/crypto_libraries/src  $(LIB_NETXDUO_DIR)/addons $(LIB_NETXDUO_DIR)/addons/dns $(LIB_NETXDUO_DIR)/addons/mqtt $(LIB_NETXDUO_DIR)/addons/cloud $(LIB_NETXDUO_DIR)/nx_secure/src $(LIB_NETXDUO_DIR)/addons/azure_iot #$(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/collectors $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/src/runtime $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/utils $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/extensions $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/model $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/collectors $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/model/objects $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/utils
LIB_NETXDUO_CXXSRCSDIR	= $(LIB_NETXDUO_DIR) $(LIB_NETXDUO_DIR)/common/src $(LIB_NETXDUO_DIR)/crypto_libraries/src  $(LIB_NETXDUO_DIR)/addons $(LIB_NETXDUO_DIR)/addons/dns $(LIB_NETXDUO_DIR)/addons/mqtt $(LIB_NETXDUO_DIR)/addons/cloud $(LIB_NETXDUO_DIR)/nx_secure/src $(LIB_NETXDUO_DIR)/addons/azure_iot #$(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/collectors $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/src/runtime $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/utils $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/serializer/extensions $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/model $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/collectors $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/src/model/objects $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/src/utils

endif
LIB_NETXDUO_INCDIR		= $(LIB_NETXDUO_DIR) $(LIB_NETXDUO_DIR)/common $(LIB_NETXDUO_DIR)/common/inc $(LIB_NETXDUO_DIR)/crypto_libraries/inc $(LIB_NETXDUO_DIR)/addons/azure_iot $(LIB_NETXDUO_DIR)/addons $(LIB_NETXDUO_DIR)/addons/dns $(LIB_NETXDUO_DIR)/addons/cloud $(LIB_NETXDUO_DIR)/addons/mqtt $(LIB_NETXDUO_DIR)/nx_secure/inc  #$(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module  $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/inc $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/inc $(LIB_NETXDUO_DIR)/addons/azure_iot/azure_iot_security_module/iot-security-module-core/deps/flatcc/include 

# find all the source files in the target directories
LIB_NETXDUO_CSRCS = $(call get_csrcs, $(LIB_NETXDUO_CSRCDIR))
LIB_NETXDUO_CXXSRCS = $(call get_cxxsrcs, $(LIB_NETXDUO_CXXSRCSDIR))
LIB_NETXDUO_ASMSRCS = $(call get_asmsrcs, $(LIB_NETXDUO_ASMSRCDIR))

# get object files
LIB_NETXDUO_COBJS = $(call get_relobjs, $(LIB_NETXDUO_CSRCS))
LIB_NETXDUO_CXXOBJS = $(call get_relobjs, $(LIB_NETXDUO_CXXSRCS))
LIB_NETXDUO_ASMOBJS = $(call get_relobjs, $(LIB_NETXDUO_ASMSRCS))
LIB_NETXDUO_OBJS = $(LIB_NETXDUO_COBJS) $(LIB_NETXDUO_ASMOBJS)

# get dependency files
LIB_NETXDUO_DEPS = $(call get_deps, $(LIB_NETXDUO_OBJS))

# extra macros to be defined
LIB_NETXDUO_DEFINES = -DLIB_NETXDUO

LIB_INCDIR += $(LIB_NETXDUO_INCDIR)

ifeq ($(LIB_NETXDUO_PREBUILT), 1)

NETXDUO_LIB = $(BOARD_OUT_DIR)/$(BUILD_INFO)/libnetxduo.a
NETXDUO_PREBUILT_LIB = ./library/netxduo/prebuilt_lib/libnetxduo.a

$(NETXDUO_LIB) :
		$(CP) $(NETXDUO_PREBUILT_LIB) $(NETXDUO_LIB)

		

$(warning $(NETXDUO_PREBUILT_LIB))

else
# genearte library
NETXDUO_LIB = $(OUT_DIR)/libnetxduo.a

# library generation rule
$(NETXDUO_LIB): $(LIB_NETXDUO_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(LIB_NETXDUO_OBJS)
	$(CP) $(BOARD_OUT_DIR)/$(BUILD_INFO)/libnetxduo.a ./library/netxduo/prebuilt_lib/libnetxduo.a	

	

# specific compile rules
# user can add rules to compile this middleware
# if not rules specified to this middleware, it will use default compiling rules

# Middleware Definitions

NETXDUO_LIB_CSRCDIR += $(LIB_NETXDUO_CSRCDIR)
NETXDUO_LIB_ASMSRCDIR += $(LIB_NETXDUO_ASMSRCDIR)

NETXDUO_LIB_CSRCS += $(LIB_NETXDUO_CSRCS)
NETXDUO_LIB_CXXSRCS +=
NETXDUO_LIB_ASMSRCS += $(LIB_NETXDUO_ASMSRCS)
NETXDUO_LIB_ALLSRCS += $(LIB_NETXDUO_CSRCS) $(LIB_NETXDUO_ASMSRCS)

NETXDUO_LIB_COBJS += $(LIB_NETXDUO_COBJS)
NETXDUO_LIB_CXXOBJS +=
NETXDUO_LIB_ASMOBJS += $(LIB_NETXDUO_ASMOBJS)
NETXDUO_LIB_ALLOBJS += $(LIB_NETXDUO_OBJS)

NETXDUO_LIB_DEFINES += $(LIB_NETXDUO_DEFINES)
NETXDUO_LIB_DEPS += $(LIB_NETXDUO_DEPS)
NETXDUO_LIB_LIBS += $(LIB_NETXDUO)
endif
