megs: 32

romimage: file=/usr/local/share/bochs/BIOS-bochs-latest
vgaromimage: file=/usr/local/share/bochs/VGABIOS-lgpl-latest

floppya: 1_44=a.img, status=inserted

boot: floppy

log: bochsout.txt

mouse: enabled=0

keyboard: keymap=/usr/local/share/bochs/keymaps/x11-pc-us.map

ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata0-master: type=disk, path="c.img", mode=flat, cylinders=162, heads=16, spt=63

gdbstub: enabled=1, port=1234, text_base=0, data_base=0, bss_base=0