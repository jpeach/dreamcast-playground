SPECS := -specs=$(dir $(lastword $(MAKEFILE_LIST)))/kos-dreamcast.specs

CC := $(KOS_CC_BASE)/bin/$(KOS_CC_PREFIX)-gcc $(SPECS)
CXX := $(KOS_CC_BASE)/bin/$(KOS_CC_PREFIX)-g++ $(SPECS)

Load_Program = dc-tool-ser -t /dev/cu.usbserial-DTDDb115819 -b 115200 -x $<
Clean_Objects = $(RM) *.o *.elf
