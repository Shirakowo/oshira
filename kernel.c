volatile unsigned char *video = (unsigned char *)0xb8000;

void clear_screen() {
    for (int i = 0; i < 80*25*2; i += 2) {
        video[i] = ' ';
        video[i+1] = 0x07;
    }
}

void print(const char *str) {
    int i = 0;
    while (str[i] != '\0') {
        video[i*2] = (unsigned char)str[i];
        video[i*2 + 1] = 0x0A;
        i++;
    }
}

void kmain() {
    clear_screen();
    print("Hello OS :3");
    
    while (1) {
        asm volatile("hlt");
    }
}