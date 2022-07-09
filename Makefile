ifdef $(GENDEV)
ROOTDIR = $(GENDEV)
else
ROOTDIR = /opt/toolchains/sega
endif

LDSCRIPTSDIR = $(ROOTDIR)/ldscripts

BOOTBLOCKDIR = $(ROOTDIR)/bootblocks
#BOOTBLOCK = $(BOOTBLOCKDIR)/US_BOOT.BIN
BOOTBLOCK = $(BOOTBLOCKDIR)/EU_BOOT.BIN
#BOOTBLOCK = $(BOOTBLOCKDIR)/JP_BOOT.BIN

LIBPATH = -L$(ROOTDIR)/m68k-elf/lib -L$(ROOTDIR)/m68k-elf/lib/gcc/m68k-elf/4.6.2 -L$(ROOTDIR)/m68k-elf/m68k-elf/lib
INCPATH = -I. -I../include -I$(ROOTDIR)/m68k-elf/include -I$(ROOTDIR)/m68k-elf/m68k-elf/include

MDCCFLAGS = -m68000 -Wall -Ofast -c -fomit-frame-pointer
MDHWFLAGS = -m68000 -Wall -O1 -c -fomit-frame-pointer
MDLDFLAGS = -T $(LDSCRIPTSDIR)/cd.ld -Wl,-Map=output.map -nostdlib
MDASFLAGS = -m68000 --register-prefix-optional

MDPREFIX = $(ROOTDIR)/m68k-elf/bin/m68k-elf-
MDCC = $(MDPREFIX)gcc
MDAS = $(MDPREFIX)as
MDLD = $(MDPREFIX)ld
MDOBJC = $(MDPREFIX)objcopy

ASMZ80 = $(ROOTDIR)/bin/zasm
FLAGSZ80 = -vb2

DD = dd
RM = rm -rf

TARGET = RaycastDemo
LIBS = $(LIBPATH) -lpcm -lm -lc -lgcc -lnosys
OBJS = crt0.o main.o module.o cdfh.o hw_md.o wolfdemo.o
FILES = files.o

all: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(MDOBJC) -O binary $< temp.bin
	$(DD) if=temp.bin of=$@ bs=2048 conv=sync

$(TARGET).elf: $(OBJS) $(FILES)
	$(MDCC) $(MDLDFLAGS) $(OBJS) $(LIBS) $(FILES) -o $(TARGET).elf

%.o80: %.s80
	$(ASMZ80) $(FLAGSZ80) -o $@ $<

main.o: main.c
	$(MDCC) $(MDHWFLAGS) $(INCPATH) $< -o $@

module.o: module.c
	$(MDCC) $(MDHWFLAGS) $(INCPATH) $< -o $@

%.o: %.c
	$(MDCC) $(MDCCFLAGS) $(INCPATH) $< -o $@

%.o: %.s
	$(MDAS) $(MDASFLAGS) $(INCPATH) $< -o $@

cd: $(TARGET).bin
	mkdir -p cd
	mkdir -p cd/MUSIC
	cp $(TARGET).bin cd/APP.BIN
	cp -r music/* cd/MUSIC/
	genisoimage -sysid "SEGA SEGACD" -volid "RaycastDemo" -generic-boot $(BOOTBLOCK) -full-iso9660-filenames -o $(TARGET).iso cd

clean:
	$(RM) *.o *.o80 *.bin *.elf *.map *.log *.iso cd

