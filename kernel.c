volatile unsigned char *video = (unsigned char *)0xb8000;
int cursor_x = 0;
int cursor_y = 0;

const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    asm volatile("inb %1, %0" : "=a"(result) : "dN"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    asm volatile("outb %0, %1" : : "a"(data), "dN"(port));
}

unsigned char kbd_US[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

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
        if ((inb(0x64) & 1) != 0) {
            unsigned char scancode = inb(0x60);
            if (scancode < 128) {
                return kbd_US[scancode];
            }
        }
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