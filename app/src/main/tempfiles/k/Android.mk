#
# This confidential and proprietary software may be used only as
# authorised by a licensing agreement from ARM Limited
# (C) COPYRIGHT 2014 ARM Limited
# ALL RIGHTS RESERVED
# The entire notice above must be reproduced on all authorised
# copies and copies may only be made to the extent permitted
# by a licensing agreement from ARM Limited.
#

# Set the LOCAL_PATH variable to the current directory.
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Add the simple-framework include directories
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../common_native/inc/ \
                    $(LOCAL_PATH)/../../common_native/inc/mali/

# Retrieve the name of all the .cpp files in the current folder
ALL_SOURCES := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(ALL_SOURCES:$(LOCAL_PATH)/%=%)

# Retrieve the name of all the .c files in the current folder
ALL_SOURCES := $(wildcard $(LOCAL_PATH)/*.c)
LOCAL_SRC_FILES += $(ALL_SOURCES:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS += -Werror -D__android__
LOCAL_CPPFLAGS += -fno-rtti

# Build as a library
LOCAL_MODULE := Native

# Link with OpenGL ES 3.0 libraries
LOCAL_LDLIBS := -llog -lGLESv3 -lm

# Link with the common_native
LOCAL_SHARED_LIBRARIES := common_native

include $(BUILD_SHARED_LIBRARY)
