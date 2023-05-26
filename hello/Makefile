include ../build/common.mk

hello.elf: hello.o
	$(CXX) -o $@ $^

.PHONY: load
load: hello.elf
	$(Load_Program)

.PHONY: clean
clean:
	$(Clean_Objects)
