#
# Copyright (c) 2008-2009 Instrumentation Technologies
# All Rights Reserved.
#
# $Id: Makefile 14606 2011-12-21 12:43:41Z matej.kenda $
#

export REVISION
export VER_BUILD

override SRC_TOP:=.

# These rules are local and will not be passed to $(SUBDIRS)
LOCAL_RULES=deb show_env dupload
.PHONY: $(LOCAL_RULES)

# Build these subdirectories
SUBDIRS = src

export BASE_VER?=2.6

BASE_INC_DIR?=/usr/include/libera-$(BASE_VER)
BASE_LIB_DIR?=/opt/libera/lib
BASE_MKF_DIR?=/usr/share/libera-$(BASE_VER)-dev

include $(BASE_MKF_DIR)/MakeGeneral
include $(BASE_MKF_DIR)/MakeDir

# ------------------------------------------------------------------------------
# Packaging
# ------------------------------------------------------------------------------

DEBUILD_OPTS += --preserve-envvar DEBFULLNAME --preserve-envvar DEBEMAIL --preserve-envvar VER
DEBUILD_OPTS += --preserve-envvar CXX
DEBUILD_OPTS += --set-envvar REVISION=$(REVISION) --set-envvar VER_BUILD=$(VER_BUILD) 
DEBUILD_OPTS += --set-envvar RELEASE_BUILD=$(RELEASE_BUILD) 
DEBUILD_OPTS += --set-envvar OPTIMISED_BUILD=$(OPTIMISED_BUILD) 

ifeq ($(RELEASE_BUILD),1)
ifeq ($(MACHINE_TYPE),x86_64)
  DEBUILD_OPTS += --set-envvar KERNEL_VERSION=2.6.38-10-server
else
  DEBUILD_OPTS += --set-envvar KERNEL_VERSION=2.6.38-10-generic-pae
endif
else
  # not release build
  DEBUILD_OPTS += --preserve-envvar KERNEL_VERSION
endif

$(info Release build- "$(RELEASE_BUILD)" )

deb:
	$(MAKE) -f debian/rules update-changelog VER=$(VER_DPKG) LABEL=$(VER_LABEL)
	fakeroot $(MAKE) -f debian/rules clean
	debuild $(DEBUILD_OPTS) --no-tgz-check -b -us -uc

DUPLOAD_TARGET_NAME?=devel

dupload:
	@echo "--> Uploading packages with dupload"
	@cd $(SRC_TOP)/.. ; dupload -t $(DUPLOAD_TARGET_NAME) libera-dummy_*.changes

# ------------------------------------------------------------------------------
# Helper targets
# ------------------------------------------------------------------------------

show_env:
	$(info SRC_TOP     $(SRC_TOP))
	$(info SRC_PATH    $(SRC_PATH))
	$(info LOGNAME     $(LOGNAME) )
	$(info PLATFORM    $(PLATFORM))
	$(info VER_DPKG    $(VER_DPKG))
	$(info VER_MAJOR   $(VER_MAJOR))
	$(info VER_MINOR   $(VER_MINOR))
	$(info VER_LABEL   $(VER_LABEL))
	$(info VER_BUILD   $(VER_BUILD))
	$(info REVISION    $(REVISION))
	$(info VERSION_DIR $(VERSION_DIR))
	$(info VERSION_TXT $(VERSION_TXT))
	$(info VERSION_SRC $(VERSION_SRC))
	$(info VERSION_OBJ $(VERSION_OBJ))
	$(info VERSIONSTR  $(VERSIONSTR))
	$(info RELEASE_BUILD $(RELEASE_BUILD))
	$(info VER_BUILDUSER $(VER_BUILDUSER))
	$(info RELEASE_BUILD_USER $(RELEASE_BUILD_USER))
	$(info VER_BUILDTIME $(VER_BUILDTIME))
	$(info BUILD_LABEL_TXT $(BUILD_LABEL_TXT))
	$(info VER_COMPILERINFO $(VER_COMPILERINFO))
	$(info VER_BUILDMACHINE $(VER_BUILDMACHINE))
	$(info VERSIONDEF  $(VERSIONDEF))
	$(info BUILD_ENV   $(BUILD_ENV))

