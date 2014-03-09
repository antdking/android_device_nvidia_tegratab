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
LOCAL_PATH:= $(call my-dir)
LOCAL_SENSOR_HEADERS := $(LOCAL_PATH)
LOCAL_SENSOR_HEADERS += $(LOCAL_PATH)/mlsdk/driver/include
LOCAL_SENSOR_HEADERS += $(LOCAL_PATH)/mlsdk/driver/include/linux
LOCAL_SENSOR_HEADERS += $(LOCAL_PATH)/mlsdk/HAL/include
LOCAL_SENSOR_HEADERS += $(LOCAL_PATH)/mlsdk/mllite
LOCAL_SENSOR_HEADERS += $(LOCAL_PATH)/mlsdk/mllite/linux
LOCAL_SENSOR_HEADERS += $(LOCAL_PATH)/mlsdk/mpl

include $(CLEAR_VARS)
LOCAL_MODULE := libsensors.base
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"
LOCAL_SRC_FILES := SensorBase.cpp SensorUtil.cpp InputEventReader.cpp
LOCAL_C_INCLUDES += $(LOCAL_SENSOR_HEADERS)
LOCAL_SHARED_LIBRARIES := liblog libdl libcutils libutils
LOCAL_CPPFLAGS+=-DLINUX=1
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libsensors.nvs_input
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"
LOCAL_SRC_FILES := nvs_input.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES := liblog libsensors.base
LOCAL_CPPFLAGS+=-DLINUX=1
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libsensors.mpl
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"
LOCAL_CFLAGS += -DINVENSENSE_COMPASS_CAL
LOCAL_CFLAGS += -std=gnu++0x
LOCAL_C_INCLUDES += $(LOCAL_SENSOR_HEADERS)
LOCAL_SRC_FILES := MPLSensor.cpp MPLSupport.cpp CompassSensor.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils libutils libdl libsensors.base \
			  libmllite libmplmpu libsensors.nvs_input
LOCAL_CPPFLAGS+=-DLINUX=1
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

# HAL module implemenation, not prelinked and stored in
# hw/<COPYPIX_HARDWARE_MODULE_ID>.<ro.board.platform>.so
include $(CLEAR_VARS)

include $(CLEAR_VARS)
LOCAL_MODULE := libsensors.cm3217
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"
LOCAL_SRC_FILES := cm3217.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES := liblog libsensors.base
LOCAL_CPPFLAGS+=-DLINUX=1
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

LOCAL_CFLAGS := -DLOG_TAG=\"Sensors\"
LOCAL_SRC_FILES := tegratab_sensors.cpp
LOCAL_C_INCLUDES += $(LOCAL_SENSOR_HEADERS)
LOCAL_SHARED_LIBRARIES := liblog libcutils libutils libdl libsensors.base \
                          libinvensense_hal libsensors.mpl \
                          libsensors.cm3217 libsensors.nvs_input
LOCAL_CPPFLAGS+=-DLINUX=1
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := sensors.tegratab

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/mlsdk/Android.mk
endif