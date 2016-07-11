# Makefile for OS

# Entry point
ENTRYPOINT	= 0x30400

ASM		= nasm
#DASM		= ndisasm
CC		= gcc
LD		= ld
ASMBFLAGS	= -I boot/include/
# flags for compiling kernel file
ASMKFLAGS	= -I include/ -f elf
CFLAGS		= -I include -m32 -c -fno-builtin -fno-stack-protector
LDFLAGS		= -m elf_i386 -s -Ttext $(ENTRYPOINT)
#DASMFLAGS

# TARGETS
ORANGESBOOT	= boot/boot.bin boot/loader.bin
ORANGESKERNEL   = kernel.bin
OBJS		= kernel/kernel.o kernel/start.o kernel/i8259.o kernel/protect.o \
			kernel/global.o lib/kliba.o lib/string.o lib/klib.o 

.PHONY: everything final image clean realclean all buildimg

everything: $(ORANGESBOOT) $(ORANGESKERNEL)

all: realclean everything

final : all clean

image : final buildimg

clean:
	rm -f $(OBJS)

realclean:
	rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL)

buildimg:
	dd if=boot/boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy/
	sudo cp -fv boot/loader.bin /mnt/floppy/
	sudo cp -fv kernel.bin /mnt/floppy/
	sudo umount /mnt/floppy

boot/boot.bin: boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin: boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc \
				boot/include/pm.inc boot/include/lib.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL): $(OBJS)
	$(LD) $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS)

kernel/kernel.o: kernel/kernel.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/start.o: kernel/start.c 
	$(CC) $(CFLAGS) -o $@ $<

kernel/i8259.o: kernel/i8259.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/protect.o: kernel/protect.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/global.o: kernel/global.c
	$(CC) $(CFLAGS) -o $@ $<

lib/kliba.o: lib/kliba.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o: lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/klib.o: lib/klib.c
	$(CC) $(CFLAGS) -o $@ $<
