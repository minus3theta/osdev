TARGET = kernel.elf
OBJS = main.o graphics.o font.o hankaku.o newlib_support.o console.o pci.o asmfunc.o \
	logger.o libcxx_support.o mouse.o interrupt.o segment.o paging.o \
	memory_manager.o window.o layer.o timer.o frame_buffer.o acpi.o \
	keyboard.o task.o terminal.o fat.o syscall.o \
	usb/memory.o usb/device.o usb/xhci/ring.o usb/xhci/trb.o usb/xhci/xhci.o \
	usb/xhci/port.o usb/xhci/device.o usb/xhci/devmgr.o usb/xhci/registers.o \
	usb/classdriver/base.o usb/classdriver/hid.o usb/classdriver/keyboard.o \
	usb/classdriver/mouse.o
DEPENDS = $(join $(dir $(OBJS)),$(addprefix .,$(notdir $(OBJS:.o=.d))))

CC = clang
CPPFLAGS += -I.
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
	$(LD) $(LDFLAGS) -o kernel.elf $^ -lc -lc++ -lc++abi -lm

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.%.d: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.cpp=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.%.d: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.c=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

%.o: %.asm
	nasm -f elf64 -o $@ $<

hankaku.bin: hankaku.txt
	../tools/makefont.py -o $@ $<

hankaku.o: hankaku.bin
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

.%.d: %.bin
	touch $@

depends:
	$(Make) $(DEPENDS)

-include $(DEPENDS)
