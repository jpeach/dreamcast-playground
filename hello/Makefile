hello.elf: hello.o
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: load
load: hello.elf
	$(Load_Program)

include ../build/common.mk
