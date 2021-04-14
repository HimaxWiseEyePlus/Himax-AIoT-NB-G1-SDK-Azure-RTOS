# directory declaration
LIB_AZ_SPHERE_DIR = $(LIBRARIES_ROOT)/az_sphere
$(warning build az_sphere lib) 
ifeq ($(LIB_AZ_SPHERE_PREBUILT), 1)

LIB_AZ_SPHERE_ASMSRCDIR	=
LIB_AZ_SPHERE_CSRCDIR	= 
LIB_AZ_SPHERE_CXXSRCSDIR    = 

else
LIB_AZ_SPHERE_ASMSRCDIR	  = $(LIB_AZ_SPHERE_DIR)
LIB_AZ_SPHERE_CSRCDIR	  = $(LIB_AZ_SPHERE_DIR)
LIB_AZ_SPHERE_CXXSRCSDIR  = $(LIB_AZ_SPHERE_DIR)
endif
LIB_AZ_SPHERE_INCDIR	= $(LIB_AZ_SPHERE_DIR)/inc

# find all the source files in the target directories
LIB_AZ_SPHERE_CSRCS = $(call get_csrcs, $(LIB_AZ_SPHERE_CSRCDIR))
LIB_AZ_SPHERE_CXXSRCS = $(call get_cxxsrcs, $(LIB_AZ_SPHERE_CXXSRCSDIR))
LIB_AZ_SPHERE_ASMSRCS = $(call get_asmsrcs, $(LIB_AZ_SPHERE_ASMSRCDIR))


#$(warning build neaz_SPHEREduo lib $(LIB_AZ_SPHERE_CSRCS)) 
# get object files
LIB_AZ_SPHERE_COBJS = $(call get_relobjs, $(LIB_AZ_SPHERE_CSRCS))
LIB_AZ_SPHERE_CXXOBJS = $(call get_relobjs, $(LIB_AZ_SPHERE_CXXSRCS))
LIB_AZ_SPHERE_ASMOBJS = $(call get_relobjs, $(LIB_AZ_SPHERE_ASMSRCS))
LIB_AZ_SPHERE_OBJS = $(LIB_AZ_SPHERE_COBJS) $(LIB_AZ_SPHERE_ASMOBJS) $(LIB_AZ_SPHERE_CXXOBJS)
#$(warning build neaz_SPHEREduo lib $(LIB_AZ_SPHERE_OBJS)) 
# get dependency files
LIB_AZ_SPHERE_DEPS = $(call get_deps, $(LIB_AZ_SPHERE_OBJS))

# extra macros to be defined
LIB_AZ_SPHERE_DEFINES = -DLIB_AZ_SPHERE

LIB_INCDIR += $(LIB_AZ_SPHERE_INCDIR)

ifeq ($(LIB_AZ_SPHERE_PREBUILT), 1)

AZ_SPHERE_LIB = $(BOARD_OUT_DIR)/$(BUILD_INFO)/libaz_sphere.a
AZ_SPHERE_PREBUILT_LIB = ./library/az_sphere/prebuilt_lib/libaz_sphere.a

$(AZ_SPHERE_LIB) :
	$(CP) $(AZ_SPHERE_PREBUILT_LIB) $(AZ_SPHERE_LIB)


$(warning $(AZ_SPHERE_LIB))
$(warning $(AZ_SPHERE_PREBUILT_LIB))

else

# genearte library
AZ_SPHERE_LIB = $(OUT_DIR)/libaz_sphere.a


# library generation rule
$(AZ_SPHERE_LIB): $(LIB_AZ_SPHERE_OBJS)
	$(TRACE_ARCHIVE)
	$(Q)$(AR) $(AR_OPT) $@ $(LIB_AZ_SPHERE_OBJS)
	$(CP) $(BOARD_OUT_DIR)/$(BUILD_INFO)/libaz_sphere.a ./library/az_sphere/prebuilt_lib/libaz_sphere.a
$(warning ****build LIB_AZ_SPHERE lib $(AZ_SPHERE_LIB) $(Q)$(AR) $(AR_OPT)) 
 
# specific compile rules
# user can add rules to compile this middleware
# if not rules specified to this middleware, it will use default compiling rules

# Middleware Definitions

AZ_SPHERE_LIB_CSRCDIR += $(LIB_AZ_SPHERE_CSRCDIR)
AZ_SPHERE_LIB_CXXSRCDIR += $(LIB_AZ_SPHERE_CXXSRCDIR)
AZ_SPHERE_LIB_ASMSRCDIR += $(LIB_AZ_SPHERE_ASMSRCDIR)

AZ_SPHERE_LIB_CSRCS += $(LIB_AZ_SPHERE_CSRCS)
AZ_SPHERE_LIB_CXXSRCS += $(LIB_AZ_SPHERE_CXXSRCS)
AZ_SPHERE_LIB_ASMSRCS += $(LIB_AZ_SPHERE_ASMSRCS)
AZ_SPHERE_LIB_ALLSRCS += $(LIB_AZ_SPHERE_CSRCS) $(LIB_AZ_SPHERE_ASMSRCS)

AZ_SPHERE_LIB_COBJS += $(LIB_AZ_SPHERE_COBJS)
AZ_SPHERE_LIB_CXXOBJS += $(LIB_AZ_SPHERE_CXXOBJS)
AZ_SPHERE_LIB_ASMOBJS += $(LIB_AZ_SPHERE_ASMOBJS)
AZ_SPHERE_LIB_ALLOBJS += $(LIB_AZ_SPHERE_OBJS)

AZ_SPHERE_LIB_DEFINES += $(LIB_AZ_SPHERE_DEFINES)
AZ_SPHERE_LIB_DEPS += $(LIB_AZ_SPHERE_DEPS)
AZ_SPHERE_LIB_LIBS += $(LIB_LIB_AZ_SPHERE)

endif
