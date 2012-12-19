#
# Copyright (c) 2008-2012 Instrumentation Technologies
# All Rights Reserved.
#
# $Id: $
#

SRC_TOP=.
TARGET_NAME = platform-dummy

export BASE_VER?=2.6

ifeq ($(BASE_LIB_DIR),)
  #if EBPM_LIB_DIR is not defined, libraries are taken from installed package
  LIBIAPP  = libiapp$(BASE_VER).a
  LIBISTD  = libistd$(BASE_VER).a
  LIBICFG  = libicfg$(BASE_VER).a
  LIBMCI   = libmci$(BASE_VER).a
  LIBIREG  = libireg$(BASE_VER).a
  LIBINET  = libinet$(BASE_VER).a
  LIBBMC   = libbmc$(BASE_VER).a
  LIBISIG  = libisig$(BASE_VER).a
else
  #if EBPM_LIB_DIR is defined, libraries are taken from development branch from appropriate subfolders
  LIBIAPP  = iapp/libiapp$(BASE_VER).a
  LIBISTD  = istd/libistd$(BASE_VER).a
  LIBMCI   = mci/libmci$(BASE_VER).a
  LIBICFG  = icfg/libicfg$(BASE_VER).a
  LIBIREG  = ireg/libireg$(BASE_VER).a
  LIBINET  = inet/libinet$(BASE_VER).a
  LIBBMC   = bmc/libbmc$(BASE_VER).a
  LIBISIG  = isig/libisig$(BASE_VER).a
endif

BASE_INC_DIR?=/usr/include/libera-$(BASE_VER)
BASE_LIB_DIR?=/opt/libera/lib
BASE_MKF_DIR?=/usr/share/libera-$(BASE_VER)-dev

include $(BASE_MKF_DIR)/MakeGeneral

OBJECT_FILES = platform_dummy.o

TARGETS=$(TARGET_NAME)

INCLUDE_DIRS+= $(BASE_INC_DIR) $(CMN_INCLUDE) 

ifeq ($(RELEASE_BUILD),1)
$(info RELEASE BUILD)

    LDFLAGS+=-L/opt/libera/lib

    LDLIBS+= \
        -lliberaiapp$(BASE_VER) \
        -lliberaireg$(BASE_VER) \
        -lliberamci$(BASE_VER) \
        -lliberaisig$(BASE_VER) \
        -lliberaistd$(BASE_VER) \
        -lomniORB4 -lomnithread
else

    LDLIBS+= \
        $(BASE_LIB_DIR)/$(LIBIAPP) \
        $(BASE_LIB_DIR)/$(LIBIREG) \
        $(BASE_LIB_DIR)/$(LIBMCI) \
        $(BASE_LIB_DIR)/$(LIBISIG) \
        $(BASE_LIB_DIR)/$(LIBBMC) \
        $(BASE_LIB_DIR)/$(LIBICFG) \
        $(BASE_LIB_DIR)/$(LIBINET) \
        $(BASE_LIB_DIR)/$(LIBISTD) \
        -lcrypto \
        -lboost_program_options \
        -lxml2 \
        -lsensors \
        -lstatgrab \
        -lusb-1.0 \
        -lopenhpi -lopenhpiutils \
        -lomniORB4 -lomnithread -lomniDynamic4
endif

include $(BASE_MKF_DIR)/MakeRules
EXTRA_CXX_FLAGS+=-Wold-style-cast


all: $(TARGETS)

install:
	$(INSTALL) -o root -g root -m 755 $(TARGET_NAME) $(INST_SBIN)
	$(INSTALL) -o root -g root -m 755 conf/$(TARGET_NAME)-prerm \
		$(INST_KERNEL_PRERM)/$(TARGET_NAME)
	$(INSTALL) -o root -g root -m 755 conf/$(TARGET_NAME)-postinst \
		$(INST_KERNEL_POSTINST)/$(TARGET_NAME)
	$(INSTALL) -o root -g root -m 644 conf/$(TARGET_NAME) $(INST_LOG_ROTATE)
