[bits 32]

MB_MAGIC    equ 0x1BADB002
MB_FLAGS    equ (1 << 0) | (1 << 1)
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

section .multiboot
align 4
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM

section .text
global _start
extern kmain

_start:
    cli
    mov esp, stack_top 

    call kmain

    hlt
    jmp $

section .bss
align 16
stack_bottom:
    resb 16384
    
stack_top: