TARGET = kernel.elf
OBJS = main.o graphics.o font.o hankaku.o
DEPENDS = $(join $(dir $(OBJS)),$(addprefix .,$(notdir $(OBJS:.o=.d))))

CXX = clang++
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
	-fno-exceptions -fno-rtti -std=c++17

LD = ld.lld
LDFLAGS += --entry KernelMain -z norelro --image-base 0x100000 --static

.PHONY: all clean depends

all: $(TARGET)

clean:
	$(RM) *.o

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.elf $^

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

.%.d: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.cpp=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

hankaku.bin: hankaku.txt
	../tools/makefont.py -o $@ $<

hankaku.o: hankaku.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

depends:
	$(Make) $(DEPENDS)

-include $(DEPENDS)
