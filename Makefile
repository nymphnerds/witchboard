
ifndef NT_API_PATH
	NT_API_PATH := ../distingNT_API
endif

INCLUDE_PATH := $(NT_API_PATH)/include

source := src/WitchboardClean.cpp
output := plugins/Witchboard.o

all: $(output)

clean:
	-rm -f $(output)

$(output): $(source)
	mkdir -p $(@D)
	arm-none-eabi-c++ -std=c++11 -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -fno-rtti -fno-exceptions -Os -fPIC -Wall -I$(INCLUDE_PATH) -c -o /tmp/Witchboard.o $^
	cp -a /tmp/Witchboard.o $@
