#include <stdbool.h>

volatile unsigned char *video = (unsigned char *)0xb8000;
int cursor_x = 0;
int cursor_y = 0;

const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

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
            char numpad_chars[] = "789-456+1230.";
            if (scancode >= 0x47 && scancode <= 0x53) {
                putchar(numpad_chars[scancode - 0x47], 0x0F);
                continue;
            }
        }

        char c = 0;
        bool use_shift = shift || caps;
        if (use_shift)
            c = kbd_shifted[scancode];
        else
            c = kbd_normal[scancode];

        if (c != 0) return c;
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