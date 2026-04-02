#include <stdbool.h>

volatile unsigned char *video = (unsigned char *)0xb8000;
int cursor_x = 0;
int cursor_y = 0;

const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

const char *version = "OShira v0.1";

unsigned char kbd_normal[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned char kbd_shifted[128] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    asm volatile("outb %0, %1" : : "a"(data), "dN"(port));
}

bool shift = false;
bool caps = false;

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = 0x0F;
    }

    cursor_x = 0;
    cursor_y = 0;
}

void putchar(char c, unsigned char color) {
    if (c == '\n' || cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_HEIGHT) {
        clear_screen();
    }

    if (c == '\b') {
        if (cursor_x > 0) cursor_x--;
        int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        video[pos] = ' ';
        video[pos + 1] = color;
        return;
    }

    int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
    video[pos] = c;
    video[pos + 1] = color;
    cursor_x++;
}

void print(const char *str, unsigned char color) {
    while (*str) {
        putchar(*str++, color);
    }
}

void print_line(const char *str) {
    print(str, 0x0F);
    putchar('\n', 0x0F);
}

char get_key() {
    while (1) {
        if ((inb(0x64) & 1) == 0) continue;

        unsigned char scancode = inb(0x60);

        if (scancode >= 128) {
            scancode -= 128;
            if (scancode == 0x2A || scancode == 0x36) shift = false;
            continue;
        }

        if (scancode == 0x2A || scancode == 0x36) {
            shift = true;
            continue;
        }
        
        if (scancode == 0x3A) {
            caps = !caps;
            continue;
        }

        if (scancode >= 0x3B && scancode <= 0x43) {
            print(" [F", 0x0C);
            putchar('1' + (scancode - 0x3B), 0x0C);
            print("]", 0x0C);
            continue;
        }

        if (scancode == 0x44) {
            print(" [F10]", 0x0C);
            continue;
        }

        if (scancode == 0x57) {
            print(" [F11]", 0x0C);
            continue;
        }

        if (scancode == 0x58) {
            print(" [F12]", 0x0C);
            continue;
        }

        if (scancode >= 0x47 && scancode <= 0x53) {
            const char *numpad = "789-456+1230.";
            char c = numpad[scancode - 0x47];
            if (c) return c;
        }

        bool use_shift = shift || caps;
        char c = use_shift ? kbd_shifted[scancode] : kbd_normal[scancode];

        if (c != 0) return c;
    }
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, int n) {
    while (n-- && *s1 && (*s1 == *s2)) { s1++; s2++; }
    if (n < 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void execute_command(char *cmd) {
    if (cmd[0] == '\0') return;

    if (strcmp(cmd, "help") == 0) {
        print_line("Available commands:");
        print_line("CLEAR     Clear the screen");
        print_line("ECHO      Echo text back");
        print_line("HELP      Show this message");
        print_line("VERSION   Show OS version");
    } else if (strcmp(cmd, "clear") == 0) {
        clear_screen();
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        print(cmd + 5, 0x0F);
        putchar('\n', 0x0F);
    } else if (strcmp(cmd, "version") == 0) {
        print_line(version);
    } else {
        print_line("Unknown command");
    }
}

void kmain() {
    clear_screen();

    print_line("Welcome to OShira");

    char buffer[256];
    int buf_idx = 0;

    while (1) {
        if (cursor_x == 0) {
            putchar('>', 0x0F);
        }

        char c = get_key();

        if (c == '\n') {
            putchar('\n', 0x0F);
            buffer[buf_idx] = '\0';
            execute_command(buffer);
            buf_idx = 0;
        }

        else if (c == '\b') {
            if (buf_idx > 0) {
                buf_idx--;
                putchar('\b', 0x0F);
            }
        }

        else if (c >= ' ' && c <= '~' && buf_idx < 255) {
            buffer[buf_idx++] = c;
            putchar(c, 0x0F);
        }
    }
}