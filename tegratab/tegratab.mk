# NVIDIA Tegra4 "Tegratab" development system
#
# Copyright (c) 2012-2013, NVIDIA Corporation.  All rights reserved.
# AndroidProducts.mk is included before BoardConfig.mk, variable essential at
# start of build and used in here should always be intialized in this file

$(call inherit-product, $(SRC_TARGET_DIR)/product/generic_no_telephony.mk)

# Thse are default settings, it gets changed as per sku manifest properties
PRODUCT_NAME := tegratab
PRODUCT_DEVICE := tegratab
PRODUCT_MODEL := TegraTab
PRODUCT_MANUFACTURER := NVIDIA
PRODUCT_BRAND := nvidia
TARGET_DEVICE := tegratab

PRODUCT_PROPERTY_OVERRIDES += ro.com.google.clientidbase=android-nvidia

# Rest of the packages
$(call inherit-product, device/nvidia/tegratab/device.mk)
