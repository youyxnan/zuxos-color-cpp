LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := zygisk
LOCAL_SRC_FILES := example.cpp
LOCAL_LDLIBS := -llog -ldl
LOCAL_CPP_FEATURES := exceptions
include $(BUILD_SHARED_LIBRARY)
