# Copyright (C) 2012 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ifeq ($(TARGET_DEVICE),tegratab)

include $(CLEAR_VARS)

LOCAL_MODULE := libpowerhal
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
# TODO: for now build as a stub until unecessary parts can be stripped out
#       and all the needed edits in frameworks are implemented

#ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS),TRUE)
#    ifeq ($(BOARD_USES_POWERHAL),true)
#        LOCAL_SRC_FILES += nvpowerhal.cpp timeoutpoker.cpp
#    else
#        LOCAL_SRC_FILES += powerhal_stub.cpp
#    endif
#else
    LOCAL_SRC_FILES += powerhal_stub.cpp
#endif

LOCAL_SRC_FILES += powerhal_utils.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils libutils
LOCAL_MODULE_TAGS := optional
include $(BUILD_STATIC_LIBRARY)

# HAL module implemenation stored in
# hw/<POWERS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libcutils libutils libdl
LOCAL_STATIC_LIBRARIES := libpowerhal
LOCAL_SRC_FILES := tegratab_power.cpp
LOCAL_MODULE := power.tegratab
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

endif