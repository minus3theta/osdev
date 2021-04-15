TARGET = kernel.elf
OBJS = main.o graphics.o font.o hankaku.o newlib_support.o console.o
DEPENDS = $(join $(dir $(OBJS)),$(addprefix .,$(notdir $(OBJS:.o=.d))))

CC = clang
CFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone
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
	$(LD) $(LDFLAGS) -o kernel.elf $^ -lc

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

.%.d: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.cpp=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

.%.d: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.c=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

hankaku.bin: hankaku.txt
	../tools/makefont.py -o $@ $<

hankaku.o: hankaku.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

.%.d: %.bin
	touch $@

depends:
	$(Make) $(DEPENDS)

-include $(DEPENDS)
