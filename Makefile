# Makefile for OS

# Entry point
ENTRYPOINT	= 0x1000

ASM		= nasm
#DASM		= ndisasm
CC		= gcc
LD		= ld
ASMBFLAGS	= -I boot/include/
# flags for compiling kernel file
ASMKFLAGS	= -I include/ -f elf
CFLAGS		= -I include -m32 -c -fno-builtin -fno-stack-protector  -g
LDFLAGS		= -m elf_i386  -Ttext $(ENTRYPOINT)
#DASMFLAGS

# TARGETS
ORANGESBOOT	= boot/boot.bin boot/loader.bin
ORANGESKERNEL   = kernel.bin
OBJS		= kernel/kernel.o kernel/start.o kernel/i8259.o kernel/protect.o \
		kernel/global.o lib/kliba.o lib/string.o lib/klib.o kernel/main.o \
		kernel/clock.o kernel/syscall.o kernel/proc.o kernel/keyboard.o \
		kernel/tty.o kernel/console.o kernel/printf.o lib/misc.o \
		kernel/systask.o kernel/hd.o kernel/fs.o kernel/keymap.o fslib/misc.o \
		fslib/open.o fslib/read_write.o fslib/link.o fslib/lseek.o kernel/mm.o \
		lib/fork.o lib/getpid.o lib/exit.o mmlib/do_exec.o fslib/do_stat.o

LIBOBJS         = lib/libexit.o lib/libwait.o lib/libprintf.o lib/libopen.o \
		lib/libclose.o lib/libfork.o lib/libwrite.o lib/libread.o \
		lib/libvsprintf.o lib/libunlink.o lib/exec.o lib/libstat.o

CRT             = lib/oscrt.a

.PHONY: everything final image clean realclean all buildimg

everything: $(ORANGESBOOT) $(ORANGESKERNEL) $(CRT)

all: realclean everything

final : all clean

image : final buildimg

clean:
	rm -f $(OBJS)

realclean:
	rm -f $(OBJS) $(ORANGESBOOT) $(ORANGESKERNEL) $(LIBOBJS) $(CRT)

buildimg:
	dd if=boot/boot.bin of=a.img bs=512 count=1 conv=notrunc
	sudo mount -o loop a.img /mnt/floppy/
	strip kernel.bin -o kernel.bin.strip
	sudo cp -fv boot/loader.bin /mnt/floppy/
	sudo cp -fv kernel.bin.strip /mnt/floppy/kernel.bin
	sudo umount /mnt/floppy

boot/boot.bin: boot/boot.asm boot/include/load.inc boot/include/fat12hdr.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

boot/loader.bin: boot/loader.asm boot/include/load.inc boot/include/fat12hdr.inc \
				boot/include/pm.inc boot/include/lib.inc
	$(ASM) $(ASMBFLAGS) -o $@ $<

$(ORANGESKERNEL): $(OBJS) $(LIBOBJS)
	$(LD) $(LDFLAGS) -o $(ORANGESKERNEL) $(OBJS) $(LIBOBJS)

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

kernel/main.o: kernel/main.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/clock.o: kernel/clock.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/proc.o: kernel/proc.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/syscall.o: kernel/syscall.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

kernel/keyboard.o: kernel/keyboard.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/tty.o: kernel/tty.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/console.o: kernel/console.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/printf.o: kernel/printf.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/systask,o: kernel/systask.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/hd.o: kernel/hd.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/fs.o: kernel/fs.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/keymap.o: kernel/keymap.c
	$(CC) $(CFLAGS) -o $@ $<

kernel/mm.o: kernel/mm.c
	$(CC) $(CFLAGS) -o $@ $<

fslib/misc.o: fslib/misc.c
	$(CC) $(CFLAGS) -o $@ $<

fslib/open.o: fslib/open.c
	$(CC) $(CFLAGS) -o $@ $<

fslib/read_write.o: fslib/read_write.c
	$(CC) $(CFLAGS) -o $@ $<

fslib/link.o: fslib/link.c
	$(CC) $(CFLAGS) -o $@ $<

fslib/lseek.o: fslib/lseek.c
	$(CC) $(CFLAGS) -o $@ $<

fslib/do_stat.o: fslib/do_stat.c
	$(CC) $(CFLAGS) -o $@ $<

mmlib/do_exec.o: mmlib/do_exec.c
	$(CC) $(CFLAGS) -o $@ $<

lib/kliba.o: lib/kliba.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/string.o: lib/string.asm
	$(ASM) $(ASMKFLAGS) -o $@ $<

lib/klib.o: lib/klib.c
	$(CC) $(CFLAGS) -o $@ $<

lib/fork.o: lib/fork.c
	$(CC) $(CFLAGS) -o $@ $<

lib/misc.o: lib/misc.c
	$(CC) $(CFLAGS) -o $@ $<

lib/getpid.o: lib/getpid.c
	$(CC) $(CFLAGS) -o $@ $<

lib/exit.o: lib/exit.c
	$(CC) $(CFLAGS) -o $@ $<

# LIB OBJs

lib/libexit.o: lib/libexit.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libwait.o: lib/libwait.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libprintf.o: lib/libprintf.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libopen.o: lib/libopen.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libclose.o: lib/libclose.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libfork.o: lib/libfork.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libwrite.o: lib/libwrite.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libread.o: lib/libread.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libvsprintf.o: lib/libvsprintf.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libunlink.o: lib/libunlink.c
	$(CC) $(CFLAGS) -o $@ $<

lib/exec.o: lib/exec.c
	$(CC) $(CFLAGS) -o $@ $<

lib/libstat.o: lib/libstat.c
	$(CC) $(CFLAGS) -o $@ $<

# CRT
lib/oscrt.a:
	ar rcs lib/oscrt.a kernel/syscall.o lib/string.o lib/libopen.o lib/libread.o \
	lib/libexit.o lib/libprintf.o lib/libclose.o lib/libfork.o lib/libwrite.o \
	lib/misc.o lib/libvsprintf.o kernel/printf.o lib/libunlink.o

# Command
