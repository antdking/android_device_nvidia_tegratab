# NVIDIA Tegra4 "Tegratab" development system
#
# Copyright (c) 2012-2013 NVIDIA Corporation.  All rights reserved.

$(call inherit-product-if-exists, vendor/nvidia/tegra/core/nvidia-tegra-vendor.mk)
$(call inherit-product-if-exists, frameworks/base/data/videos/VideoPackage2.mk)
$(call inherit-product-if-exists, frameworks/base/data/sounds/AudioPackage3.mk)
$(call inherit-product-if-exists, vendor/nvidia/tegra/apps/tfc/tfc.mk)
$(call inherit-product, build/target/product/languages_full.mk)

PRODUCT_LOCALES += mdpi hdpi xhdpi

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml

PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/configs/root/ueventd.tegratab.rc:root/ueventd.tegratab.rc \
  $(LOCAL_PATH)/configs/usr/tegra-kbc.kl:system/usr/keylayout/tegra-kbc.kl \
  $(LOCAL_PATH)/configs/usr/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
  $(LOCAL_PATH)/configs/etc/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
  $(LOCAL_PATH)/configs/usr/raydium_ts.idc:system/usr/idc/raydium_ts.idc \
  $(LOCAL_PATH)/configs/usr/sensor00fn11.idc:system/usr/idc/sensor00fn11.idc \
  $(LOCAL_PATH)/configs/bin/add_p2p_iface.sh:system/bin/add_p2p_iface.sh \
  $(LOCAL_PATH)/configs/usr/touch_fusion.idc:system/usr/idc/touch_fusion.idc \
  $(LOCAL_PATH)/configs/bin/ussr_setup.sh:system/bin/ussr_setup.sh \
  $(LOCAL_PATH)/configs/bin/input_cfboost_init.sh:system/bin/input_cfboost_init.sh \
  $(LOCAL_PATH)/configs/bin/set_hwui_params.sh:system/bin/set_hwui_params.sh \

PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/configs/bin/sensor_init.sh:system/bin/sensor_init.sh

#ifeq ($(NV_ANDROID_FRAMEWORK_ENHANCEMENTS),TRUE)
#PRODUCT_COPY_FILES += \
#  $(LOCAL_PATH)/configs/etc/media_profiles.xml:system/etc/media_profiles.xml \
#  $(LOCAL_PATH)/configs/etc/media_codecs.xml:system/etc/media_codecs.xml \
#  $(LOCAL_PATH)/configs/etc/audio_policy.conf:system/etc/audio_policy.conf
#else
PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/configs/etc/media_profiles_noenhance.xml:system/etc/media_profiles.xml \
  $(LOCAL_PATH)/configs/etc/media_codecs_noenhance.xml:system/etc/media_codecs.xml \
  $(LOCAL_PATH)/configs/etc/audio_policy_noenhance.conf:system/etc/audio_policy.conf
#endif

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/etc/power.tegratab.rc:system/etc/power.tegratab.rc \
    $(LOCAL_PATH)/configs/root/init.hdcp.rc:root/init.hdcp.rc \
    $(LOCAL_PATH)/configs/root/init.qvs.rc:root/init.qvs.rc \
    $(LOCAL_PATH)/configs/root/init.tegratab.rc:root/init.tegratab.rc \
    $(LOCAL_PATH)/configs/root/fstab.tegratab:root/fstab.tegratab \
    $(LOCAL_PATH)/configs/root/init.tegratab.usb.rc:root/init.tegratab.usb.rc

# Face detection model
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/etc/model_frontalface.xml:system/etc/model_frontal.xml

# Test files
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/bin/cluster:system/bin/cluster \
    $(LOCAL_PATH)/configs/bin/cluster_get.sh:system/bin/cluster_get.sh \
    $(LOCAL_PATH)/configs/bin/cluster_set.sh:system/bin/cluster_set.sh \
    $(LOCAL_PATH)/configs/bin/hotplug:system/bin/hotplug \
    $(LOCAL_PATH)/configs/bin/mount_debugfs.sh:system/bin/mount_debugfs.sh

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/lib/egl/egl.cfg:system/lib/egl/egl.cfg

PRODUCT_COPY_FILES += \
    device/nvidia/tegratab/nvcms/device.cfg:system/lib/nvcms/device.cfg

PRODUCT_COPY_FILES += \
	external/alsa-lib/src/conf/alsa.conf:system/usr/share/alsa/alsa.conf \
	external/alsa-lib/src/conf/pcm/dsnoop.conf:system/usr/share/alsa/pcm/dsnoop.conf \
	external/alsa-lib/src/conf/pcm/modem.conf:system/usr/share/alsa/pcm/modem.conf \
	external/alsa-lib/src/conf/pcm/dpl.conf:system/usr/share/alsa/pcm/dpl.conf \
	external/alsa-lib/src/conf/pcm/default.conf:system/usr/share/alsa/pcm/default.conf \
	external/alsa-lib/src/conf/pcm/surround51.conf:system/usr/share/alsa/pcm/surround51.conf \
	external/alsa-lib/src/conf/pcm/surround41.conf:system/usr/share/alsa/pcm/surround41.conf \
	external/alsa-lib/src/conf/pcm/surround50.conf:system/usr/share/alsa/pcm/surround50.conf \
	external/alsa-lib/src/conf/pcm/dmix.conf:system/usr/share/alsa/pcm/dmix.conf \
	external/alsa-lib/src/conf/pcm/center_lfe.conf:system/usr/share/alsa/pcm/center_lfe.conf \
	external/alsa-lib/src/conf/pcm/surround40.conf:system/usr/share/alsa/pcm/surround40.conf \
	external/alsa-lib/src/conf/pcm/side.conf:system/usr/share/alsa/pcm/side.conf \
	external/alsa-lib/src/conf/pcm/iec958.conf:system/usr/share/alsa/pcm/iec958.conf \
	external/alsa-lib/src/conf/pcm/rear.conf:system/usr/share/alsa/pcm/rear.conf \
	external/alsa-lib/src/conf/pcm/surround71.conf:system/usr/share/alsa/pcm/surround71.conf \
	external/alsa-lib/src/conf/pcm/front.conf:system/usr/share/alsa/pcm/front.conf \
	external/alsa-lib/src/conf/cards/aliases.conf:system/usr/share/alsa/cards/aliases.conf \
	$(LOCAL_PATH)/configs/etc/asound.conf:system/etc/asound.conf

PRODUCT_COPY_FILES += \
	$(LOCAL_PATH)/configs/etc/enctune.conf:system/etc/enctune.conf

# nvcpud specific cpu frequencies config
PRODUCT_COPY_FILES += \
        $(LOCAL_PATH)/configs/etc/nvcpud.conf:system/etc/nvcpud.conf

#enable Widevine drm
PRODUCT_PROPERTY_OVERRIDES += drm.service.enabled=true
PRODUCT_PACKAGES += \
    com.google.widevine.software.drm.xml \
    com.google.widevine.software.drm \
    libdrmwvmplugin \
    libwvm \
    libWVStreamControlAPI_L1 \
    libwvdrm_L1

# Live Wallpapers
PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    HoloSpiralWallpaper \
    MagicSmokeWallpapers \
    NoiseField \
    Galaxy4 \
    VisualizationWallpapers \
    PhaseBeam \
    librs_jni

PRODUCT_PACKAGES += \
	sensors.tegratab \
	lights.tegratab \
	audio.primary.tegra \
	audio.a2dp.default \
	audio.usb.default \
	audio_policy.tegra \
	audio.r_submix.default \
	power.tegratab \
	setup_fs \
	drmserver \
	Gallery2 \
	libdrmframework_jni \
	e2fsck \
        NVSS

PRODUCT_PACKAGES += \
	charger\
	charger_res_images\

PRODUCT_PACKAGES += nvaudio_test

#WiFi
PRODUCT_PACKAGES += \
		TQS_S_2.6.ini \
		iw \
		crda \
		regulatory.bin \
		wpa_supplicant.conf \
		p2p_supplicant.conf \
		p2p_disabled.conf \
		hostapd.conf \
		ibss_supplicant.conf \
		dhcpd.conf \
		dhcpcd.conf

#Wifi firmwares
PRODUCT_PACKAGES += \
		wl1271-nvs_default.bin \
		wl128x-fw-4-sr.bin \
		wl128x-fw-4-mr.bin \
		wl128x-fw-4-plt.bin \
		wl18xx-fw-mc.bin \
		wl18xx-fw-mc_pg22.bin \
		wl18xx-fw-2.bin \
		wl1271-nvs_wl8.bin

#BT & FM packages
PRODUCT_PACKAGES += \
		uim-sysfs \
		TIInit_10.6.15.bts \
		TIInit_11.8.32.bts \
		TIInit_12.8.32.bts

#GPS
PRODUCT_PACKAGES += \
		agnss_connect \
		client_app \
		client_hwd \
		Connect_Config.txt \
		devproxy \
		dproxy.conf \
		dproxy.patch \
		gps.tegra.so \
		hwd \
		libagnss.so \
		libassist.so \
		libclientlogger.so \
		libdevproxy.so \
		libgnssutils.so \
		Log_MD \
		log_MD.txt \
		logs.txt \
		nvs.txt \
		ser2soc \
		test_server
# CPU volt cap daemon
PRODUCT_PACKAGES += \
	nvcpuvoltcapd

# promotional contents
PRODUCT_PACKAGES += \
	media_ad

# Factory test packages
PRODUCT_PACKAGES += \
	nvbdktestbl \
        pcba_testcases.xml \
        postassembly_testcases.xml \
        preassembly_testcases.xml \
        audio_testcases.xml \
        usbhostumsread \
	tdc \
	tmc \
	tst \
	camera_existence \
	factory_adbd \
	oemcrypto_api_test

PRODUCT_PACKAGES += \
    nvflash_cfg_populator \
    lbh_images

# Markers app (renamed to Tegra Draw)
PRODUCT_PACKAGES += \
        TegraDraw

# HDCP SRM Support
PRODUCT_PACKAGES += \
                hdcp1x.srm \
                hdcp2x.srm \
                hdcp2xtest.srm

# OV5693 bayer sensor calibration manager
PRODUCT_PACKAGES += otp-ov5693

PRODUCT_PACKAGES += ControllerMapper

ifneq ($(filter $(TARGET_PRODUCT),kalamata flaxen),)
PRODUCT_PACKAGES += gen_tegranote_fuseblob
endif

include frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk
include $(LOCAL_PATH)/touchscreen/maxim/maxim.mk
# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

PRODUCT_CHARACTERISTICS := tablet

PRODUCT_PACKAGE_OVERLAYS := $(LOCAL_PATH)/overlay

# Set default USB interface
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

# Set DPI
PRODUCT_PROPERTY_OVERRIDES += ro.sf.lcd_density=213
