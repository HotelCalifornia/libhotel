################################################################################
######################### User configurable parameters #########################
# filename extensions
CEXTS:=c
ASMEXTS:=s S
CXXEXTS:=cpp c++ cc

# probably shouldn't modify these, but you may need them below
ROOT=.
FWDIR:=$(ROOT)/firmware
BINDIR=$(ROOT)/bin
SRCDIR=$(ROOT)/src
INCDIR=$(ROOT)/include

WARNFLAGS+=
EXTRA_CFLAGS=
EXTRA_CXXFLAGS=-fcoroutines

CXX_STANDARD=gnu++20

# Set to 1 to enable hot/cold linking
USE_PACKAGE:=0

# Add libraries you do not wish to include in the cold image here
# EXCLUDE_COLD_LIBRARIES:= $(FWDIR)/your_library.a
EXCLUDE_COLD_LIBRARIES:= 

# Set this to 1 to add additional rules to compile your project as a PROS library template
IS_LIBRARY:=1
LIBNAME:=libhotel
VERSION:=0.0.1
# EXCLUDE_SRC_FROM_LIB= $(SRCDIR)/unpublishedfile.c
# this line excludes opcontrol.c and similar files
EXCLUDE_SRC_FROM_LIB+=$(foreach file, $(SRCDIR)/main,$(foreach cext,$(CEXTS),$(file).$(cext)) $(foreach cxxext,$(CXXEXTS),$(file).$(cxxext)))

# files that get distributed to every user (beyond your source archive) - add
# whatever files you want here. This line is configured to add all header files
# that are in the the include directory get exported
TEMPLATE_FILES=$(INCDIR)/hotel/**/*.hpp

.DEFAULT_GOAL=quick

DOCKER_USER:=hotelcalifornia
DOCKER_CMD:=docker
DOCKER_COMPOSE_CMD:=$(DOCKER_CMD)-compose

DOCKER_IMAGE:=doxygen-base
DOCKER_IMAGE_DIR:=$(ROOT)/$(DOCKER_IMAGE)
DOCKER_IMAGE_VERSION:=1.0.3
DOCKER_BUILD_CMD:=buildx build
DOCKER_BUILD_ARGS:=--platform linux/amd64,linux/arm64 --push
DOCKER_BUILD_TAG:=hotelcalifornia/$(DOCKER_IMAGE):$(DOCKER_IMAGE_VERSION)

.PHONY: $(DOCKER_IMAGE) push-$(DOCKER_IMAGE) clean-docs site

$(DOCKER_IMAGE):
	@$(DOCKER) $(DOCKER_BUILD_CMD) $(DOCKER_BUILD_ARGS) -t $(DOCKER_BUILD_TAG) $(DOCKER_IMAGE_DIR)

DOCS_OUTPUTS:=html latex xml

clean-docs:
	@rm -rf $(DOCS_OUTPUTS)

site:
	@$(DOCKER_COMPOSE_CMD) up -d

$(ROOT)/html: site
	@$(DOCKER_COMPOSE_CMD) exec site $(ROOT)/m.css/documentation/doxygen.py Doxyfile-mcss --debug

################################################################################
################################################################################
########## Nothing below this line should be edited by typical users ###########
-include ./common.mk
