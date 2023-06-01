SPECS := -specs=$(dir $(lastword $(MAKEFILE_LIST)))kos-dreamcast.specs

CFLAGS += $(SPECS)
CXXFLAGS += $(SPECS)

Load_Program = dc-tool-ser -t /dev/cu.usbserial-DTDDb115819 -b 115200 -x $<
Clean_Objects = $(RM) *.o *.elf compile_commands.json

.PHONY: compilationdb
compilationdb:
	CC=$(CC) CXX=$(CXX) bear --config $(dir $(lastword $(MAKEFILE_LIST)))../.bear -- $(MAKE)

.PHONY: clean
clean:
	$(Clean_Objects)
