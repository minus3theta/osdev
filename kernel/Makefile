TARGET = kernel.elf
OBJS = main.o

CXX = clang++
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
	-fno-exceptions -fno-rtti -std=c++17

LD = ld.lld
LDFLAGS += --entry KernelMain -z norelro --image-base 0x100000 --static

.PHONY: all clean

all: $(TARGET)

clean:
	$(RM) *.o

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.elf $^

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<
