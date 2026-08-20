
SHELL := /bin/sh

NT_API_PATH ?= ../distingNT_API
INCLUDE_PATH := $(NT_API_PATH)/include
API_HEADER := $(INCLUDE_PATH)/distingnt/api.h

HOST_CXX ?= g++
ARM_CXX ?= arm-none-eabi-c++
ARM_NM ?= arm-none-eabi-nm
ARM_READELF ?= arm-none-eabi-readelf
ARM_SIZE ?= arm-none-eabi-size

BUILD_DIR := build
RELEASE_DIR := release
SOURCE := plugins/Witchboard/Witchboard.cpp
OUTPUT := plugins/Witchboard.o
HOST_TEST := $(BUILD_DIR)/WitchboardCleanTest

HOST_FLAGS := -std=c++11 -O2 -Wall -Wextra -fno-exceptions -fno-rtti
ARM_ARCH := -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb
ARM_FLAGS := -std=c++11 $(ARM_ARCH) -Os -fPIC -fno-rtti -fno-exceptions -Wall

.PHONY: all check-api test hardware inspect verify package clean

all: hardware

check-api:
	@test -f "$(API_HEADER)" || { \
		echo "Missing $(API_HEADER). Set NT_API_PATH to the official distingNT_API checkout." >&2; \
		exit 1; \
	}

$(BUILD_DIR):
	mkdir -p "$@"

$(HOST_TEST): tests/WitchboardCleanTest.cpp $(SOURCE) | check-api $(BUILD_DIR)
	$(HOST_CXX) $(HOST_FLAGS) -I"$(INCLUDE_PATH)" "$<" -o "$@"

test: $(HOST_TEST)
	python3 tests/validate_preset.py
	"$(HOST_TEST)"

hardware: $(OUTPUT)

$(OUTPUT): $(SOURCE) | check-api
	mkdir -p "$(@D)"
	$(ARM_CXX) $(ARM_FLAGS) -I"$(INCLUDE_PATH)" -c "$<" -o /tmp/Witchboard.o
	cp -a /tmp/Witchboard.o "$@"

inspect: hardware
	ARM_NM="$(ARM_NM)" ARM_READELF="$(ARM_READELF)" ARM_SIZE="$(ARM_SIZE)" \
		bash scripts/inspect_object.sh "$(OUTPUT)"

verify: test inspect

package: verify
	OBJECT="$(OUTPUT)" RELEASE_DIR="$(RELEASE_DIR)" bash scripts/package_release.sh

clean:
	-rm -rf -- "$(BUILD_DIR)" "$(RELEASE_DIR)" "$(OUTPUT)"
