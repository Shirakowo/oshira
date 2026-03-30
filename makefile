CC = gcc
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra \
         -fno-pie -fno-pic -fno-stack-protector -nostdlib -nodefaultlibs

AS = nasm
LD = ld

all: kernel.elf iso

kernel.elf: boot.o kernel.o
	$(LD) -m elf_i386 -T linker.ld -o kernel.elf boot.o kernel.o

boot.o: boot.asm
	$(AS) -f elf32 boot.asm -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

iso: kernel.elf
	mkdir -p iso/boot/grub
	cp kernel.elf iso/boot/kernel.elf
	cp grub.cfg iso/boot/grub/
	grub-mkrescue -o oshira.iso iso

clean:
	rm -f *.o kernel.elf oshira.iso
	rm -rf iso

run:
	qemu-system-i386 -cdrom oshira.iso -boot d

.PHONY: all iso clean run