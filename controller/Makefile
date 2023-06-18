controller.elf: controller.o
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: load
load: controller.elf
	$(Load_Program)

include ../build/common.mk
